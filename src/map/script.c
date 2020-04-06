/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#define HERCULES_CORE

#include "config/core.h" // RENEWAL, RENEWAL_ASPD, RENEWAL_CAST, RENEWAL_DROP, RENEWAL_EDP, RENEWAL_EXP, RENEWAL_LVDMG, SCRIPT_CALLFUNC_CHECK, SECURE_NPCTIMEOUT, SECURE_NPCTIMEOUT_INTERVAL
#include "script.h"

#include "map/atcommand.h"
#include "map/battle.h"
#include "map/battleground.h"
#include "map/channel.h"
#include "map/chat.h"
#include "map/chrif.h"
#include "map/clan.h"
#include "map/clif.h"
#include "map/date.h"
#include "map/elemental.h"
#include "map/guild.h"
#include "map/homunculus.h"
#include "map/instance.h"
#include "map/intif.h"
#include "map/itemdb.h"
#include "map/log.h"
#include "map/mail.h"
#include "map/map.h"
#include "map/mapreg.h"
#include "map/mercenary.h"
#include "map/messages.h"
#include "map/mob.h"
#include "map/npc.h"
#include "map/party.h"
#include "map/path.h"
#include "map/pc.h"
#include "map/pet.h"
#include "map/pet.h"
#include "map/quest.h"
#include "map/refine.h"
#include "map/skill.h"
#include "map/status.h"
#include "map/status.h"
#include "map/storage.h"
#include "map/unit.h"
#include "map/achievement.h"
#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/db.h"
#include "common/memmgr.h"
#include "common/md5calc.h"
#include "common/mmo.h" // NEW_CARTS
#include "common/nullpo.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/socket.h" // usage: getcharip
#include "common/sql.h"
#include "common/strlib.h"
#include "common/sysinfo.h"
#include "common/timer.h"
#include "common/utils.h"
#include "common/HPM.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifndef WIN32
	#include <sys/time.h>
#endif

static struct script_interface script_s;
struct script_interface *script;

static inline int GETVALUE(const struct script_buf *buf, int i)  __attribute__((nonnull (1)));
static inline int GETVALUE(const struct script_buf *buf, int i)
{
	Assert_ret(VECTOR_LENGTH(*buf) > i + 2);
	return (int)MakeDWord(MakeWord(VECTOR_INDEX(*buf, i), VECTOR_INDEX(*buf, i+1)),
	                      MakeWord(VECTOR_INDEX(*buf, i+2), 0));
}

static inline void SETVALUE(struct script_buf *buf, int i, int n) __attribute__((nonnull (1)));
static inline void SETVALUE(struct script_buf *buf, int i, int n)
{
	Assert_retv(VECTOR_LENGTH(*buf) > i + 2);
	VECTOR_INDEX(*buf, i)   = GetByte(n, 0);
	VECTOR_INDEX(*buf, i+1) = GetByte(n, 1);
	VECTOR_INDEX(*buf, i+2) = GetByte(n, 2);
}

static const char *script_op2name(int op)
{
#define RETURN_OP_NAME(type) case type: return #type
	switch( op ) {
	RETURN_OP_NAME(C_NOP);
	RETURN_OP_NAME(C_POS);
	RETURN_OP_NAME(C_INT);
	RETURN_OP_NAME(C_PARAM);
	RETURN_OP_NAME(C_FUNC);
	RETURN_OP_NAME(C_STR);
	RETURN_OP_NAME(C_CONSTSTR);
	RETURN_OP_NAME(C_ARG);
	RETURN_OP_NAME(C_NAME);
	RETURN_OP_NAME(C_EOL);
	RETURN_OP_NAME(C_RETINFO);
	RETURN_OP_NAME(C_USERFUNC);
	RETURN_OP_NAME(C_USERFUNC_POS);

	RETURN_OP_NAME(C_REF);
	RETURN_OP_NAME(C_LSTR);

	// operators
	RETURN_OP_NAME(C_OP3);
	RETURN_OP_NAME(C_LOR);
	RETURN_OP_NAME(C_LAND);
	RETURN_OP_NAME(C_LE);
	RETURN_OP_NAME(C_LT);
	RETURN_OP_NAME(C_GE);
	RETURN_OP_NAME(C_GT);
	RETURN_OP_NAME(C_EQ);
	RETURN_OP_NAME(C_NE);
	RETURN_OP_NAME(C_XOR);
	RETURN_OP_NAME(C_OR);
	RETURN_OP_NAME(C_AND);
	RETURN_OP_NAME(C_ADD);
	RETURN_OP_NAME(C_SUB);
	RETURN_OP_NAME(C_MUL);
	RETURN_OP_NAME(C_POW);
	RETURN_OP_NAME(C_DIV);
	RETURN_OP_NAME(C_MOD);
	RETURN_OP_NAME(C_NEG);
	RETURN_OP_NAME(C_LNOT);
	RETURN_OP_NAME(C_NOT);
	RETURN_OP_NAME(C_R_SHIFT);
	RETURN_OP_NAME(C_L_SHIFT);
	RETURN_OP_NAME(C_ADD_POST);
	RETURN_OP_NAME(C_SUB_POST);
	RETURN_OP_NAME(C_ADD_PRE);
	RETURN_OP_NAME(C_SUB_PRE);
	RETURN_OP_NAME(C_RE_EQ);
	RETURN_OP_NAME(C_RE_NE);

	default:
		ShowDebug("script_op2name: unexpected op=%d\n", op);
		return "???";
	}
#undef RETURN_OP_NAME
}

#ifdef SCRIPT_DEBUG_DUMP_STACK
static void script_dump_stack(struct script_state *st)
{
	int i;
	nullpo_retv(st);
	ShowMessage("\tstart = %d\n", st->start);
	ShowMessage("\tend   = %d\n", st->end);
	ShowMessage("\tdefsp = %d\n", st->stack->defsp);
	ShowMessage("\tsp    = %d\n", st->stack->sp);
	for( i = 0; i < st->stack->sp; ++i )
	{
		struct script_data* data = &st->stack->stack_data[i];
		ShowMessage("\t[%d] %s", i, script->op2name(data->type));
		switch( data->type )
		{
		case C_INT:
		case C_POS:
			ShowMessage(" %d\n", data->u.num);
			break;

		case C_STR:
		case C_CONSTSTR:
			ShowMessage(" \"%s\"\n", data->u.str);
			break;

		case C_NAME:
			ShowMessage(" \"%s\" (id=%d ref=%p subtype=%s)\n", reference_getname(data), data->u.num, data->ref, script->op2name(script->str_data[data->u.num].type));
			break;

		case C_RETINFO:
			{
				struct script_retinfo* ri = data->u.ri;
				ShowMessage(" %p {scope.vars=%p, scope.arrays=%p, script=%p, pos=%d, nargs=%d, defsp=%d}\n", ri, ri->scope.vars, ri->scope.arrays, ri->script, ri->pos, ri->nargs, ri->defsp);
			}
			break;
		default:
			ShowMessage("\n");
			break;
		}
	}
}
#endif

/// Reports on the console the src of a script error.
static void script_reportsrc(struct script_state *st)
{
	struct block_list* bl;

	nullpo_retv(st);
	if( st->oid == 0 )
		return; //Can't report source.

	bl = map->id2bl(st->oid);
	if( bl == NULL )
		return;

	switch( bl->type ) {
		case BL_NPC:
		{
			const struct npc_data *nd = BL_UCCAST(BL_NPC, bl);
			if (bl->m >= 0)
				ShowDebug("Source (NPC): %s at %s (%d,%d)\n", nd->name, map->list[bl->m].name, bl->x, bl->y);
			else
				ShowDebug("Source (NPC): %s (invisible/not on a map)\n", nd->name);
		}
			break;
		default:
			if( bl->m >= 0 )
				ShowDebug("Source (Non-NPC type %u): name %s at %s (%d,%d)\n", bl->type, clif->get_bl_name(bl), map->list[bl->m].name, bl->x, bl->y);
			else
				ShowDebug("Source (Non-NPC type %u): name %s (invisible/not on a map)\n", bl->type, clif->get_bl_name(bl));
			break;
	}
}

/// Reports on the console information about the script data.
static void script_reportdata(struct script_data *data)
{
	if( data == NULL )
		return;
	switch( data->type ) {
		case C_NOP:// no value
			ShowDebug("Data: nothing (nil)\n");
			break;
		case C_INT:// number
			ShowDebug("Data: number value=%"PRId64"\n", data->u.num);
			break;
		case C_STR:
		case C_CONSTSTR:// string
			if( data->u.str ) {
				ShowDebug("Data: string value=\"%s\"\n", data->u.str);
			} else {
				ShowDebug("Data: string value=NULL\n");
			}
			break;
		case C_NAME:// reference
			if( reference_tovariable(data) ) {// variable
				const char* name = reference_getname(data);
				ShowDebug("Data: variable name='%s' index=%u\n", name, reference_getindex(data));
			} else if( reference_toconstant(data) ) {// constant
				ShowDebug("Data: constant name='%s' value=%d\n", reference_getname(data), reference_getconstant(data));
			} else if( reference_toparam(data) ) {// param
				ShowDebug("Data: param name='%s' type=%d\n", reference_getname(data), reference_getparamtype(data));
			} else {// ???
				ShowDebug("Data: reference name='%s' type=%s\n", reference_getname(data), script->op2name(data->type));
				ShowDebug("Please report this!!! - script->str_data.type=%s\n", script->op2name(script->str_data[reference_getid(data)].type));
			}
			break;
		case C_POS:// label
			ShowDebug("Data: label pos=%"PRId64"\n", data->u.num);
			break;
		default:
			ShowDebug("Data: %s\n", script->op2name(data->type));
			break;
	}
}

/// Reports on the console information about the current built-in function.
static void script_reportfunc(struct script_state *st)
{
	int params, id;
	struct script_data* data;

	if (!script_hasdata(st,0)) {
		// no stack
		return;
	}

	data = script_getdata(st,0);

	if (!data_isreference(data) || script->str_data[reference_getid(data)].type != C_FUNC) {
		// script currently not executing a built-in function or corrupt stack
		return;
	}

	id     = reference_getid(data);
	params = script_lastdata(st)-1;

	if (params > 0) {
		int i;
		ShowDebug("Function: %s (%d parameter%s):\n", script->get_str(id), params, ( params == 1 ) ? "" : "s");

		for (i = 2; i <= script_lastdata(st); i++) {
			script->reportdata(script_getdata(st,i));
		}
	} else {
		ShowDebug("Function: %s (no parameters)\n", script->get_str(id));
	}
}

/*==========================================
 * Output error message
 *------------------------------------------*/
static void disp_error_message2(const char *mes, const char *pos, int report)  __attribute__((nonnull (1))) analyzer_noreturn;
static void disp_error_message2(const char *mes, const char *pos, int report)
{
	script->error_msg = aStrdup(mes);
	script->error_pos = pos;
	script->error_report = report;
	longjmp( script->error_jump, 1 );
}
#define disp_error_message(mes,pos) (disp_error_message2((mes),(pos),1))

static void disp_warning_message(const char *mes, const char *pos)
{
	script->warning(script->parser_current_src,script->parser_current_file,script->parser_current_line,mes,pos);
}

/// Checks event parameter validity
static void check_event(struct script_state *st, const char *evt)
{
	if( evt && evt[0] && !stristr(evt, "::On") )
	{
		ShowWarning("NPC event parameter deprecated! Please use 'NPCNAME::OnEVENT' instead of '%s'.\n", evt);
		script->reportsrc(st);
	}
}

/*==========================================
 * Hashes the input string
 *------------------------------------------*/
static unsigned int calc_hash(const char *p)
{
	unsigned int h;

	nullpo_ret(p);
#if defined(SCRIPT_HASH_DJB2)
	h = 5381;
	while( *p ) // hash*33 + c
		h = ( h << 5 ) + h + ((unsigned char)(*p++));
#elif defined(SCRIPT_HASH_SDBM)
	h = 0;
	while( *p ) // hash*65599 + c
		h = ( h << 6 ) + ( h << 16 ) - h + ((unsigned char)(*p++));
#elif defined(SCRIPT_HASH_ELF) // UNIX ELF hash
	h = 0;
	while( *p ) {
		unsigned int g;
		h = ( h << 4 ) + ((unsigned char)(*p++));
		g = h & 0xF0000000;
		if( g ) {
			h ^= g >> 24;
			h &= ~g;
		}
	}
#else // athena hash
	h = 0;
	while( *p )
		h = ( h << 1 ) + ( h >> 3 ) + ( h >> 5 ) + ( h >> 8 ) + (unsigned char)(*p++);
#endif

	return h % SCRIPT_HASH_SIZE;
}

/*==========================================
 * Hashes the input string in a case insensitive way
 *------------------------------------------*/
static unsigned int calc_hash_ci(const char *p)
{
	unsigned int h = 0;
#ifdef ENABLE_CASE_CHECK

	nullpo_ret(p);
#if defined(SCRIPT_HASH_DJB2)
	h = 5381;
	while( *p ) // hash*33 + c
		h = ( h << 5 ) + h + ((unsigned char)TOLOWER(*p++));
#elif defined(SCRIPT_HASH_SDBM)
	h = 0;
	while( *p ) // hash*65599 + c
		h = ( h << 6 ) + ( h << 16 ) - h + ((unsigned char)TOLOWER(*p++));
#elif defined(SCRIPT_HASH_ELF) // UNIX ELF hash
	h = 0;
	while( *p ) {
		unsigned int g;
		h = ( h << 4 ) + ((unsigned char)TOLOWER(*p++));
		g = h & 0xF0000000;
		if( g ) {
			h ^= g >> 24;
			h &= ~g;
		}
	}
#else // athena hash
	h = 0;
	while( *p )
		h = ( h << 1 ) + ( h >> 3 ) + ( h >> 5 ) + ( h >> 8 ) + (unsigned char)TOLOWER(*p++);
#endif

#endif // ENABLE_CASE_CHECK
	return h % SCRIPT_HASH_SIZE;
}

/*==========================================
 * script->str_data manipulation functions
 *------------------------------------------*/

/// Looks up string using the provided id.
static const char *script_get_str(int id)
{
	Assert_retr(NULL, id >= LABEL_START && id < script->str_size);
	return script->str_buf+script->str_data[id].str;
}

/// Returns the uid of the string, or -1.
static int script_search_str(const char *p)
{
	int i;

	for( i = script->str_hash[script->calc_hash(p)]; i != 0; i = script->str_data[i].next ) {
		if( strcmp(script->get_str(i),p) == 0 ) {
			return i;
		}
	}

	return -1;
}

static void script_casecheck_clear_sub(struct casecheck_data *ccd)
{
#ifdef ENABLE_CASE_CHECK
	nullpo_retv(ccd);
	if (ccd->str_data) {
		aFree(ccd->str_data);
		ccd->str_data = NULL;
	}
	ccd->str_data_size = 0;
	ccd->str_num = 1;
	if (ccd->str_buf) {
		aFree(ccd->str_buf);
		ccd->str_buf = NULL;
	}
	ccd->str_pos = 0;
	ccd->str_size = 0;
	memset(ccd->str_hash, 0, sizeof(ccd->str_hash));
#endif // ENABLE_CASE_CHECK
}

static void script_global_casecheck_clear(void)
{
	script_casecheck_clear_sub(&script->global_casecheck);
}

static void script_local_casecheck_clear(void)
{
	script_casecheck_clear_sub(&script->local_casecheck);
}

static const char *script_casecheck_add_str_sub(struct casecheck_data *ccd, const char *p)
{
#ifdef ENABLE_CASE_CHECK
	int len;
	int h = script->calc_hash_ci(p);
	nullpo_retr(NULL, ccd);
	if (ccd->str_hash[h] == 0) {
		//empty bucket, add new node here
		ccd->str_hash[h] = ccd->str_num;
	} else {
		int i;
		for (i = ccd->str_hash[h]; ; i = ccd->str_data[i].next) {
			const char *s = NULL;
			Assert_retb(i >= 0 && i < ccd->str_size);
			s = ccd->str_buf+ccd->str_data[i].str;
			if (strcasecmp(s,p) == 0) {
				return s; // string already in list
			}
			if (ccd->str_data[i].next == 0)
				break; // reached the end
		}

		// append node to end of list
		ccd->str_data[i].next = ccd->str_num;
	}

	// grow list if neccessary
	if( ccd->str_num >= ccd->str_data_size ) {
		ccd->str_data_size += 1280;
		RECREATE(ccd->str_data,struct str_data_struct,ccd->str_data_size);
		memset(ccd->str_data + (ccd->str_data_size - 1280), '\0', 1280);
	}

	len=(int)strlen(p);

	// grow string buffer if neccessary
	while( ccd->str_pos+len+1 >= ccd->str_size ) {
		ccd->str_size += 10240;
		RECREATE(ccd->str_buf,char,ccd->str_size);
		memset(ccd->str_buf + (ccd->str_size - 10240), '\0', 10240);
	}

	safestrncpy(ccd->str_buf+ccd->str_pos, p, len+1);
	ccd->str_data[ccd->str_num].type = C_NOP;
	ccd->str_data[ccd->str_num].str = ccd->str_pos;
	ccd->str_data[ccd->str_num].val = 0;
	ccd->str_data[ccd->str_num].next = 0;
	ccd->str_data[ccd->str_num].func = NULL;
	ccd->str_data[ccd->str_num].backpatch = -1;
	ccd->str_data[ccd->str_num].label = -1;
	ccd->str_pos += len+1;

	ccd->str_num++;
#endif // ENABLE_CASE_CHECK
	return NULL;
}

static const char *script_global_casecheck_add_str(const char *p)
{
	return script_casecheck_add_str_sub(&script->global_casecheck, p);
}

static const char *script_local_casecheck_add_str(const char *p)
{
	return script_casecheck_add_str_sub(&script->local_casecheck, p);
}

/// Stores a copy of the string and returns its id.
/// If an identical string is already present, returns its id instead.
static int script_add_str(const char *p)
{
	int len, h = script->calc_hash(p);
#ifdef ENABLE_CASE_CHECK
	const char *existingentry = NULL;
#endif // ENABLE_CASE_CHECK

	if (script->str_hash[h] == 0) {
		// empty bucket, add new node here
		script->str_hash[h] = script->str_num;
	} else {
		// scan for end of list, or occurence of identical string
		int i;
		for (i = script->str_hash[h]; ; i = script->str_data[i].next) {
			if( strcmp(script->get_str(i),p) == 0 ) {
				return i; // string already in list
			}
			if( script->str_data[i].next == 0 )
				break; // reached the end
		}

		// append node to end of list
		script->str_data[i].next = script->str_num;
	}

#ifdef ENABLE_CASE_CHECK
	if( (strncmp(p, ".@", 2) == 0) ) {
		// Local scope vars are checked separately to decrease false positives
		existingentry = script->local_casecheck.add_str(p);
	} else {
		existingentry = script->global_casecheck.add_str(p);
		if( existingentry ) {
			if( strcasecmp(p, "disguise") == 0 || strcasecmp(p, "Poison_Spore") == 0
			 || strcasecmp(p, "PecoPeco_Egg") == 0 || strcasecmp(p, "Soccer_Ball") == 0
			 || strcasecmp(p, "Horn") == 0 || strcasecmp(p, "Treasure_Box_") == 0
			 || strcasecmp(p, "Lord_of_Death") == 0
			  ) // Known duplicates, don't bother warning the user
				existingentry = NULL;
		}
	}
	if( existingentry ) {
		DeprecationCaseWarning("script_add_str", p, existingentry, script->parser_current_file); // TODO
	}
#endif // ENABLE_CASE_CHECK

	// grow list if neccessary
	if( script->str_num >= script->str_data_size ) {
		script->str_data_size += 1280;
		RECREATE(script->str_data,struct str_data_struct,script->str_data_size);
		memset(script->str_data + (script->str_data_size - 1280), '\0', 1280);
	}

	len=(int)strlen(p);

	// grow string buffer if neccessary
	while( script->str_pos+len+1 >= script->str_size ) {
		script->str_size += 10240;
		RECREATE(script->str_buf,char,script->str_size);
		memset(script->str_buf + (script->str_size - 10240), '\0', 10240);
	}

	safestrncpy(script->str_buf+script->str_pos, p, len+1);
	script->str_data[script->str_num].type = C_NOP;
	script->str_data[script->str_num].str = script->str_pos;
	script->str_data[script->str_num].val = 0;
	script->str_data[script->str_num].next = 0;
	script->str_data[script->str_num].func = NULL;
	script->str_data[script->str_num].backpatch = -1;
	script->str_data[script->str_num].label = -1;
	script->str_pos += len+1;

	return script->str_num++;
}

static int script_add_variable(const char *varname)
{
	int key = script->search_str(varname);

	if (key < 0) {
		key = script->add_str(varname);
		script->str_data[key].type = C_NAME;
	}

	return key;
}

/**
 * Appends 1 byte to the script buffer.
 *
 * @param a The byte to append.
 */
static void add_scriptb(int a)
{
	VECTOR_ENSURE(script->buf, 1, SCRIPT_BLOCK_SIZE);
	VECTOR_PUSH(script->buf, (uint8)a);
}

/**
 * Appends a c_op value to the script buffer.
 *
 * The value is variable-length encoded into 8-bit blocks.
 * The encoding scheme is ( 01?????? )* 00??????, LSB first.
 * All blocks but the last hold 7 bits of data, topmost bit is always 1 (carries).
 *
 * @param a The value to append.
 */
static void add_scriptc(int a)
{
	while( a >= 0x40 )
	{
		script->addb((a&0x3f)|0x40);
		a = (a - 0x40) >> 6;
	}

	script->addb(a);
}

/**
 * Appends an integer value to the script buffer.
 *
 * The value is variable-length encoded into 8-bit blocks.
 * The encoding scheme is ( 11?????? )* 10??????, LSB first.
 * All blocks but the last hold 7 bits of data, topmost bit is always 1 (carries).
 *
 * @param a The value to append.
 */
static void add_scripti(int a)
{
	while( a >= 0x40 )
	{
		script->addb((a&0x3f)|0xc0);
		a = (a - 0x40) >> 6;
	}
	script->addb(a|0x80);
}

/**
 * Appends a script->str_data object (label/function/variable/integer) to the script buffer.
 *
 * @param l The id of the script->str_data entry (Maximum up to 16M)
 */
static void add_scriptl(int l)
{
	int backpatch = script->str_data[l].backpatch;

	switch(script->str_data[l].type) {
		case C_POS:
		case C_USERFUNC_POS:
			script->addc(C_POS);
			script->addb(script->str_data[l].label);
			script->addb(script->str_data[l].label>>8);
			script->addb(script->str_data[l].label>>16);
			break;
		case C_NOP:
		case C_USERFUNC:
			// Embedded data backpatch there is a possibility of label
			script->addc(C_NAME);
			script->str_data[l].backpatch = VECTOR_LENGTH(script->buf);
			script->addb(backpatch);
			script->addb(backpatch>>8);
			script->addb(backpatch>>16);
			break;
		case C_INT:
			script->addi(abs(script->str_data[l].val));
			if( script->str_data[l].val < 0 ) //Notice that this is negative, from jA (Rayce)
				script->addc(C_NEG);
			break;
		default: // assume C_NAME
			script->addc(C_NAME);
			script->addb(l);
			script->addb(l>>8);
			script->addb(l>>16);
			break;
	}
}

/*==========================================
 * Resolve the label
 *------------------------------------------*/
static void set_label(int l, int pos, const char *script_pos)
{
	int i;

	if(script->str_data[l].type==C_INT || script->str_data[l].type==C_PARAM || script->str_data[l].type==C_FUNC) {
		//Prevent overwriting constants values, parameters and built-in functions [Skotlex]
		disp_error_message("set_label: invalid label name",script_pos);
		return;
	}
	if(script->str_data[l].label!=-1) {
		disp_error_message("set_label: dup label ",script_pos);
		return;
	}
	script->str_data[l].type=(script->str_data[l].type == C_USERFUNC ? C_USERFUNC_POS : C_POS);
	script->str_data[l].label=pos;
	for (i = script->str_data[l].backpatch; i >= 0 && i != 0x00ffffff; ) {
		int next = GETVALUE(&script->buf, i);
		VECTOR_INDEX(script->buf, i-1) = (script->str_data[l].type == C_USERFUNC ? C_USERFUNC_POS : C_POS);
		SETVALUE(&script->buf, i, pos);
		i = next;
	}
}

/// Skips spaces and/or comments.
static const char *script_skip_space(const char *p)
{
	if( p == NULL )
		return NULL;
	for(;;)
	{
		while( ISSPACE(*p) )
			++p;
		if( *p == '/' && p[1] == '/' )
		{// line comment
			while(*p && *p!='\n')
				++p;
		}
		else if( *p == '/' && p[1] == '*' )
		{// block comment
			p += 2;
			for(;;)
			{
				if( *p == '\0' ) {
					script->disp_warning_message("script:script->skip_space: end of file while parsing block comment. expected "CL_BOLD"*/"CL_NORM, p);
					return p;
				}
				if( *p == '*' && p[1] == '/' )
				{// end of block comment
					p += 2;
					break;
				}
				++p;
			}
		}
		else
			break;
	}
	return p;
}

/// Skips a word.
/// A word consists of undercores and/or alphanumeric characters,
/// and valid variable prefixes/postfixes.
static const char *skip_word(const char *p)
{
	nullpo_retr(NULL, p);
	// prefix
	switch( *p ) {
		case '@':// temporary char variable
			++p; break;
		case '#':// account variable
			p += ( p[1] == '#' ? 2 : 1 ); break;
		case '\'':// instance variable
			++p; break;
		case '.':// npc variable
			p += ( p[1] == '@' ? 2 : 1 ); break;
		case '$':// global variable
			p += ( p[1] == '@' ? 2 : 1 ); break;
	}

	while (ISALNUM(*p) || *p == '_')
		++p;

	// postfix
	if( *p == '$' )// string
		p++;

	return p;
}
/// Adds a word to script->str_data.
/// @see skip_word
/// @see script->add_str
static int add_word(const char *p)
{
	size_t len;
	int i;

	nullpo_retr(0, p);
	// Check for a word
	len = script->skip_word(p) - p;
	if( len == 0 )
		disp_error_message("script:add_word: invalid word. A word consists of undercores and/or alphanumeric characters, and valid variable prefixes/postfixes.", p);

	// Duplicate the word
	if( len+1 > script->word_size )
		RECREATE(script->word_buf, char, (script->word_size = (len+1)));

	memcpy(script->word_buf, p, len);
	script->word_buf[len] = 0;

	// add the word
	i = script->add_str(script->word_buf);

	return i;
}

/// Parses a function call.
/// The argument list can have parenthesis or not.
/// The number of arguments is checked.
static const char *parse_callfunc(const char *p, int require_paren, int is_custom)
{
	const char *p2;
	char *arg = NULL;
	char null_arg = '\0';
	int func;
	bool macro = false;

	nullpo_retr(NULL, p);
	// is need add check for arg null pointer below?
	func = script->add_word(p);
	if (script->str_data[func].type == C_FUNC) {
		script->syntax.nested_call++;
		if (script->syntax.last_func != -1) {
			if (script->str_data[func].val == script->buildin_lang_macro_offset) {
				script->syntax.lang_macro_active = true;
				macro = true;
			} else if (script->str_data[func].val == script->buildin_lang_macro_fmtstring_offset) {
				script->syntax.lang_macro_fmtstring_active = true;
				macro = true;
			}
		}

		if( !macro ) {
			// buildin function
			script->syntax.last_func = script->str_data[func].val;
			script->addl(func);
			script->addc(C_ARG);
		}

		arg = script->buildin[script->str_data[func].val];
		if (script->str_data[func].deprecated)
			DeprecationWarning(p);
		if( !arg ) arg = &null_arg; // Use a dummy, null string
	} else if( script->str_data[func].type == C_USERFUNC || script->str_data[func].type == C_USERFUNC_POS ) {
		// script defined function
		script->addl(script->buildin_callsub_ref);
		script->addc(C_ARG);
		script->addl(func);
		arg = script->buildin[script->str_data[script->buildin_callsub_ref].val];
		if( *arg == 0 )
			disp_error_message("parse_callfunc: callsub has no arguments, please review its definition",p);
		if( *arg != '*' )
			++arg; // count func as argument
	} else {
#ifdef SCRIPT_CALLFUNC_CHECK
		const char* name = script->get_str(func);
		if( !is_custom && strdb_get(script->userfunc_db, name) == NULL ) {
#endif
			disp_error_message("parse_line: expect command, missing function name or calling undeclared function",p);
#ifdef SCRIPT_CALLFUNC_CHECK
		} else {;
			script->addl(script->buildin_callfunc_ref);
			script->addc(C_ARG);
			script->addc(C_STR);
			while( *name ) script->addb(*name ++);
			script->addb(0);
			arg = script->buildin[script->str_data[script->buildin_callfunc_ref].val];
			if( *arg != '*' ) ++ arg;
		}
#endif
	}

	p = script->skip_word(p);
	p = script->skip_space(p);
	script->syntax.curly[script->syntax.curly_count].type = TYPE_ARGLIST;
	script->syntax.curly[script->syntax.curly_count].count = 0;
	if( *p == ';' )
	{// <func name> ';'
		script->syntax.curly[script->syntax.curly_count].flag = ARGLIST_NO_PAREN;
	} else if( *p == '(' && *(p2=script->skip_space(p+1)) == ')' )
	{// <func name> '(' ')'
		script->syntax.curly[script->syntax.curly_count].flag = ARGLIST_PAREN;
		p = p2;
	/*
	} else if( 0 && require_paren && *p != '(' )
	{// <func name>
		script->syntax.curly[script->syntax.curly_count].flag = ARGLIST_NO_PAREN;
	*/
	} else {// <func name> <arg list>
		if( require_paren ) {
			if( *p != '(' )
				disp_error_message("need '('",p);
			++p; // skip '('
			script->syntax.curly[script->syntax.curly_count].flag = ARGLIST_PAREN;
		} else if( *p == '(' ) {
			script->syntax.curly[script->syntax.curly_count].flag = ARGLIST_UNDEFINED;
		} else {
			script->syntax.curly[script->syntax.curly_count].flag = ARGLIST_NO_PAREN;
		}
		++script->syntax.curly_count;
		while( *arg ) {
			p2=script->parse_subexpr(p,-1);
			if( p == p2 )
				break; // not an argument
			if( *arg != '*' )
				++arg; // next argument

			p=script->skip_space(p2);
			if( *arg == 0 || *p != ',' )
				break; // no more arguments
			++p; // skip comma
		}
		--script->syntax.curly_count;
	}
	if( arg && *arg && *arg != '?' && *arg != '*' )
		disp_error_message2("parse_callfunc: not enough arguments, expected ','", p, script->config.warn_func_mismatch_paramnum);
	if( script->syntax.curly[script->syntax.curly_count].type != TYPE_ARGLIST )
		disp_error_message("parse_callfunc: DEBUG last curly is not an argument list",p);
	if( script->syntax.curly[script->syntax.curly_count].flag == ARGLIST_PAREN ) {
		if( *p != ')' )
			disp_error_message("parse_callfunc: expected ')' to close argument list",p);
		++p;

		if (script->str_data[func].val == script->buildin_lang_macro_offset)
			script->syntax.lang_macro_active = false;
		else if (script->str_data[func].val == script->buildin_lang_macro_fmtstring_offset)
			script->syntax.lang_macro_fmtstring_active = false;
	}

	if (!macro) {
		if (0 == --script->syntax.nested_call)
			script->syntax.last_func = -1;
		script->addc(C_FUNC);
	}
	return p;
}

/// Processes end of logical script line.
/// @param first When true, only fix up scheduling data is initialized
/// @param p Script position for error reporting in set_label
static void parse_nextline(bool first, const char *p)
{
	if( !first )
	{
		script->addc(C_EOL);  // mark end of line for stack cleanup
		script->set_label(LABEL_NEXTLINE, VECTOR_LENGTH(script->buf), p);  // fix up '-' labels
	}

	// initialize data for new '-' label fix up scheduling
	script->str_data[LABEL_NEXTLINE].type      = C_NOP;
	script->str_data[LABEL_NEXTLINE].backpatch = -1;
	script->str_data[LABEL_NEXTLINE].label     = -1;
}

/**
 * Pushes a variable into stack, processing its array index if needed.
 * @see parse_variable
 */
static void parse_variable_sub_push(int word, const char *p2)
{
	if( p2 ) {
		const char* p3 = NULL;
		// process the variable index

		// push the getelementofarray method into the stack
		script->addl(script->buildin_getelementofarray_ref);
		script->addc(C_ARG);
		script->addl(word);

		// process the sub-expression for this assignment
		p3 = script->parse_subexpr(p2 + 1, 1);
		p3 = script->skip_space(p3);

		if( *p3 != ']' ) {// closing parenthesis is required for this script
			disp_error_message("Missing closing ']' parenthesis for the variable assignment.", p3);
		}

		// push the closing function stack operator onto the stack
		script->addc(C_FUNC);
		p3++;
	} else {
		// No array index, simply push the variable or value onto the stack
		script->addl(word);
	}
}

/// Parse a variable assignment using the direct equals operator
/// @param p script position where the function should run from
/// @return NULL if not a variable assignment, the new position otherwise
static const char *parse_variable(const char *p)
{
	int word;
	c_op type = C_NOP;
	const char *p2 = NULL;
	const char *var = p;

	nullpo_retr(NULL, p);
	if( ( p[0] == '+' && p[1] == '+' && (type = C_ADD_PRE, true) ) // pre ++
	 || ( p[0] == '-' && p[1] == '-' && (type = C_SUB_PRE, true) ) // pre --
	) {
		var = p = script->skip_space(&p[2]);
	}

	// skip the variable where applicable
	p = script->skip_word(p);
	p = script->skip_space(p);

	if( p == NULL ) {
		// end of the line or invalid buffer
		return NULL;
	}

	if (*p == '[') {
		int i, j;
		// array variable so process the array as appropriate
		for (p2 = p, i = 0, j = 1; p; ++ i) {
			if( *p ++ == ']' && --(j) == 0 ) break;
			if( *p == '[' ) ++ j;
		}

		if( !(p = script->skip_space(p)) ) {
			// end of line or invalid characters remaining
			disp_error_message("Missing right expression or closing bracket for variable.", p);
		}
	}

	if( type == C_NOP &&
	!( ( p[0] == '=' && p[1] != '=' && (type = C_EQ, true) ) // =
	|| ( p[0] == '+' && p[1] == '=' && (type = C_ADD, true) ) // +=
	|| ( p[0] == '-' && p[1] == '=' && (type = C_SUB, true) ) // -=
	|| ( p[0] == '^' && p[1] == '=' && (type = C_XOR, true) ) // ^=
	|| ( p[0] == '|' && p[1] == '=' && (type = C_OR, true) ) // |=
	|| ( p[0] == '&' && p[1] == '=' && (type = C_AND, true) ) // &=
	|| ( p[0] == '*' && p[1] == '=' && (type = C_MUL, true) ) // *=
	|| ( p[0] == '*' && p[1] == '*' && p[2] == '=' && (type = C_POW, true) ) // **=
	|| ( p[0] == '/' && p[1] == '=' && (type = C_DIV, true) ) // /=
	|| ( p[0] == '%' && p[1] == '=' && (type = C_MOD, true) ) // %=
	|| ( p[0] == '+' && p[1] == '+' && (type = C_ADD_POST, true) ) // post ++
	|| ( p[0] == '-' && p[1] == '-' && (type = C_SUB_POST, true) ) // post --
	|| ( p[0] == '<' && p[1] == '<' && p[2] == '=' && (type = C_L_SHIFT, true) ) // <<=
	|| ( p[0] == '>' && p[1] == '>' && p[2] == '=' && (type = C_R_SHIFT, true) ) // >>=
	) )
	{// failed to find a matching operator combination so invalid
		return NULL;
	}

	switch( type ) {
		case C_ADD_PRE: // pre ++
		case C_SUB_PRE: // pre --
			// (nothing more to skip)
			break;

		case C_EQ: // =
			p = script->skip_space( &p[1] );
			break;

		case C_L_SHIFT: // <<=
		case C_R_SHIFT: // >>=
		case C_POW: // **=
			p = script->skip_space( &p[3] );
			break;

		default: // everything else
			p = script->skip_space( &p[2] );
	}

	if( p == NULL ) {
		// end of line or invalid buffer
		return NULL;
	}

	// push the set function onto the stack
	script->syntax.nested_call++;
	script->syntax.last_func = script->str_data[script->buildin_set_ref].val;
	script->addl(script->buildin_set_ref);
	script->addc(C_ARG);

	// always append parenthesis to avoid errors
	script->syntax.curly[script->syntax.curly_count].type = TYPE_ARGLIST;
	script->syntax.curly[script->syntax.curly_count].count = 0;
	script->syntax.curly[script->syntax.curly_count].flag = ARGLIST_PAREN;

	// increment the total curly count for the position in the script
	++script->syntax.curly_count;

	// parse the variable currently being modified
	word = script->add_word(var);

	if( script->str_data[word].type == C_FUNC
	 || script->str_data[word].type == C_USERFUNC
	 || script->str_data[word].type == C_USERFUNC_POS
	) {
		// cannot assign a variable which exists as a function or label
		disp_error_message("Cannot modify a variable which has the same name as a function or label.", p);
	}

	parse_variable_sub_push(word, p2); // Push variable onto the stack

	if( type != C_EQ ) {
		parse_variable_sub_push(word, p2); // Push variable onto the stack once again (first argument of setr)
	}

	if( type == C_ADD_POST || type == C_SUB_POST ) { // post ++ / --
		script->addi(1);
		script->addc(type == C_ADD_POST ? C_ADD : C_SUB);

		parse_variable_sub_push(word, p2); // Push variable onto the stack (third argument of setr)
	} else if( type == C_ADD_PRE || type == C_SUB_PRE ) { // pre ++ / --
		script->addi(1);
		script->addc(type == C_ADD_PRE ? C_ADD : C_SUB);
	} else {
		// process the value as an expression
		p = script->parse_subexpr(p, -1);

		if( type != C_EQ ) {
			// push the type of modifier onto the stack
			script->addc(type);
		}
	}

	// decrement the curly count for the position within the script
	--script->syntax.curly_count;

	// close the script by appending the function operator
	script->addc(C_FUNC);
	if (--script->syntax.nested_call == 0)
		script->syntax.last_func = -1;

	// push the buffer from the method
	return p;
}

/*
 * Checks whether the gives string is a number literal
 *
 * Mainly necessary to differentiate between number literals and NPC name
 * constants, since several of those start with a digit.
 *
 * All this does is to check if the string begins with an optional + or - sign,
 * followed by a hexadecimal or decimal number literal literal and is NOT
 * followed by a underscore or letter.
 *
 * @param p Pointer to the string to check
 * @return Whether the string is a number literal
 */
static bool is_number(const char *p)
{
	const char *np;
	if (!p)
		return false;
	if (*p == '-' || *p == '+')
		p++;
	np = p;
	if (*p == '0' && p[1] == 'x') {
		p+=2;
		np = p;
		// Hexadecimal
		while (ISXDIGIT(*np))
			np++;
	} else {
		// Decimal
		while (ISDIGIT(*np))
			np++;
	}
	if (p != np && *np != '_' && !ISALPHA(*np)) // At least one digit, and next isn't a letter or _
		return true;
	return false;
}

/**
 * Duplicates a script string into the script string list.
 *
 * Grows the script string list as needed.
 *
 * @param str The string to insert.
 * @return the string position in the script string list.
 */
static int script_string_dup(char *str)
{
	int len;
	int pos = script->string_list_pos;

	nullpo_retr(pos, str);
	len = (int)strlen(str);

	while (pos+len+1 >= script->string_list_size) {
		script->string_list_size += (1024*1024)/2;
		RECREATE(script->string_list,char,script->string_list_size);
	}

	safestrncpy(script->string_list+pos, str, len+1);
	script->string_list_pos += len+1;

	return pos;
}

/*==========================================
 * Analysis section
 *------------------------------------------*/
static const char *parse_simpleexpr(const char *p)
{
	p=script->skip_space(p);

	nullpo_retr(NULL, p);
	if (*p == ';' || *p == ',')
		disp_error_message("parse_simpleexpr: unexpected end of expression",p);
	if (*p == '(') {
		return script->parse_simpleexpr_paren(p);
	} else if (is_number(p)) {
		return script->parse_simpleexpr_number(p);
	} else if(*p == '"') {
		return script->parse_simpleexpr_string(p);
	} else {
		return script->parse_simpleexpr_name(p);
	}
}

static const char *parse_simpleexpr_paren(const char *p)
{
	int i = script->syntax.curly_count - 1;
	nullpo_retr(NULL, p);
	if (i >= 0 && script->syntax.curly[i].type == TYPE_ARGLIST)
		++script->syntax.curly[i].count;

	p = script->parse_subexpr(p + 1, -1);
	p = script->skip_space(p);
	if ((i = script->syntax.curly_count - 1) >= 0
	 && script->syntax.curly[i].type == TYPE_ARGLIST
	 && script->syntax.curly[i].flag == ARGLIST_UNDEFINED
	 && --script->syntax.curly[i].count == 0
	) {
		if (*p == ',') {
			script->syntax.curly[i].flag = ARGLIST_PAREN;
			return p;
		} else {
			script->syntax.curly[i].flag = ARGLIST_NO_PAREN;
		}
	}
	if (*p != ')')
		disp_error_message("parse_simpleexpr: unmatched ')'", p);

	return p + 1;
}

static const char *parse_simpleexpr_number(const char *p)
{
	char *np = NULL;
	long long lli;

	nullpo_retr(NULL, p);
	while (*p == '0' && ISDIGIT(p[1]))
		p++; // Skip leading zeros, we don't support octal literals

	lli = strtoll(p, &np, 0);
	if (lli < INT_MIN) {
		lli = INT_MIN;
		script->disp_warning_message("parse_simpleexpr: underflow detected, capping value to INT_MIN", p);
	} else if (lli > INT_MAX) {
		lli = INT_MAX;
		script->disp_warning_message("parse_simpleexpr: overflow detected, capping value to INT_MAX", p);
	}
	script->addi((int)lli); // Cast is safe, as it's already been checked for overflows

	return np;
}

static const char *parse_simpleexpr_string(const char *p)
{
	const char *start_point = p;

	nullpo_retr(NULL, p);
	do {
		p++;
		while (*p != '\0' && *p != '"') {
			if ((unsigned char)p[-1] <= 0x7e && *p == '\\') {
				char buf[8];
				size_t len = sv->skip_escaped_c(p) - p;
				size_t n = sv->unescape_c(buf, p, len);
				if (n != 1)
					ShowDebug("parse_simpleexpr: unexpected length %d after unescape (\"%.*s\" -> %.*s)\n", (int)n, (int)len, p, (int)n, buf);
				p += len;
				VECTOR_ENSURE(script->parse_simpleexpr_strbuf, 1, 512);
				VECTOR_PUSH(script->parse_simpleexpr_strbuf, buf[0]);
				continue;
			}
			if (*p == '\n') {
				disp_error_message("parse_simpleexpr: unexpected newline @ string", p);
			}
			VECTOR_ENSURE(script->parse_simpleexpr_strbuf, 1, 512);
			VECTOR_PUSH(script->parse_simpleexpr_strbuf, *p++);
		}
		if (*p == '\0')
			disp_error_message("parse_simpleexpr: unexpected end of file @ string", p);
		p++; //'"'
		p = script->skip_space(p);
	} while (*p != '\0' && *p == '"');

	VECTOR_ENSURE(script->parse_simpleexpr_strbuf, 1, 512);
	VECTOR_PUSH(script->parse_simpleexpr_strbuf, '\0');

	script->add_translatable_string(&script->parse_simpleexpr_strbuf, start_point);

	VECTOR_TRUNCATE(script->parse_simpleexpr_strbuf);

	return p;
}

static const char *parse_simpleexpr_name(const char *p)
{
	int l;
	const char *pv = NULL;

	// label , register , function etc
	if (script->skip_word(p) == p)
		disp_error_message("parse_simpleexpr: unexpected character", p);

	l = script->add_word(p);
	if (script->str_data[l].type == C_FUNC || script->str_data[l].type == C_USERFUNC || script->str_data[l].type == C_USERFUNC_POS) {
		return script->parse_callfunc(p,1,0);
#ifdef SCRIPT_CALLFUNC_CHECK
	} else {
		const char *name = script->get_str(l);
		if (strdb_get(script->userfunc_db,name) != NULL) {
			return script->parse_callfunc(p, 1, 1);
		}
#endif
	}

	if ((pv = script->parse_variable(p)) != NULL) {
		// successfully processed a variable assignment
		return pv;
	}

	if (script->str_data[l].type == C_INT && script->str_data[l].deprecated) {
		disp_warning_message("This constant is deprecated and it will be removed in a future version. Please see the script documentation and constants.conf for an alternative.\n", p);
	}

	p = script->skip_word(p);
	if (*p == '[') {
		// array(name[i] => getelementofarray(name,i) )
		script->addl(script->buildin_getelementofarray_ref);
		script->addc(C_ARG);
		script->addl(l);

		p = script->parse_subexpr(p + 1, -1);
		p = script->skip_space(p);
		if (*p != ']')
			disp_error_message("parse_simpleexpr: unmatched ']'", p);
		++p;
		script->addc(C_FUNC);
	} else if (script->str_data[l].type == C_INT) {
		script->addc(C_NAME);
		script->addb(l);
		script->addb(l >> 8);
		script->addb(l >> 16);
	} else {
		script->addl(l);
	}

	return p;
}

static void script_add_translatable_string(const struct script_string_buf *string, const char *start_point)
{
	struct string_translation *st = NULL;

	nullpo_retv(string);
	if (script->syntax.translation_db == NULL
	 || (st = strdb_get(script->syntax.translation_db, VECTOR_DATA(*string))) == NULL) {
		script->addc(C_STR);

		VECTOR_ENSURE(script->buf, VECTOR_LENGTH(*string), SCRIPT_BLOCK_SIZE);

		VECTOR_PUSHARRAY(script->buf, VECTOR_DATA(*string), VECTOR_LENGTH(*string));
	} else {
		unsigned char u;
		int st_cursor = 0;

		script->addc(C_LSTR);

		VECTOR_ENSURE(script->buf, (int)(sizeof(st->string_id) + sizeof(st->translations)), SCRIPT_BLOCK_SIZE);
		VECTOR_PUSHARRAY(script->buf, (void *)&st->string_id, sizeof(st->string_id));
		VECTOR_PUSHARRAY(script->buf, (void *)&st->translations, sizeof(st->translations));

		for (u = 0; u != st->translations; u++) {
			struct string_translation_entry *entry = (void *)(st->buf+st_cursor);
			char *stringptr = &entry->string[0];
			st_cursor += sizeof(*entry);
			VECTOR_ENSURE(script->buf, (int)(sizeof(entry->lang_id) + sizeof(char *)), SCRIPT_BLOCK_SIZE);
			VECTOR_PUSHARRAY(script->buf, (void *)&entry->lang_id, sizeof(entry->lang_id));
			VECTOR_PUSHARRAY(script->buf, (void *)&stringptr, sizeof(stringptr));
			st_cursor += sizeof(uint8); // FIXME: What are we skipping here?
			while (st->buf[st_cursor++] != 0)
				(void)0; // Skip string
			st_cursor += sizeof(uint8); // FIXME: What are we skipping here?
		}
	}
}

/*==========================================
 * Analysis of the expression
 *------------------------------------------*/
static const char *script_parse_subexpr(const char *p, int limit)
{
	int op,opl,len;

	nullpo_retr(NULL, p);
	p=script->skip_space(p);

	if( *p == '-' ) {
		const char *tmpp = script->skip_space(p+1);
		if( *tmpp == ';' || *tmpp == ',' ) {
			script->addl(LABEL_NEXTLINE);
			p++;
			return p;
		}
	}

	if( (p[0]=='+' && p[1]=='+') /* C_ADD_PRE */ || (p[0]=='-'&&p[1]=='-') /* C_SUB_PRE */ ) { // Pre ++ -- operators
		p=script->parse_variable(p);
	} else if( (op=C_NEG,*p=='-') || (op=C_LNOT,*p=='!') || (op=C_NOT,*p=='~') ) { // Unary - ! ~ operators
		p=script->parse_subexpr(p+1,11);
		script->addc(op);
	} else {
		p=script->parse_simpleexpr(p);
	}
	p=script->skip_space(p);
	while((
	   (op=C_OP3,    opl=0, len=1,*p=='?')              // ?:
	|| (op=C_ADD,    opl=9, len=1,*p=='+' && p[1]!='+') // +
	|| (op=C_SUB,    opl=9, len=1,*p=='-' && p[1]!='-') // -
	|| (op=C_POW,    opl=11,len=2,*p=='*' && p[1]=='*') // **
	|| (op=C_MUL,    opl=10,len=1,*p=='*')              // *
	|| (op=C_DIV,    opl=10,len=1,*p=='/')              // /
	|| (op=C_MOD,    opl=10,len=1,*p=='%')              // %
	|| (op=C_LAND,   opl=2, len=2,*p=='&' && p[1]=='&') // &&
	|| (op=C_AND,    opl=5, len=1,*p=='&')              // &
	|| (op=C_LOR,    opl=1, len=2,*p=='|' && p[1]=='|') // ||
	|| (op=C_OR,     opl=3, len=1,*p=='|')              // |
	|| (op=C_XOR,    opl=4, len=1,*p=='^')              // ^
	|| (op=C_EQ,     opl=6, len=2,*p=='=' && p[1]=='=') // ==
	|| (op=C_NE,     opl=6, len=2,*p=='!' && p[1]=='=') // !=
	|| (op=C_RE_EQ,  opl=6, len=2,*p=='~' && p[1]=='=') // ~=
	|| (op=C_RE_NE,  opl=6, len=2,*p=='~' && p[1]=='!') // ~!
	|| (op=C_R_SHIFT,opl=8, len=2,*p=='>' && p[1]=='>') // >>
	|| (op=C_GE,     opl=7, len=2,*p=='>' && p[1]=='=') // >=
	|| (op=C_GT,     opl=7, len=1,*p=='>')              // >
	|| (op=C_L_SHIFT,opl=8, len=2,*p=='<' && p[1]=='<') // <<
	|| (op=C_LE,     opl=7, len=2,*p=='<' && p[1]=='=') // <=
	|| (op=C_LT,     opl=7, len=1,*p=='<')              // <
	) && opl>limit) {
		p+=len;
		if(op == C_OP3) {
			p=script->parse_subexpr(p,-1);
			p=script->skip_space(p);
			if( *(p++) != ':')
				disp_error_message("parse_subexpr: need ':'", p-1);
			p=script->parse_subexpr(p,-1);
		} else {
			p=script->parse_subexpr(p,opl);
		}
		script->addc(op);
		p=script->skip_space(p);
	}

	return p;  /* return first untreated operator */
}

/*==========================================
 * Evaluation of the expression
 *------------------------------------------*/
static const char *parse_expr(const char *p)
{
	nullpo_retr(NULL, p);
	switch(*p) {
	case ')': case ';': case ':': case '[': case ']':
	case '}':
		disp_error_message("parse_expr: unexpected char",p);
	}
	p=script->parse_subexpr(p,-1);
	return p;
}

/*==========================================
 * Analysis of the line
 *------------------------------------------*/
static const char *parse_line(const char *p)
{
	const char* p2;

	nullpo_retr(NULL, p);
	p=script->skip_space(p);
	if(*p==';') {
		//Close decision for if(); for(); while();
		p = script->parse_syntax_close(p + 1);
		return p;
	}
	if(*p==')' && script->parse_syntax_for_flag)
		return p+1;

	p = script->skip_space(p);
	if(p[0] == '{') {
		script->syntax.curly[script->syntax.curly_count].type  = TYPE_NULL;
		script->syntax.curly[script->syntax.curly_count].count = -1;
		script->syntax.curly[script->syntax.curly_count].index = -1;
		script->syntax.curly_count++;
		return p + 1;
	} else if(p[0] == '}') {
		return script->parse_curly_close(p);
	}

	// Syntax-related processing
	p2 = script->parse_syntax(p);
	if(p2 != NULL)
		return p2;

	// attempt to process a variable assignment
	p2 = script->parse_variable(p);

	if (p2 != NULL) {
		// variable assignment processed so leave the method
		if (script->parse_syntax_for_flag) {
			if (*p2 != ')')
				disp_error_message("parse_line: need ')'", p2);
		} else {
			if (*p2 != ';')
				disp_error_message("parse_line: need ';'", p2);
		}
		return script->parse_syntax_close(p2 + 1);
	}

	p = script->parse_callfunc(p,0,0);
	p = script->skip_space(p);

	if(script->parse_syntax_for_flag) {
		if( *p != ')' )
			disp_error_message("parse_line: need ')'",p);
	} else {
		if( *p != ';' )
			disp_error_message("parse_line: need ';'",p);
	}

	//Binding decision for if(), for(), while()
	p = script->parse_syntax_close(p+1);

	return p;
}

// { ... } Closing process
static const char *parse_curly_close(const char *p)
{
	nullpo_retr(NULL, p);
	if(script->syntax.curly_count <= 0) {
		disp_error_message("parse_curly_close: unexpected string",p);
		return p + 1;
	} else if(script->syntax.curly[script->syntax.curly_count-1].type == TYPE_NULL) {
		script->syntax.curly_count--;
		//Close decision  if, for , while
		p = script->parse_syntax_close(p + 1);
		return p;
	} else if(script->syntax.curly[script->syntax.curly_count-1].type == TYPE_SWITCH) {
		//Closing switch()
		int pos = script->syntax.curly_count-1;
		char label[256];
		int l;
		// Remove temporary variables
		sprintf(label, "__setr $@__SW%x_VAL,0;", (unsigned int)script->syntax.curly[pos].index);
		script->syntax.curly[script->syntax.curly_count++].type = TYPE_NULL;
		script->parse_line(label);
		script->syntax.curly_count--;

		// Go to the end pointer unconditionally
		sprintf(label,"goto __SW%x_FIN;", (unsigned int)script->syntax.curly[pos].index);
		script->syntax.curly[script->syntax.curly_count++].type = TYPE_NULL;
		script->parse_line(label);
		script->syntax.curly_count--;

		// You are here labeled
		sprintf(label,"__SW%x_%x", (unsigned int)script->syntax.curly[pos].index, (unsigned int)script->syntax.curly[pos].count);
		l=script->add_str(label);
		script->set_label(l, VECTOR_LENGTH(script->buf), p);

		if(script->syntax.curly[pos].flag) {
			//Exists default
			sprintf(label,"goto __SW%x_DEF;", (unsigned int)script->syntax.curly[pos].index);
			script->syntax.curly[script->syntax.curly_count++].type = TYPE_NULL;
			script->parse_line(label);
			script->syntax.curly_count--;
		}

		// Label end
		sprintf(label,"__SW%x_FIN", (unsigned int)script->syntax.curly[pos].index);
		l=script->add_str(label);
		script->set_label(l, VECTOR_LENGTH(script->buf), p);
		linkdb_final(&script->syntax.curly[pos].case_label); // free the list of case label
		script->syntax.curly_count--;
		//Closing decision if, for , while
		p = script->parse_syntax_close(p + 1);
		return p;
	} else {
		disp_error_message("parse_curly_close: unexpected string",p);
		return p + 1;
	}
}

// Syntax-related processing
// break, case, continue, default, do, for, function,
// if, switch, while ? will handle this internally.
static const char *parse_syntax(const char *p)
{
	const char *p2 = script->skip_word(p);

	nullpo_retr(NULL, p);
	switch(*p) {
	case 'B':
	case 'b':
		if( p2 - p == 5 && strncmp(p,"break",5) == 0 ) {
			// break Processing
			char label[256];
			int pos = script->syntax.curly_count - 1;
			while(pos >= 0) {
				if(script->syntax.curly[pos].type == TYPE_DO) {
					sprintf(label, "goto __DO%x_FIN;", (unsigned int)script->syntax.curly[pos].index);
					break;
				} else if(script->syntax.curly[pos].type == TYPE_FOR) {
					sprintf(label, "goto __FR%x_FIN;", (unsigned int)script->syntax.curly[pos].index);
					break;
				} else if(script->syntax.curly[pos].type == TYPE_WHILE) {
					sprintf(label, "goto __WL%x_FIN;", (unsigned int)script->syntax.curly[pos].index);
					break;
				} else if(script->syntax.curly[pos].type == TYPE_SWITCH) {
					sprintf(label, "goto __SW%x_FIN;", (unsigned int)script->syntax.curly[pos].index);
					break;
				}
				pos--;
			}
			if(pos < 0) {
				disp_error_message("parse_syntax: unexpected 'break'",p);
			} else {
				script->syntax.curly[script->syntax.curly_count++].type = TYPE_NULL;
				script->parse_line(label);
				script->syntax.curly_count--;
			}
			p = script->skip_space(p2);
			if(*p != ';')
				disp_error_message("parse_syntax: need ';'",p);
			// Closing decision if, for , while
			p = script->parse_syntax_close(p + 1);
			return p;
		}
		break;
	case 'c':
	case 'C':
		if( p2 - p == 4 && strncmp(p, "case", 4) == 0 ) {
			//Processing case
			int pos = script->syntax.curly_count-1;
			if(pos < 0 || script->syntax.curly[pos].type != TYPE_SWITCH) {
				disp_error_message("parse_syntax: unexpected 'case' ",p);
				return p+1;
			} else {
				char label[256];
				int  l,v;
				char *np;
				if(script->syntax.curly[pos].count != 1) {
					//Jump for FALLTHRU
					sprintf(label,"goto __SW%x_%xJ;", (unsigned int)script->syntax.curly[pos].index, (unsigned int)script->syntax.curly[pos].count);
					script->syntax.curly[script->syntax.curly_count++].type = TYPE_NULL;
					script->parse_line(label);
					script->syntax.curly_count--;

					// You are here labeled
					sprintf(label,"__SW%x_%x", (unsigned int)script->syntax.curly[pos].index, (unsigned int)script->syntax.curly[pos].count);
					l=script->add_str(label);
					script->set_label(l, VECTOR_LENGTH(script->buf), p);
				}
				//Decision statement switch
				p = script->skip_space(p2);
				if(p == p2) {
					disp_error_message("parse_syntax: expect space ' '",p);
				}
				// check whether case label is integer or not
				if(is_number(p)) {
					//Numeric value
					v = (int)strtol(p,&np,0);
					if((*p == '-' || *p == '+') && ISDIGIT(p[1])) // pre-skip because '-' can not skip_word
						p++;
					p = script->skip_word(p);
					if(np != p)
						disp_error_message("parse_syntax: 'case' label is not an integer",np);
				} else {
					//Check for constants
					p2 = script->skip_word(p);
					v = (int)(size_t) (p2-p); // length of word at p2
					memcpy(label,p,v);
					label[v]='\0';
					if( !script->get_constant(label, &v) )
						disp_error_message("parse_syntax: 'case' label is not an integer",p);
					p = script->skip_word(p);
				}
				p = script->skip_space(p);
				if(*p != ':')
					disp_error_message("parse_syntax: expect ':'",p);
				sprintf(label,"if(%d != $@__SW%x_VAL) goto __SW%x_%x;",
					v, (unsigned int)script->syntax.curly[pos].index, (unsigned int)script->syntax.curly[pos].index, (unsigned int)script->syntax.curly[pos].count+1);
				script->syntax.curly[script->syntax.curly_count++].type = TYPE_NULL;
				// Bad I do not parse twice
				p2 = script->parse_line(label);
				script->parse_line(p2);
				script->syntax.curly_count--;
				if(script->syntax.curly[pos].count != 1) {
					// Label after the completion of FALLTHRU
					sprintf(label, "__SW%x_%xJ", (unsigned int)script->syntax.curly[pos].index, (unsigned int)script->syntax.curly[pos].count);
					l=script->add_str(label);
					script->set_label(l, VECTOR_LENGTH(script->buf), p);
				}
				// check duplication of case label [Rayce]
				if(linkdb_search(&script->syntax.curly[pos].case_label, (void*)h64BPTRSIZE(v)) != NULL)
					disp_error_message("parse_syntax: dup 'case'",p);
				linkdb_insert(&script->syntax.curly[pos].case_label, (void*)h64BPTRSIZE(v), (void*)1);

				sprintf(label, "__setr $@__SW%x_VAL,0;", (unsigned int)script->syntax.curly[pos].index);
				script->syntax.curly[script->syntax.curly_count++].type = TYPE_NULL;

				script->parse_line(label);
				script->syntax.curly_count--;
				script->syntax.curly[pos].count++;
			}
			return p + 1;
		} else if( p2 - p == 8 && strncmp(p, "continue", 8) == 0 ) {
			// Processing continue
			char label[256];
			int pos = script->syntax.curly_count - 1;
			while(pos >= 0) {
				if(script->syntax.curly[pos].type == TYPE_DO) {
					sprintf(label, "goto __DO%x_NXT;", (unsigned int)script->syntax.curly[pos].index);
					script->syntax.curly[pos].flag = 1; //Flag put the link for continue
					break;
				} else if(script->syntax.curly[pos].type == TYPE_FOR) {
					sprintf(label, "goto __FR%x_NXT;", (unsigned int)script->syntax.curly[pos].index);
					break;
				} else if(script->syntax.curly[pos].type == TYPE_WHILE) {
					sprintf(label, "goto __WL%x_NXT;", (unsigned int)script->syntax.curly[pos].index);
					break;
				}
				pos--;
			}
			if(pos < 0) {
				disp_error_message("parse_syntax: unexpected 'continue'",p);
			} else {
				script->syntax.curly[script->syntax.curly_count++].type = TYPE_NULL;
				script->parse_line(label);
				script->syntax.curly_count--;
			}
			p = script->skip_space(p2);
			if(*p != ';')
				disp_error_message("parse_syntax: need ';'",p);
			//Closing decision if, for , while
			p = script->parse_syntax_close(p + 1);
			return p;
		}
		break;
	case 'd':
	case 'D':
		if( p2 - p == 7 && strncmp(p, "default", 7) == 0 ) {
			// Switch - default processing
			int pos = script->syntax.curly_count-1;
			if(pos < 0 || script->syntax.curly[pos].type != TYPE_SWITCH) {
				disp_error_message("parse_syntax: unexpected 'default'",p);
			} else if(script->syntax.curly[pos].flag) {
				disp_error_message("parse_syntax: dup 'default'",p);
			} else {
				char label[256];
				int l;
				// Put the label location
				p = script->skip_space(p2);
				if(*p != ':') {
					disp_error_message("parse_syntax: need ':'",p);
				}
				sprintf(label, "__SW%x_%x", (unsigned int)script->syntax.curly[pos].index, (unsigned int)script->syntax.curly[pos].count);
				l=script->add_str(label);
				script->set_label(l, VECTOR_LENGTH(script->buf), p);

				// Skip to the next link w/o condition
				sprintf(label, "goto __SW%x_%x;", (unsigned int)script->syntax.curly[pos].index, (unsigned int)script->syntax.curly[pos].count + 1);
				script->syntax.curly[script->syntax.curly_count++].type = TYPE_NULL;
				script->parse_line(label);
				script->syntax.curly_count--;

				// The default label
				sprintf(label, "__SW%x_DEF", (unsigned int)script->syntax.curly[pos].index);
				l=script->add_str(label);
				script->set_label(l, VECTOR_LENGTH(script->buf), p);

				script->syntax.curly[script->syntax.curly_count - 1].flag = 1;
				script->syntax.curly[pos].count++;
			}
			return p + 1;
		} else if( p2 - p == 2 && strncmp(p, "do", 2) == 0 ) {
			int l;
			char label[256];
			p=script->skip_space(p2);

			script->syntax.curly[script->syntax.curly_count].type  = TYPE_DO;
			script->syntax.curly[script->syntax.curly_count].count = 1;
			script->syntax.curly[script->syntax.curly_count].index = script->syntax.index++;
			script->syntax.curly[script->syntax.curly_count].flag  = 0;
			// Label of the (do) form here
			sprintf(label, "__DO%x_BGN", (unsigned int)script->syntax.curly[script->syntax.curly_count].index);
			l=script->add_str(label);
			script->set_label(l, VECTOR_LENGTH(script->buf), p);
			script->syntax.curly_count++;
			return p;
		}
		break;
	case 'f':
	case 'F':
		if( p2 - p == 3 && strncmp(p, "for", 3) == 0 ) {
			int l;
			char label[256];
			int  pos = script->syntax.curly_count;
			script->syntax.curly[script->syntax.curly_count].type  = TYPE_FOR;
			script->syntax.curly[script->syntax.curly_count].count = 1;
			script->syntax.curly[script->syntax.curly_count].index = script->syntax.index++;
			script->syntax.curly[script->syntax.curly_count].flag  = 0;
			script->syntax.curly_count++;

			p=script->skip_space(p2);

			if(*p != '(')
				disp_error_message("parse_syntax: need '('",p);
			p++;

			// Execute the initialization statement
			script->syntax.curly[script->syntax.curly_count++].type = TYPE_NULL;
			p=script->parse_line(p);
			script->syntax.curly_count--;

			// Form the start of label decision
			sprintf(label, "__FR%x_J", (unsigned int)script->syntax.curly[pos].index);
			l=script->add_str(label);
			script->set_label(l, VECTOR_LENGTH(script->buf), p);

			p=script->skip_space(p);
			if(*p == ';') {
				// For (; Because the pattern of always true ;)
				;
			} else {
				// Skip to the end point if the condition is false
				sprintf(label, "__FR%x_FIN", (unsigned int)script->syntax.curly[pos].index);
				script->addl(script->add_str("__jump_zero"));
				script->addc(C_ARG);
				p=script->parse_expr(p);
				p=script->skip_space(p);
				script->addl(script->add_str(label));
				script->addc(C_FUNC);
			}
			if(*p != ';')
				disp_error_message("parse_syntax: need ';'",p);
			p++;

			// Skip to the beginning of the loop
			sprintf(label, "goto __FR%x_BGN;", (unsigned int)script->syntax.curly[pos].index);
			script->syntax.curly[script->syntax.curly_count++].type = TYPE_NULL;
			script->parse_line(label);
			script->syntax.curly_count--;

			// Labels to form the next loop
			sprintf(label, "__FR%x_NXT", (unsigned int)script->syntax.curly[pos].index);
			l=script->add_str(label);
			script->set_label(l, VECTOR_LENGTH(script->buf), p);

			// Process the next time you enter the loop
			// A ')' last for; flag to be treated as'
			script->parse_syntax_for_flag = 1;
			script->syntax.curly[script->syntax.curly_count++].type = TYPE_NULL;
			p=script->parse_line(p);
			script->syntax.curly_count--;
			script->parse_syntax_for_flag = 0;

			// Skip to the determination process conditions
			sprintf(label, "goto __FR%x_J;", (unsigned int)script->syntax.curly[pos].index);
			script->syntax.curly[script->syntax.curly_count++].type = TYPE_NULL;
			script->parse_line(label);
			script->syntax.curly_count--;

			// Loop start labeling
			sprintf(label, "__FR%x_BGN", (unsigned int)script->syntax.curly[pos].index);
			l=script->add_str(label);
			script->set_label(l, VECTOR_LENGTH(script->buf), p);
			return p;
		} else if( p2 - p == 8 && strncmp(p, "function", 8) == 0 ) {
			// internal script function
			const char *func_name;

			func_name = script->skip_space(p2);
			p = script->skip_word(func_name);
			if( p == func_name )
				disp_error_message("parse_syntax:function: function name is missing or invalid", p);
			p2 = script->skip_space(p);
			if( *p2 == ';' )
			{// function <name> ;
				// function declaration - just register the name
				int l;
				l = script->add_word(func_name);
				if( script->str_data[l].type == C_NOP )// register only, if the name was not used by something else
					script->str_data[l].type = C_USERFUNC;
				else if( script->str_data[l].type == C_USERFUNC )
					;  // already registered
				else
					disp_error_message("parse_syntax:function: function name is invalid", func_name);

				// Close condition of if, for, while
				p = script->parse_syntax_close(p2 + 1);
				return p;
			}
			else if(*p2 == '{')
			{// function <name> <line/block of code>
				char label[256];
				int l;

				script->syntax.curly[script->syntax.curly_count].type  = TYPE_USERFUNC;
				script->syntax.curly[script->syntax.curly_count].count = 1;
				script->syntax.curly[script->syntax.curly_count].index = script->syntax.index++;
				script->syntax.curly[script->syntax.curly_count].flag  = 0;
				++script->syntax.curly_count;

				// Jump over the function code
				sprintf(label, "goto __FN%x_FIN;", (unsigned int)script->syntax.curly[script->syntax.curly_count-1].index);
				script->syntax.curly[script->syntax.curly_count].type = TYPE_NULL;
				++script->syntax.curly_count;
				script->parse_line(label);
				--script->syntax.curly_count;

				// Set the position of the function (label)
				l=script->add_word(func_name);
				if( script->str_data[l].type == C_NOP || script->str_data[l].type == C_USERFUNC )// register only, if the name was not used by something else
				{
					script->str_data[l].type = C_USERFUNC;
					script->set_label(l, VECTOR_LENGTH(script->buf), p);
					if( script->parse_options&SCRIPT_USE_LABEL_DB )
						script->label_add(l, VECTOR_LENGTH(script->buf));
				}
				else
					disp_error_message("parse_syntax:function: function name is invalid", func_name);

				return script->skip_space(p);
			}
			else
			{
				disp_error_message("expect ';' or '{' at function syntax",p);
			}
		}
		break;
	case 'i':
	case 'I':
		if( p2 - p == 2 && strncmp(p, "if", 2) == 0 ) {
			// If process
			char label[256];
			p=script->skip_space(p2);
			if(*p != '(') { //Prevent if this {} non-c script->syntax. from Rayce (jA)
				disp_error_message("need '('",p);
			}
			script->syntax.curly[script->syntax.curly_count].type  = TYPE_IF;
			script->syntax.curly[script->syntax.curly_count].count = 1;
			script->syntax.curly[script->syntax.curly_count].index = script->syntax.index++;
			script->syntax.curly[script->syntax.curly_count].flag  = 0;
			sprintf(label, "__IF%x_%x", (unsigned int)script->syntax.curly[script->syntax.curly_count].index, (unsigned int)script->syntax.curly[script->syntax.curly_count].count);
			script->syntax.curly_count++;
			script->addl(script->add_str("__jump_zero"));
			script->addc(C_ARG);
			p=script->parse_expr(p);
			p=script->skip_space(p);
			script->addl(script->add_str(label));
			script->addc(C_FUNC);
			return p;
		}
		break;
	case 's':
	case 'S':
		if( p2 - p == 6 && strncmp(p, "switch", 6) == 0 ) {
			// Processing of switch ()
			char label[256];
			p=script->skip_space(p2);
			if(*p != '(') {
				disp_error_message("need '('",p);
			}
			script->syntax.curly[script->syntax.curly_count].type  = TYPE_SWITCH;
			script->syntax.curly[script->syntax.curly_count].count = 1;
			script->syntax.curly[script->syntax.curly_count].index = script->syntax.index++;
			script->syntax.curly[script->syntax.curly_count].flag  = 0;
			sprintf(label, "$@__SW%x_VAL", (unsigned int)script->syntax.curly[script->syntax.curly_count].index);
			script->syntax.curly_count++;
			script->addl(script->add_str("__setr"));
			script->addc(C_ARG);
			script->addl(script->add_str(label));
			p=script->parse_expr(p);
			p=script->skip_space(p);
			if(*p != '{') {
				disp_error_message("parse_syntax: need '{'",p);
			}
			script->addc(C_FUNC);
			return p + 1;
		}
		break;
	case 'w':
	case 'W':
		if( p2 - p == 5 && strncmp(p, "while", 5) == 0 ) {
			int l;
			char label[256];
			p=script->skip_space(p2);
			if(*p != '(') {
				disp_error_message("need '('",p);
			}
			script->syntax.curly[script->syntax.curly_count].type  = TYPE_WHILE;
			script->syntax.curly[script->syntax.curly_count].count = 1;
			script->syntax.curly[script->syntax.curly_count].index = script->syntax.index++;
			script->syntax.curly[script->syntax.curly_count].flag  = 0;
			// Form the start of label decision
			sprintf(label, "__WL%x_NXT", (unsigned int)script->syntax.curly[script->syntax.curly_count].index);
			l=script->add_str(label);
			script->set_label(l, VECTOR_LENGTH(script->buf), p);

			// Skip to the end point if the condition is false
			sprintf(label, "__WL%x_FIN", (unsigned int)script->syntax.curly[script->syntax.curly_count].index);
			script->syntax.curly_count++;
			script->addl(script->add_str("__jump_zero"));
			script->addc(C_ARG);
			p=script->parse_expr(p);
			p=script->skip_space(p);
			script->addl(script->add_str(label));
			script->addc(C_FUNC);
			return p;
		}
		break;
	}
	return NULL;
}

static const char *parse_syntax_close(const char *p)
{
	// If (...) for (...) hoge (); as to make sure closed closed once again
	int flag;

	nullpo_retr(NULL, p);
	do {
		p = script->parse_syntax_close_sub(p,&flag);
	} while(flag);
	return p;
}

// Close judgment if, for, while, of do
// flag == 1 : closed
// flag == 0 : not closed
static const char *parse_syntax_close_sub(const char *p, int *flag)
{
	char label[256];
	int pos = script->syntax.curly_count - 1;
	int l;
	*flag = 1;

	if(script->syntax.curly_count <= 0) {
		*flag = 0;
		return p;
	} else if(script->syntax.curly[pos].type == TYPE_IF) {
		const char *bp = p;
		const char *p2;

		// if-block and else-block end is a new line
		script->parse_nextline(false, p);

		// Skip to the last location if
		sprintf(label, "goto __IF%x_FIN;", (unsigned int)script->syntax.curly[pos].index);
		script->syntax.curly[script->syntax.curly_count++].type = TYPE_NULL;
		script->parse_line(label);
		script->syntax.curly_count--;

		// Put the label of the location
		sprintf(label, "__IF%x_%x", (unsigned int)script->syntax.curly[pos].index, (unsigned int)script->syntax.curly[pos].count);
		l=script->add_str(label);
		script->set_label(l, VECTOR_LENGTH(script->buf), p);

		script->syntax.curly[pos].count++;
		p = script->skip_space(p);
		p2 = script->skip_word(p);
		if( !script->syntax.curly[pos].flag && p2 - p == 4 && strncmp(p, "else", 4) == 0 ) {
			// else  or else - if
			p = script->skip_space(p2);
			p2 = script->skip_word(p);
			if( p2 - p == 2 && strncmp(p, "if", 2) == 0 ) {
				// else - if
				p=script->skip_space(p2);
				if(*p != '(') {
					disp_error_message("need '('",p);
				}
				sprintf(label, "__IF%x_%x", (unsigned int)script->syntax.curly[pos].index, (unsigned int)script->syntax.curly[pos].count);
				script->addl(script->add_str("__jump_zero"));
				script->addc(C_ARG);
				p=script->parse_expr(p);
				p=script->skip_space(p);
				script->addl(script->add_str(label));
				script->addc(C_FUNC);
				*flag = 0;
				return p;
			} else {
				// else
				if(!script->syntax.curly[pos].flag) {
					script->syntax.curly[pos].flag = 1;
					*flag = 0;
					return p;
				}
			}
		}
		// Close if
		script->syntax.curly_count--;
		// Put the label of the final location
		sprintf(label, "__IF%x_FIN", (unsigned int)script->syntax.curly[pos].index);
		l=script->add_str(label);
		script->set_label(l, VECTOR_LENGTH(script->buf), p);
		if(script->syntax.curly[pos].flag == 1) {
			// Because the position of the pointer is the same if not else for this
			return bp;
		}
		return p;
	} else if(script->syntax.curly[pos].type == TYPE_DO) {
		const char *p2;

		if(script->syntax.curly[pos].flag) {
			// (Come here continue) to form the label here
			sprintf(label, "__DO%x_NXT", (unsigned int)script->syntax.curly[pos].index);
			l=script->add_str(label);
			script->set_label(l, VECTOR_LENGTH(script->buf), p);
		}

		// Skip to the end point if the condition is false
		p = script->skip_space(p);
		p2 = script->skip_word(p);
		if( p2 - p != 5 || strncmp(p, "while", 5) != 0 ) {
			disp_error_message("parse_syntax: need 'while'",p);
		}

		p = script->skip_space(p2);
		if(*p != '(') {
			disp_error_message("need '('",p);
		}

		// do-block end is a new line
		script->parse_nextline(false, p);

		sprintf(label, "__DO%x_FIN", (unsigned int)script->syntax.curly[pos].index);
		script->addl(script->add_str("__jump_zero"));
		script->addc(C_ARG);
		p=script->parse_expr(p);
		p=script->skip_space(p);
		script->addl(script->add_str(label));
		script->addc(C_FUNC);

		// Skip to the starting point
		sprintf(label, "goto __DO%x_BGN;", (unsigned int)script->syntax.curly[pos].index);
		script->syntax.curly[script->syntax.curly_count++].type = TYPE_NULL;
		script->parse_line(label);
		script->syntax.curly_count--;

		// Form label of the end point conditions
		sprintf(label, "__DO%x_FIN", (unsigned int)script->syntax.curly[pos].index);
		l=script->add_str(label);
		script->set_label(l, VECTOR_LENGTH(script->buf), p);
		p = script->skip_space(p);
		if(*p != ';') {
			disp_error_message("parse_syntax: need ';'",p);
			return p+1;
		}
		p++;
		script->syntax.curly_count--;
		return p;
	} else if(script->syntax.curly[pos].type == TYPE_FOR) {
		// for-block end is a new line
		script->parse_nextline(false, p);

		// Skip to the next loop
		sprintf(label, "goto __FR%x_NXT;", (unsigned int)script->syntax.curly[pos].index);
		script->syntax.curly[script->syntax.curly_count++].type = TYPE_NULL;
		script->parse_line(label);
		script->syntax.curly_count--;

		// End for labeling
		sprintf(label, "__FR%x_FIN", (unsigned int)script->syntax.curly[pos].index);
		l=script->add_str(label);
		script->set_label(l, VECTOR_LENGTH(script->buf), p);
		script->syntax.curly_count--;
		return p;
	} else if(script->syntax.curly[pos].type == TYPE_WHILE) {
		// while-block end is a new line
		script->parse_nextline(false, p);

		// Skip to the decision while
		sprintf(label, "goto __WL%x_NXT;", (unsigned int)script->syntax.curly[pos].index);
		script->syntax.curly[script->syntax.curly_count++].type = TYPE_NULL;
		script->parse_line(label);
		script->syntax.curly_count--;

		// End while labeling
		sprintf(label, "__WL%x_FIN", (unsigned int)script->syntax.curly[pos].index);
		l=script->add_str(label);
		script->set_label(l, VECTOR_LENGTH(script->buf), p);
		script->syntax.curly_count--;
		return p;
	} else if(script->syntax.curly[pos].type == TYPE_USERFUNC) {
		// Back
		sprintf(label,"return;");
		script->syntax.curly[script->syntax.curly_count++].type = TYPE_NULL;
		script->parse_line(label);
		script->syntax.curly_count--;

		// Put the label of the location
		sprintf(label, "__FN%x_FIN", (unsigned int)script->syntax.curly[pos].index);
		l=script->add_str(label);
		script->set_label(l, VECTOR_LENGTH(script->buf), p);
		script->syntax.curly_count--;
		return p;
	} else {
		*flag = 0;
		return p;
	}
}

/// Retrieves the value of a constant.
static bool script_get_constant(const char *name, int *value)
{
	int n = script->search_str(name);

	nullpo_retr(false, value);
	if( n == -1 || script->str_data[n].type != C_INT )
	{// not found or not a constant
		return false;
	}
	value[0] = script->str_data[n].val;
	if (script->str_data[n].deprecated) {
		ShowWarning("The constant '%s' is deprecated and it will be removed in a future version. Please see the script documentation and constants.conf for an alternative.\n", name);
	}

	return true;
}

/// Creates new constant or parameter with given value.
static void script_set_constant(const char *name, int value, bool is_parameter, bool is_deprecated)
{
	int n = script->add_str(name);

	if (script->str_data[n].type == C_NOP) {
		script->str_data[n].type = is_parameter ? C_PARAM : C_INT;
		script->str_data[n].val  = value;
		script->str_data[n].deprecated = is_deprecated ? 1 : 0;
	} else if( script->str_data[n].type == C_PARAM || script->str_data[n].type == C_INT ) {// existing parameter or constant
		ShowError("script_set_constant: Attempted to overwrite existing %s '%s' (old value=%d, new value=%d).\n", ( script->str_data[n].type == C_PARAM ) ? "parameter" : "constant", name, script->str_data[n].val, value);
	} else {// existing name
		ShowError("script_set_constant: Invalid name for %s '%s' (already defined as %s).\n", is_parameter ? "parameter" : "constant", name, script->op2name(script->str_data[n].type));
	}
}
/* adds data to a existent constant in the database, inserted normally via parse */
static void script_set_constant2(const char *name, int value, bool is_parameter, bool is_deprecated)
{
	int n = script->add_str(name);

	if( script->str_data[n].type == C_PARAM ) {
		ShowError("script_set_constant2: Attempted to overwrite existing parameter '%s' with a constant (value=%d).\n", name, value);
		return;
	}

	if( script->str_data[n].type == C_NAME && script->str_data[n].val ) {
		ShowWarning("script_set_constant2: Attempted to overwrite existing variable '%s' with a constant (value=%d).\n", name, value);
		return;
	}

	if( script->str_data[n].type == C_INT && value && value != script->str_data[n].val ) { // existing constant
		ShowWarning("script_set_constant2: Attempted to overwrite existing constant '%s' (old value=%d, new value=%d).\n", name, script->str_data[n].val, value);
		return;
	}

	if( script->str_data[n].type != C_NOP ) {
		script->str_data[n].func = NULL;
		script->str_data[n].backpatch = -1;
		script->str_data[n].label = -1;
	}

	script->str_data[n].type = is_parameter ? C_PARAM : C_INT;
	script->str_data[n].val  = value;
	script->str_data[n].deprecated = is_deprecated ? 1 : 0;
}

/**
 * Loads the constants database from constants.conf
 */
static void read_constdb(bool reload)
{
	struct config_t constants_conf;
	char filepath[256];
	struct config_setting_t *cdb;
	struct config_setting_t *t;
	int i = 0;

	safesnprintf(filepath, 256, "%s/constants.conf", map->db_path);

	if (!libconfig->load_file(&constants_conf, filepath))
		return;

	if ((cdb = libconfig->setting_get_member(constants_conf.root, "constants_db")) == NULL) {
		ShowError("can't read %s\n", filepath);
		return;
	}

	while ((t = libconfig->setting_get_elem(cdb, i++))) {
		bool is_deprecated = false;
		int value = 0;
		const char *name = config_setting_name(t);
		const char *p = name;

		while (*p != '\0') {
			if (!ISALNUM(*p) && *p != '_')
				break;
			p++;
		}
		if (*p != '\0') {
			ShowWarning("read_constdb: Invalid constant name %s. Skipping.\n", name);
			continue;
		}
		if (strcmp(name, "comment__") == 0) {
			const char *comment = libconfig->setting_get_string(t);
			if (comment == NULL)
				continue;
			if (*comment == '\0')
				comment = NULL;
			script->constdb_comment(comment);
			continue;
		}
		if (config_setting_is_aggregate(t)) {
			int i32;
			if (!libconfig->setting_lookup_int(t, "Value", &i32)) {
				ShowWarning("read_constdb: Invalid entry for %s. Skipping.\n", name);
				continue;
			}
			value = i32;
			if (libconfig->setting_lookup_bool(t, "Deprecated", &i32)) {
				if (i32 != 0)
					is_deprecated = true;
			}
		} else {
			value = libconfig->setting_get_int(t);
		}

		if (reload) {
			int n = script->add_str(name);
			script->str_data[n].type = C_NOP; // ensures it will be overwritten
		}

		script->set_constant(name, value, false, is_deprecated);
	}
	script->constdb_comment(NULL);
	libconfig->destroy(&constants_conf);
}

/**
 * Sets the current constdb comment.
 *
 * This function does nothing (used by plugins only)
 *
 * @param comment The comment to set (NULL to unset)
 */
static void script_constdb_comment(const char *comment)
{
	(void)comment;
}

static void script_load_parameters(void)
{
	int i = 0;
	struct {
		char *name;
		enum status_point_types type;
	} parameters[] = {
		{"BaseExp", SP_BASEEXP},
		{"JobExp", SP_JOBEXP},
		{"Karma", SP_KARMA},
		{"Manner", SP_MANNER},
		{"Hp", SP_HP},
		{"MaxHp", SP_MAXHP},
		{"Sp", SP_SP},
		{"MaxSp", SP_MAXSP},
		{"StatusPoint", SP_STATUSPOINT},
		{"BaseLevel", SP_BASELEVEL},
		{"SkillPoint", SP_SKILLPOINT},
		{"Class", SP_CLASS},
		{"Zeny", SP_ZENY},
		{"BankVault", SP_BANKVAULT},
		{"Sex", SP_SEX},
		{"NextBaseExp", SP_NEXTBASEEXP},
		{"NextJobExp", SP_NEXTJOBEXP},
		{"Weight", SP_WEIGHT},
		{"MaxWeight", SP_MAXWEIGHT},
		{"JobLevel", SP_JOBLEVEL},
		{"Upper", SP_UPPER},
		{"BaseJob", SP_BASEJOB},
		{"BaseClass", SP_BASECLASS},
		{"killerrid", SP_KILLERRID},
		{"killedrid", SP_KILLEDRID},
		{"SlotChange", SP_SLOTCHANGE},
		{"CharRename", SP_CHARRENAME},
		{"ModExp", SP_MOD_EXP},
		{"ModDrop", SP_MOD_DROP},
		{"ModDeath", SP_MOD_DEATH},
	};

	script->constdb_comment("Parameters");
	for (i=0; i < ARRAYLENGTH(parameters); ++i)
		script->set_constant(parameters[i].name, parameters[i].type, true, false);
	script->constdb_comment(NULL);
}
// Standard UNIX tab size is 8
#define TAB_SIZE 8
#define update_tabstop(tabstop,chars) \
	do { \
		(tabstop) -= (chars); \
		while ((tabstop) <= 0) (tabstop) += TAB_SIZE; \
	} while (false)

/*==========================================
 * Display emplacement line of script
 *------------------------------------------*/
static const char *script_print_line(StringBuf *buf, const char *p, const char *mark, int line)
{
	int i, mark_pos = 0, tabstop = TAB_SIZE;
	if( p == NULL || !p[0] ) return NULL;
	if( line < 0 )
		StrBuf->Printf(buf, "*%5d: ", -line);         // len = 8
	else
		StrBuf->Printf(buf, " %5d: ", line);          // len = 8
	update_tabstop(tabstop,8);                            // len = 8
	for( i=0; p[i] && p[i] != '\n'; i++ ) {
		char c = p[i];
		int w = 1;
		// Like Clang does, let's print the code with tabs expanded to spaces to ensure that the marker will be under the right character
		if( c == '\t' ) {
			c = ' ';
			w = tabstop;
		}
		update_tabstop(tabstop, w);
		if( p + i < mark)
			mark_pos += w;
		if( p + i != mark)
			StrBuf->Printf(buf, "%*c", w, c);
		else
			StrBuf->Printf(buf, CL_BT_RED"%*c"CL_RESET, w, c);
	}
	StrBuf->AppendStr(buf, "\n");
	if( mark ) {
		StrBuf->AppendStr(buf, "        "CL_BT_CYAN); // len = 8
		for( ; mark_pos > 0; mark_pos-- ) {
			StrBuf->AppendStr(buf, "~");
		}
		StrBuf->AppendStr(buf, CL_RESET CL_BT_GREEN"^"CL_RESET"\n");
	}
	return p+i+(p[i] == '\n' ? 1 : 0);
}
#undef TAB_SIZE
#undef update_tabstop

#define CONTEXTLINES 3
static void script_errorwarning_sub(StringBuf *buf, const char *src, const char *file, int start_line, const char *error_msg, const char *error_pos)
{
	// Find the line where the error occurred
	int j;
	int line = start_line;
	const char *p, *error_linepos;
	const char *linestart[CONTEXTLINES] = { NULL };

	for(p=src;p && *p;line++) {
		const char *lineend=strchr(p,'\n');
		if(lineend==NULL || error_pos<lineend) {
			break;
		}
		for( j = 0; j < CONTEXTLINES-1; j++ ) {
			linestart[j] = linestart[j+1];
		}
		linestart[CONTEXTLINES-1] = p;
		p = lineend+1;
	}
	error_linepos = p;

	if( line >= 0 )
		StrBuf->Printf(buf, "script error in file '%s' line %d column %"PRIdPTR"\n", file, line, error_pos-error_linepos+1);
	else
		StrBuf->Printf(buf, "script error in file '%s' item ID %d\n", file, -line);

	StrBuf->Printf(buf, "    %s\n", error_msg);
	for(j = 0; j < CONTEXTLINES; j++ ) {
		script->print_line(buf, linestart[j], NULL, line + j - CONTEXTLINES);
	}
	p = script->print_line(buf, p, error_pos, -line);
	for(j = 0; j < CONTEXTLINES; j++) {
		p = script->print_line(buf, p, NULL, line + j + 1 );
	}
}
#undef CONTEXTLINES

static void script_error(const char *src, const char *file, int start_line, const char *error_msg, const char *error_pos)
{
	StringBuf buf;

	StrBuf->Init(&buf);
	StrBuf->AppendStr(&buf, "\a");

	script->errorwarning_sub(&buf, src, file, start_line, error_msg, error_pos);

	ShowError("%s", StrBuf->Value(&buf));
	StrBuf->Destroy(&buf);
}

static void script_warning(const char *src, const char *file, int start_line, const char *error_msg, const char *error_pos)
{
	StringBuf buf;

	StrBuf->Init(&buf);

	script->errorwarning_sub(&buf, src, file, start_line, error_msg, error_pos);

	ShowWarning("%s", StrBuf->Value(&buf));
	StrBuf->Destroy(&buf);
}

/*==========================================
 * Analysis of the script
 *------------------------------------------*/
static struct script_code *parse_script(const char *src, const char *file, int line, int options, int *retval)
{
	const char *p,*tmpp;
	int i;
	struct script_code* code = NULL;
	char end;
	bool unresolved_names = false;

	script->parser_current_src = src;
	script->parser_current_file = file;
	script->parser_current_line = line;

	if( src == NULL )
		return NULL;// empty script

	if( script->parse_cleanup_timer_id == INVALID_TIMER ) {
		script->parse_cleanup_timer_id = timer->add(timer->gettick() + 10, script->parse_cleanup_timer, 0, 0);
	}

	memset(&script->syntax,0,sizeof(script->syntax));
	script->syntax.last_func = -1;/* as valid values are >= 0 */
	if( script->parser_current_npc_name ) {
		if( !script->translation_db )
			script->load_translations();
		if( script->translation_db )
			script->syntax.translation_db = strdb_get(script->translation_db, script->parser_current_npc_name);
	}

	VECTOR_TRUNCATE(script->buf);
	script->parse_nextline(true, NULL);

	// who called parse_script is responsible for clearing the database after using it, but just in case... lets clear it here
	if( options&SCRIPT_USE_LABEL_DB )
		script->label_count = 0;
	script->parse_options = options;

	if( setjmp( script->error_jump ) != 0 ) {
		//Restore program state when script has problems. [from jA]
		const int size = ARRAYLENGTH(script->syntax.curly);
		if( script->error_report )
			script->error(src,file,line,script->error_msg,script->error_pos);
		aFree( script->error_msg );
		VECTOR_TRUNCATE(script->buf);
		for(i=LABEL_START;i<script->str_num;i++)
			if(script->str_data[i].type == C_NOP) script->str_data[i].type = C_NAME;
		for(i=0; i<size; i++)
			linkdb_final(&script->syntax.curly[i].case_label);
#ifdef ENABLE_CASE_CHECK
		script->local_casecheck.clear();
		script->parser_current_src = NULL;
		script->parser_current_file = NULL;
		script->parser_current_line = 0;
#endif // ENABLE_CASE_CHECK
		if (retval) *retval = EXIT_FAILURE;
		return NULL;
	}

	script->parse_syntax_for_flag=0;
	p=src;
	p=script->skip_space(p);
	if( options&SCRIPT_IGNORE_EXTERNAL_BRACKETS )
	{// does not require brackets around the script
		if (*p == '\0' && !(options&SCRIPT_RETURN_EMPTY_SCRIPT)) {
			// empty script and can return NULL
			VECTOR_TRUNCATE(script->buf);
#ifdef ENABLE_CASE_CHECK
			script->local_casecheck.clear();
			script->parser_current_src = NULL;
			script->parser_current_file = NULL;
			script->parser_current_line = 0;
#endif // ENABLE_CASE_CHECK
			return NULL;
		}
		end = '\0';
	}
	else
	{// requires brackets around the script
		if( *p != '{' ) {
			disp_error_message("not found '{'",p);
			if (retval) *retval = EXIT_FAILURE;
		}
		p = script->skip_space(p+1);
		if (*p == '}' && !(options&SCRIPT_RETURN_EMPTY_SCRIPT)) {
			// empty script and can return NULL
			VECTOR_TRUNCATE(script->buf);
#ifdef ENABLE_CASE_CHECK
			script->local_casecheck.clear();
			script->parser_current_src = NULL;
			script->parser_current_file = NULL;
			script->parser_current_line = 0;
#endif // ENABLE_CASE_CHECK
			return NULL;
		}
		end = '}';
	}

	// clear references of labels, variables and internal functions
	for(i=LABEL_START;i<script->str_num;i++) {
		if(
			script->str_data[i].type==C_POS || script->str_data[i].type==C_NAME ||
			script->str_data[i].type==C_USERFUNC || script->str_data[i].type == C_USERFUNC_POS
		  ) {
			script->str_data[i].type=C_NOP;
			script->str_data[i].backpatch=-1;
			script->str_data[i].label=-1;
		}
	}

	while( script->syntax.curly_count != 0 || *p != end )
	{
		if( *p == '\0' )
			disp_error_message("unexpected end of script",p);
		// Special handling only label
		tmpp=script->skip_space(script->skip_word(p));
		if(*tmpp==':' && !(strncmp(p,"default:",8) == 0 && p + 7 == tmpp)) {
			i=script->add_word(p);
			script->set_label(i, VECTOR_LENGTH(script->buf), p);
			if( script->parse_options&SCRIPT_USE_LABEL_DB )
				script->label_add(i, VECTOR_LENGTH(script->buf));
			p=tmpp+1;
			p=script->skip_space(p);
			continue;
		}

		// All other lumped
		p=script->parse_line(p);
		p=script->skip_space(p);

		script->parse_nextline(false, p);
	}

	script->addc(C_NOP);

	// default unknown references to variables
	for (i = LABEL_START; i < script->str_num; i++) {
		if (script->str_data[i].type == C_NOP) {
			int j;
			script->str_data[i].type=C_NAME;
			script->str_data[i].label=i;
			for (j = script->str_data[i].backpatch; j >= 0 && j != 0x00ffffff; ) {
				int next = GETVALUE(&script->buf, j);
				SETVALUE(&script->buf, j, i);
				j = next;
			}
		} else if(script->str_data[i].type == C_USERFUNC) {
			// 'function name;' without follow-up code
			ShowError("parse_script: function '%s' declared but not defined.\n", script->str_buf+script->str_data[i].str);
			if (retval) *retval = EXIT_FAILURE;
			unresolved_names = true;
		}
	}

	if( unresolved_names ) {
		disp_error_message("parse_script: unresolved function references", p);
		if (retval) *retval = EXIT_FAILURE;
	}

#ifdef SCRIPT_DEBUG_DISP
	for (i = 0; i < VECTOR_LENGTH(script->buf); i++) {
		if ((i&15) == 0)
			ShowMessage("%04x : ",i);
		ShowMessage("%02x ", VECTOR_INDEX(script->buf, i));
		if ((i&15) == 15)
			ShowMessage("\n");
	}
	ShowMessage("\n");
#endif
#ifdef SCRIPT_DEBUG_DISASM
	i = 0;
	while (i < VECTOR_LENGTH(script->buf)) {
		c_op op = script->get_com(&script->buf, &i);
		int j = i; // Note: i is modified in the line above.

		ShowMessage("%06x %s", i, script->op2name(op));

		switch (op) {
		case C_INT:
			ShowMessage(" %d", script->get_num(&script->buf, &i));
			break;
		case C_POS:
			ShowMessage(" 0x%06x", *(int*)(&VECTOR_INDEX(script->buf, i))&0xffffff);
			i += 3;
			break;
		case C_NAME:
			j = (*(int*)(&VECTOR_INDEX(script->buf, i))&0xffffff);
			ShowMessage(" %s", ( j == 0xffffff ) ? "?? unknown ??" : script->get_str(j));
			i += 3;
			break;
		case C_STR:
			j = (int)strlen((char*)&VECTOR_INDEX(script->buf, i));
			ShowMessage(" %s", &VECTOR_INDEX(script->buf, i));
			i += j+1;
			break;
		}
		ShowMessage(CL_CLL"\n");
	}
#endif

	CREATE(code,struct script_code,1);
	VECTOR_INIT(code->script_buf);
	VECTOR_ENSURE(code->script_buf, VECTOR_LENGTH(script->buf), 1);
	VECTOR_PUSHARRAY(code->script_buf, VECTOR_DATA(script->buf), VECTOR_LENGTH(script->buf));
	code->local.vars = NULL;
	code->local.arrays = NULL;
#ifdef ENABLE_CASE_CHECK
	script->local_casecheck.clear();
	script->parser_current_src = NULL;
	script->parser_current_file = NULL;
	script->parser_current_line = 0;
#endif // ENABLE_CASE_CHECK
	return code;
}

/// Returns the player attached to this script, identified by the rid.
/// If there is no player attached, the script is terminated.
static struct map_session_data *script_rid2sd(struct script_state *st)
{
	struct map_session_data *sd;
	nullpo_retr(NULL, st);
	if( !( sd = map->id2sd(st->rid) ) ) {
		ShowError("script_rid2sd: fatal error ! player not attached!\n");
		script->reportfunc(st);
		script->reportsrc(st);
		st->state = END;
	}
	return sd;
}

static struct map_session_data *script_id2sd(struct script_state *st, int account_id)
{
	struct map_session_data *sd;
	if ((sd = map->id2sd(account_id)) == NULL) {
		ShowWarning("script_id2sd: Player with account ID '%d' not found!\n", account_id);
		script->reportfunc(st);
		script->reportsrc(st);
	}
	return sd;
}

static struct map_session_data *script_charid2sd(struct script_state *st, int char_id)
{
	struct map_session_data *sd;
	if ((sd = map->charid2sd(char_id)) == NULL) {
		ShowWarning("script_charid2sd: Player with char ID '%d' not found!\n", char_id);
		script->reportfunc(st);
		script->reportsrc(st);
	}
	return sd;
}

static struct map_session_data *script_nick2sd(struct script_state *st, const char *name)
{
	struct map_session_data *sd;
	if ((sd = map->nick2sd(name, false)) == NULL) {
		ShowWarning("script_nick2sd: Player name '%s' not found!\n", name);
		script->reportfunc(st);
		script->reportsrc(st);
	}
	return sd;
}

static char *get_val_npcscope_str(struct script_state *st, struct reg_db *n, struct script_data *data)
{
	if (n)
		return (char*)i64db_get(n->vars, reference_getuid(data));
	else
		return NULL;
}

static char *get_val_pc_ref_str(struct script_state *st, struct reg_db *n, struct script_data *data)
{
	struct script_reg_str *p = NULL;
	nullpo_retr(NULL, n);

	p = i64db_get(n->vars, reference_getuid(data));
	return p ? p->value : NULL;
}

static char *get_val_instance_str(struct script_state *st, const char *name, struct script_data *data)
{
	nullpo_retr(NULL, st);
	if (st->instance_id >= 0) {
		return (char*)i64db_get(instance->list[st->instance_id].regs.vars, reference_getuid(data));
	} else {
		ShowWarning("script_get_val: cannot access instance variable '%s', defaulting to \"\"\n", name);
		return NULL;
	}
}

static int get_val_npcscope_num(struct script_state *st, struct reg_db *n, struct script_data *data)
{
	if (n)
		return (int)i64db_iget(n->vars, reference_getuid(data));
	else
		return 0;
}

static int get_val_pc_ref_num(struct script_state *st, struct reg_db *n, struct script_data *data)
{
	struct script_reg_num *p = NULL;
	nullpo_retr(0, n);

	p = i64db_get(n->vars, reference_getuid(data));
	return p ? p->value : 0;
}

static int get_val_instance_num(struct script_state *st, const char *name, struct script_data *data)
{
	if (st->instance_id >= 0)
		return (int)i64db_iget(instance->list[st->instance_id].regs.vars, reference_getuid(data));
	else {
		ShowWarning("script_get_val: cannot access instance variable '%s', defaulting to 0\n", name);
		return 0;
	}
}

/**
 * Dereferences a variable/constant, replacing it with a copy of the value.
 *
 * @param st[in]       script state.
 * @param data[in,out] variable/constant.
 * @return pointer to data, for convenience.
 */
static struct script_data *get_val(struct script_state *st, struct script_data *data)
{
	const char* name;
	char prefix;
	char postfix;
	struct map_session_data *sd = NULL;

	if (!data_isreference(data))
		return data;// not a variable/constant

	name = reference_getname(data);
	prefix = name[0];
	postfix = name[strlen(name) - 1];

	if (strlen(name) > SCRIPT_VARNAME_LENGTH) {
		ShowError("script_get_val: variable name too long. '%s'\n", name);
		script->reportsrc(st);
		st->state = END;
		return data;
	}

	if (((reference_tovariable(data) && not_server_variable(prefix)) || reference_toparam(data)) && reference_getref(data) == NULL) {
		sd = script->rid2sd(st);
		if (sd == NULL) {// needs player attached
			if (postfix == '$') {// string variable
				ShowWarning("script_get_val: cannot access player variable '%s', defaulting to \"\"\n", name);
				data->type = C_CONSTSTR;
				data->u.str = "";
			} else {// integer variable
				ShowWarning("script_get_val: cannot access player variable '%s', defaulting to 0\n", name);
				data->type = C_INT;
				data->u.num = 0;
			}
			return data;
		}
	}

	if (postfix == '$') {
		// string variable
		const char *str = NULL;

		switch (prefix) {
		case '@':
			if (data->ref) {
				str = script->get_val_ref_str(st, data->ref, data);
			} else {
				str = pc->readregstr(sd, data->u.num);
			}
			break;
		case '$':
			str = mapreg->readregstr(data->u.num);
			break;
		case '#':
			if (data->ref) {
				str = script->get_val_pc_ref_str(st, data->ref, data);
			} else if (name[1] == '#') {
				str = pc_readaccountreg2str(sd, data->u.num);// global
			} else {
				str = pc_readaccountregstr(sd, data->u.num);// local
			}
			break;
		case '.':
			if (data->ref) {
				str = script->get_val_ref_str(st, data->ref, data);
			} else if (name[1] == '@') {
				str = script->get_val_scope_str(st, &st->stack->scope, data);
			} else {
				str = script->get_val_npc_str(st, &st->script->local, data);
			}
			break;
		case '\'':
			str = script->get_val_instance_str(st, name, data);
			break;
		default:
			if (data->ref) {
				str = script->get_val_pc_ref_str(st, data->ref, data);
			} else {
				str = pc_readglobalreg_str(sd, data->u.num);
			}
			break;
		}

		if (str == NULL || str[0] == '\0') {
			// empty string
			data->type = C_CONSTSTR;
			data->u.str = "";
		} else {// duplicate string
			data->type = C_STR;
			data->u.mutstr = aStrdup(str);
		}

	} else {// integer variable

		data->type = C_INT;

		if( reference_toconstant(data) ) {
			data->u.num = reference_getconstant(data);
		} else if( reference_toparam(data) ) {
			data->u.num = pc->readparam(sd, reference_getparamtype(data));
		} else {
			switch (prefix) {
			case '@':
				if (data->ref) {
					data->u.num = script->get_val_ref_num(st, data->ref, data);
				} else {
					data->u.num = pc->readreg(sd, data->u.num);
				}
				break;
			case '$':
				data->u.num = mapreg->readreg(data->u.num);
				break;
			case '#':
				if (data->ref) {
					data->u.num = script->get_val_pc_ref_num(st, data->ref, data);
				} else if (name[1] == '#') {
					data->u.num = pc_readaccountreg2(sd, data->u.num);// global
				} else {
					data->u.num = pc_readaccountreg(sd, data->u.num);// local
				}
				break;
			case '.':
				if (data->ref) {
					data->u.num = script->get_val_ref_num(st, data->ref, data);
				} else if (name[1] == '@') {
					data->u.num = script->get_val_scope_num(st, &st->stack->scope, data);
				} else {
					data->u.num = script->get_val_npc_num(st, &st->script->local, data);
				}
				break;
			case '\'':
				data->u.num = script->get_val_instance_num(st, name, data);
				break;
			default:
				if (data->ref) {
					data->u.num = script->get_val_pc_ref_num(st, data->ref, data);
				} else {
					data->u.num = pc_readglobalreg(sd, data->u.num);
				}
				break;
			}
		}
	}
	data->ref = NULL;

	return data;
}

/**
 * Retrieves the value of a reference identified by uid (variable, constant, param)
 *
 * The value is left in the top of the stack and needs to be removed manually.
 *
 * @param st[in]  script state.
 * @param uid[in] reference identifier.
 * @param ref[in] the container to look up the reference into.
 * @return the retrieved value of the reference.
 */
static const void *get_val2(struct script_state *st, int64 uid, struct reg_db *ref)
{
	struct script_data* data;
	nullpo_retr(NULL, st);
	script->push_val(st->stack, C_NAME, uid, ref);
	data = script_getdatatop(st, -1);
	script->get_val(st, data);
	if (data->type == C_INT) // u.num is int32 because it comes from script->get_val
		return (const void *)h64BPTRSIZE((int32)data->u.num);
	else if (data_isreference(data) && reference_toconstant(data))
		return (const void *)h64BPTRSIZE((int32)reference_getconstant(data));
	else if (data_isreference(data) && reference_toparam(data))
		return (const void *)h64BPTRSIZE((int32)reference_getparamtype(data));
	else
		return (const void *)h64BPTRSIZE(data->u.str);
}
/**
 * Because, currently, array members with key 0 are indifferenciable from normal variables, we should ensure its actually in
 * Will be gone as soon as undefined var feature is implemented
 **/
static void script_array_ensure_zero(struct script_state *st, struct map_session_data *sd, int64 uid, struct reg_db *ref)
{
	const char *name = script->get_str(script_getvarid(uid));
	struct reg_db *src = NULL;
	bool insert = false;

	if (st == NULL) {
		// Special case with no st available, only sd
		nullpo_retv(sd);
		src = script->array_src(NULL, sd, name, ref);
		insert = true;
	} else {
		if (sd == NULL && st->rid != 0)
			sd = map->id2sd(st->rid); // Retrieve the missing sd
		src = script->array_src(st, sd, name, ref);
		if( is_string_variable(name) ) {
			const char *str = script->get_val2(st, uid, ref);
			if (str != NULL && *str != '\0')
				insert = true;
			script_removetop(st, -1, 0);
		} else {
			int32 num = (int32)h64BPTRSIZE(script->get_val2(st, uid, ref));
			if( num )
				insert = true;
			script_removetop(st, -1, 0);
		}
	}

	if (src && src->arrays) {
		struct script_array *sa = idb_get(src->arrays, script_getvarid(uid));
		if (sa) {
			unsigned int i;

			ARR_FIND(0, sa->size, i, sa->members[i] == 0);
			if( i != sa->size ) {
				if( !insert )
					script->array_remove_member(src,sa,i);
				return;
			}
			script->array_add_member(sa,0);
		} else if (insert) {
			script->array_update(src,reference_uid(script_getvarid(uid), 0),false);
		}
	}
}
/**
 * Returns array size by ID
 **/
static unsigned int script_array_size(struct script_state *st, struct map_session_data *sd, const char *name, struct reg_db *ref)
{
	struct script_array *sa = NULL;
	struct reg_db *src = script->array_src(st, sd, name, ref);

	if( src && src->arrays )
		sa = idb_get(src->arrays, script->search_str(name));

	return sa ? sa->size : 0;
}
/**
 * Returns array's highest key (for that awful getarraysize implementation that doesn't really gets the array size)
 **/
static unsigned int script_array_highest_key(struct script_state *st, struct map_session_data *sd, const char *name, struct reg_db *ref)
{
	struct script_array *sa = NULL;
	struct reg_db *src = script->array_src(st, sd, name, ref);

	if( src && src->arrays ) {
		int key = script->add_word(name);

		script->array_ensure_zero(st,sd,reference_uid(key, 0),ref);

		if( ( sa = idb_get(src->arrays, key) ) ) {
			unsigned int i, highest_key = 0;

			for(i = 0; i < sa->size; i++) {
				if( sa->members[i] > highest_key )
					highest_key = sa->members[i];
			}
			return sa->size ? highest_key + 1 : 0;
		}
	}
	return 0;
}
static int script_free_array_db(union DBKey key, struct DBData *data, va_list ap)
{
	struct script_array *sa = DB->data2ptr(data);
	aFree(sa->members);
	ers_free(script->array_ers, sa);
	return 0;
}
/**
 * Clears script_array and removes it from script->array_db
 **/
static void script_array_delete(struct reg_db *src, struct script_array *sa)
{
	nullpo_retv(src);
	nullpo_retv(sa);
	aFree(sa->members);
	idb_remove(src->arrays, sa->id);
	ers_free(script->array_ers, sa);
}
/**
 * Removes a member from a script_array list
 *
 * @param idx the index of the member in script_array struct list, not of the actual array member
 **/
static void script_array_remove_member(struct reg_db *src, struct script_array *sa, unsigned int idx)
{
	unsigned int i, cursor;

	nullpo_retv(sa);
	/* its the only member left, no need to do anything other than delete the array data */
	if( sa->size == 1 ) {
		script->array_delete(src,sa);
		return;
	}

	sa->members[idx] = UINT_MAX;

	for(i = 0, cursor = 0; i < sa->size; i++) {
		if( sa->members[i] == UINT_MAX )
			continue;
		if( i != cursor )
			sa->members[cursor] = sa->members[i];
		cursor++;
	}

	sa->size = cursor;
}
/**
 * Appends a new array index to the list in script_array
 *
 * @param idx the index of the array member being inserted
 **/
static void script_array_add_member(struct script_array *sa, unsigned int idx)
{
	nullpo_retv(sa);
	RECREATE(sa->members, unsigned int, ++sa->size);
	sa->members[sa->size - 1] = idx;
}
/**
 * Obtains the source of the array database for this type and scenario
 * Initializes such database when not yet initialized.
 **/
static struct reg_db *script_array_src(struct script_state *st, struct map_session_data *sd, const char *name, struct reg_db *ref)
{
	struct reg_db *src = NULL;
	nullpo_retr(NULL, name);

	switch (name[0]) {
		/* from player */
	default: /* char reg */
	case '@':/* temp char reg */
	case '#':/* account reg */
		if (ref != NULL) {
			src = ref;
		} else {
			nullpo_retr(NULL, sd);
			src = &sd->regs;
		}
		break;
	case '$':/* map reg */
		src = &mapreg->regs;
		break;
	case '.':/* npc/script */
		if (ref != NULL) {
			src = ref;
		} else {
			nullpo_retr(NULL, st);
			src = (name[1] == '@') ? &st->stack->scope : &st->script->local;
		}
		break;
	case '\'':/* instance */
		nullpo_retr(NULL, st);
		if (st->instance_id >= 0) {
			src = &instance->list[st->instance_id].regs;
		}
		break;
	}

	if (src) {
		if (!src->arrays) {
			src->arrays = idb_alloc(DB_OPT_BASE);
		}
		return src;
	}
	return NULL;
}

/**
 * Processes a array member modification, and update data accordingly
 *
 * @param src[in,out] Variable source database. If the array database doesn't exist, it is created.
 * @param num[in]     Variable ID
 * @param empty[in]   Whether the modified member is empty (needs to be removed)
 **/
static void script_array_update(struct reg_db *src, int64 num, bool empty)
{
	struct script_array *sa = NULL;
	int id = script_getvarid(num);
	unsigned int index = script_getvaridx(num);

	nullpo_retv(src);
	if (!src->arrays) {
		src->arrays = idb_alloc(DB_OPT_BASE);
	} else {
		sa = idb_get(src->arrays, id);
	}

	if( sa ) {
		unsigned int i;

		/* search */
		for(i = 0; i < sa->size; i++) {
			if( sa->members[i] == index )
				break;
		}

		/* if existent */
		if( i != sa->size ) {
			/* if empty, we gotta remove it */
			if( empty ) {
				script->array_remove_member(src, sa, i);
			}
		} else if( !empty ) { /* new entry */
			script->array_add_member(sa,index);
			/* we do nothing if its empty, no point in modifying array data for a new empty member */
		}
	} else if ( !empty ) {/* we only move to create if not empty */
		sa = ers_alloc(script->array_ers, struct script_array);
		sa->id = id;
		sa->members = NULL;
		sa->size = 0;
		script->array_add_member(sa,index);
		idb_put(src->arrays, id, sa);
	}
}

static void set_reg_npcscope_str(struct script_state *st, struct reg_db *n, int64 num, const char *name, const char *str)
{
	if (n)
	{
		nullpo_retv(str);
		if (str[0]) {
			i64db_put(n->vars, num, aStrdup(str));
			if (script_getvaridx(num))
				script->array_update(n, num, false);
		} else {
			i64db_remove(n->vars, num);
			if (script_getvaridx(num))
				script->array_update(n, num, true);
		}
	}
}

static void set_reg_pc_ref_str(struct script_state *st, struct reg_db *n, int64 num, const char *name, const char *str)
{
	struct DBIterator *iter = db_iterator(map->pc_db);

	for (struct map_session_data *sd = dbi_first(iter); dbi_exists(iter); sd = dbi_next(iter)) {
		if (sd != NULL && n == &sd->regs) {
			pc->setregistry_str(sd, num, str);
			break;
		}
	}
	dbi_destroy(iter);
}

static void set_reg_pc_ref_num(struct script_state *st, struct reg_db *n, int64 num, const char *name, int val)
{
	struct DBIterator *iter = db_iterator(map->pc_db);

	for (struct map_session_data *sd = dbi_first(iter); dbi_exists(iter); sd = dbi_next(iter)) {
		if (sd != NULL && n == &sd->regs) {
			pc->setregistry(sd, num, val);
			break;
		}
	}
	dbi_destroy(iter);
}

static void set_reg_npcscope_num(struct script_state *st, struct reg_db *n, int64 num, const char *name, int val)
{
	if (n) {
		if (val != 0) {
			i64db_iput(n->vars, num, val);
			if (script_getvaridx(num))
				script->array_update(n, num, false);
		} else {
			i64db_remove(n->vars, num);
			if (script_getvaridx(num))
				script->array_update(n, num, true);
		}
	}
}

static void set_reg_instance_str(struct script_state *st, int64 num, const char *name, const char *str)
{
	nullpo_retv(st);
	if (st->instance_id >= 0) {
		if (str[0]) {
			i64db_put(instance->list[st->instance_id].regs.vars, num, aStrdup(str));
			if (script_getvaridx(num))
				script->array_update(&instance->list[st->instance_id].regs, num, false);
		} else {
			i64db_remove(instance->list[st->instance_id].regs.vars, num);
			if (script_getvaridx(num))
				script->array_update(&instance->list[st->instance_id].regs, num, true);
		}
	} else {
		ShowError("script_set_reg: cannot write instance variable '%s', NPC not in a instance!\n", name);
		script->reportsrc(st);
	}
}

static void set_reg_instance_num(struct script_state *st, int64 num, const char *name, int val)
{
	nullpo_retv(st);
	if (st->instance_id >= 0) {
		if (val != 0) {
			i64db_iput(instance->list[st->instance_id].regs.vars, num, val);
			if (script_getvaridx(num))
				script->array_update(&instance->list[st->instance_id].regs, num, false);
		} else {
			i64db_remove(instance->list[st->instance_id].regs.vars, num);
			if (script_getvaridx(num))
				script->array_update(&instance->list[st->instance_id].regs, num, true);
		}
	} else {
		ShowError("script_set_reg: cannot write instance variable '%s', NPC not in a instance!\n", name);
		script->reportsrc(st);
	}
}

/**
 * Stores the value of a script variable
 *
 * @param st    current script state.
 * @param sd    current character data.
 * @param num   variable identifier.
 * @param name  variable name.
 * @param value new value.
 * @param ref   variable container, in case of a npc/scope variable reference outside the current scope.
 * @retval 0 failure.
 * @retval 1 success.
 *
 * TODO: return values are screwed up, have been for some time (reaad: years), e.g. some functions return 1 failure and success.
 *------------------------------------------*/
static int set_reg(struct script_state *st, struct map_session_data *sd, int64 num, const char *name, const void *value, struct reg_db *ref)
{
	char prefix;
	nullpo_ret(name);
	prefix = name[0];

	if (script->str_data[script_getvarid(num)].type != C_NAME && script->str_data[script_getvarid(num)].type != C_PARAM) {
		ShowError("script:set_reg: not a variable! '%s'\n", name);

		// to avoid this don't do script->add_str(") without setting its type.
		// either use script->add_variable() or manually set the type

		if (st) {
			script->reportsrc(st);
			st->state = END;
		}
		return 0;
	}

	if (strlen(name) > SCRIPT_VARNAME_LENGTH) {
		ShowError("script:set_reg: variable name too long. '%s'\n", name);
		if (st) {
			script->reportsrc(st);
			st->state = END;
		}
		return 0;
	}

	if (is_string_variable(name)) {// string variable
		const char *str = (const char*)value;

		switch (prefix) {
		case '@':
			if (ref) {
				script->set_reg_ref_str(st, ref, num, name, str);
			} else {
				pc->setregstr(sd, num, str);
			}
			return 1;
		case '$':
			mapreg->setregstr(num, str);
			return 1;
		case '#':
			if (ref) {
				script->set_reg_pc_ref_str(st, ref, num, name, str);
			} else if (name[1] == '#') {
				pc_setaccountreg2str(sd, num, str);
			} else {
				pc_setaccountregstr(sd, num, str);
			}
			return 1;
		case '.':
			if (ref) {
				script->set_reg_ref_str(st, ref, num, name, str);
			} else if (name[1] == '@') {
				script->set_reg_scope_str(st, &st->stack->scope, num, name, str);
			} else {
				script->set_reg_npc_str(st, &st->script->local, num, name, str);
			}
			return 1;
		case '\'':
			set_reg_instance_str(st, num, name, str);
			return 1;
		default:
			if (ref) {
				script->set_reg_pc_ref_str(st, ref, num, name, str);
			} else {
				pc_setglobalreg_str(sd, num, str);
			}
			return 1;
		}
	} else {// integer variable
		// FIXME: This isn't safe, in 32bits systems we're converting a 64bit pointer
		// to a 32bit int, this will lead to overflows! [Panikon]
		int val = (int)h64BPTRSIZE(value);

		if (script->str_data[script_getvarid(num)].type == C_PARAM) {
			if (pc->setparam(sd, script->str_data[script_getvarid(num)].val, val) == 0) {
				if (st != NULL) {
					ShowError("script:set_reg: failed to set param '%s' to %d.\n", name, val);
					script->reportsrc(st);
					// Instead of just stop the script execution we let the character close
					// the window if it was open.
					st->state = (sd->state.dialog) ? CLOSE : END;
					if(st->state == CLOSE) {
						clif->scriptclose(sd, st->oid);
					}
				}
				return 0;
			}
			return 1;
		}

		switch (prefix) {
		case '@':
			if (ref) {
				script->set_reg_ref_num(st, ref, num, name, val);
			} else {
				pc->setreg(sd, num, val);
			}
			return 1;
		case '$':
			mapreg->setreg(num, val);
			return 1;
		case '#':
			if (ref) {
				script->set_reg_pc_ref_num(st, ref, num, name, val);
			} else if (name[1] == '#') {
				pc_setaccountreg2(sd, num, val);
			} else {
				pc_setaccountreg(sd, num, val);
			}
			return 1;
		case '.':
			if (ref) {
				script->set_reg_ref_num(st, ref, num, name, val);
			} else if (name[1] == '@') {
				script->set_reg_scope_num(st, &st->stack->scope, num, name, val);
			} else {
				script->set_reg_npc_num(st, &st->script->local, num, name, val);
			}
			return 1;
		case '\'':
			set_reg_instance_num(st, num, name, val);
			return 1;
		default:
			if (ref) {
				script->set_reg_pc_ref_num(st, ref, num, name, val);
			} else {
				pc_setglobalreg(sd, num, val);
			}
			return 1;
		}
	}
}

static int set_var(struct map_session_data *sd, char *name, void *val)
{
	int key = script->add_variable(name);

	if (script->str_data[key].type != C_NAME) {
		ShowError("script:setd_sub: `%s` is already used by something that is not a variable.\n", name);
		return -1;
	}

	return script->set_reg(NULL, sd, reference_uid(key, 0), name, val, NULL);
}

static void setd_sub(struct script_state *st, struct map_session_data *sd, const char *varname, int elem, const void *value, struct reg_db *ref)
{
	int key = script->add_variable(varname);

	if (script->str_data[key].type != C_NAME) {
		ShowError("script:setd_sub: `%s` is already used by something that is not a variable.\n", varname);
		return;
	}

	script->set_reg(st, sd, reference_uid(key, elem), varname, value, ref);
}

/// Converts the data to a string
static const char *conv_str(struct script_state *st, struct script_data *data)
{
	script->get_val(st, data);
	if (data_isstring(data)) {
		// nothing to convert
		return data->u.str;
	}
	if (data_isint(data)) {
		// int -> string
		char *p;
		CREATE(p, char, ITEM_NAME_LENGTH);
		snprintf(p, ITEM_NAME_LENGTH, "%"PRId64"", data->u.num);
		p[ITEM_NAME_LENGTH-1] = '\0';
		data->type = C_STR;
		data->u.mutstr = p;
		return data->u.mutstr;
	}
	if (data_isreference(data)) {
		// reference -> string
		//##TODO when does this happen (check script->get_val) [FlavioJS]
		data->type = C_CONSTSTR;
		data->u.str = reference_getname(data);
		return data->u.str;
	}
	// unsupported data type
	ShowError("script:conv_str: cannot convert to string, defaulting to \"\"\n");
	script->reportdata(data);
	script->reportsrc(st);
	data->type = C_CONSTSTR;
	data->u.str = "";
	return data->u.str;
}

/// Converts the data to an int
static int conv_num(struct script_state *st, struct script_data *data)
{
	long num;

	script->get_val(st, data);
	if (data_isint(data)) {
		// nothing to convert
		return (int)data->u.num;
	}
	if (data_isstring(data)) {
		// string -> int
		// the result does not overflow or underflow, it is capped instead
		// ex: 999999999999 is capped to INT_MAX (2147483647)
		errno = 0;
		num = strtol(data->u.str, NULL, 10);// change radix to 0 to support octal numbers "o377" and hex numbers "0xFF"
		if( errno == ERANGE
#if LONG_MAX > INT_MAX
			|| num < INT_MIN || num > INT_MAX
#endif
			)
		{
			if( num <= INT_MIN )
			{
				num = INT_MIN;
				ShowError("script:conv_num: underflow detected, capping to %ld\n", num);
			}
			else//if( num >= INT_MAX )
			{
				num = INT_MAX;
				ShowError("script:conv_num: overflow detected, capping to %ld\n", num);
			}
			script->reportdata(data);
			script->reportsrc(st);
		}
		if (data->type == C_STR)
			aFree(data->u.mutstr);
		data->type = C_INT;
		data->u.num = (int)num;
		return (int)data->u.num;
	}
#if 0
	// unsupported data type
	// FIXME this function is being used to retrieve the position of labels and
	// probably other stuff [FlavioJS]
	ShowError("script:conv_num: cannot convert to number, defaulting to 0\n");
	script->reportdata(data);
	script->reportsrc(st);
	data->type = C_INT;
	data->u.num = 0;
#endif
	return (int)data->u.num;
}

//
// Stack operations
//

/// Increases the size of the stack
static void stack_expand(struct script_stack *stack)
{
	nullpo_retv(stack);
	stack->sp_max += 64;
	stack->stack_data = (struct script_data*)aRealloc(stack->stack_data,
			stack->sp_max * sizeof(stack->stack_data[0]) );
	memset(stack->stack_data + (stack->sp_max - 64), 0,
			64 * sizeof(stack->stack_data[0]) );
}

/// Pushes a value into the stack (with reference)
static struct script_data *push_val(struct script_stack *stack, enum c_op type, int64 val, struct reg_db *ref)
{
	nullpo_retr(NULL, stack);
	if( stack->sp >= stack->sp_max )
		script->stack_expand(stack);
	stack->stack_data[stack->sp].type  = type;
	stack->stack_data[stack->sp].u.num = val;
	stack->stack_data[stack->sp].ref   = ref;
	stack->sp++;
	return &stack->stack_data[stack->sp-1];
}

/// Pushes a string into the stack
static struct script_data *push_str(struct script_stack *stack, char *str)
{
	nullpo_retr(NULL, stack);
	if( stack->sp >= stack->sp_max )
		script->stack_expand(stack);
	stack->stack_data[stack->sp].type  = C_STR;
	stack->stack_data[stack->sp].u.mutstr = str;
	stack->stack_data[stack->sp].ref   = NULL;
	stack->sp++;
	return &stack->stack_data[stack->sp-1];
}

/// Pushes a constant string into the stack
static struct script_data *push_conststr(struct script_stack *stack, const char *str)
{
	nullpo_retr(NULL, stack);
	if( stack->sp >= stack->sp_max )
		script->stack_expand(stack);
	stack->stack_data[stack->sp].type  = C_CONSTSTR;
	stack->stack_data[stack->sp].u.str = str;
	stack->stack_data[stack->sp].ref   = NULL;
	stack->sp++;
	return &stack->stack_data[stack->sp-1];
}

/// Pushes a retinfo into the stack
static struct script_data *push_retinfo(struct script_stack *stack, struct script_retinfo *ri, struct reg_db *ref)
{
	nullpo_retr(NULL, stack);
	if( stack->sp >= stack->sp_max )
		script->stack_expand(stack);
	stack->stack_data[stack->sp].type = C_RETINFO;
	stack->stack_data[stack->sp].u.ri = ri;
	stack->stack_data[stack->sp].ref  = ref;
	stack->sp++;
	return &stack->stack_data[stack->sp-1];
}

/// Pushes a copy of the target position into the stack
static struct script_data *push_copy(struct script_stack *stack, int pos)
{
	nullpo_retr(NULL, stack);
	switch( stack->stack_data[pos].type ) {
		case C_CONSTSTR:
			return script->push_conststr(stack, stack->stack_data[pos].u.str);
			break;
		case C_STR:
			return script->push_str(stack, aStrdup(stack->stack_data[pos].u.mutstr));
			break;
		case C_RETINFO:
			ShowFatalError("script:push_copy: can't create copies of C_RETINFO. Exiting...\n");
			exit(1);
			break;
		default:
			return script->push_val(
				stack,stack->stack_data[pos].type,
				stack->stack_data[pos].u.num,
				stack->stack_data[pos].ref
			);
			break;
		}
}

/// Removes the values in indexes [start,end[ from the stack.
/// Adjusts all stack pointers.
static void pop_stack(struct script_state *st, int start, int end)
{
	struct script_stack* stack;
	struct script_data* data;
	int i;

	nullpo_retv(st);
	stack = st->stack;

	if( start < 0 )
		start = 0;
	if( end > stack->sp )
		end = stack->sp;
	if( start >= end )
		return;// nothing to pop

	// free stack elements
	for( i = start; i < end; i++ )
	{
		data = &stack->stack_data[i];
		if (data->type == C_STR)
			aFree(data->u.mutstr);
		if( data->type == C_RETINFO )
		{
			struct script_retinfo* ri = data->u.ri;
			if( ri->scope.vars ) {
				// Note: This is necessary evern if we're also doing it in run_func
				// (in the RETFUNC block) because not all functions return.  If a
				// function (or a sub) has an 'end' or a 'close', it'll reach this
				// block with its scope vars still to be freed.
				script->free_vars(ri->scope.vars);
				ri->scope.vars = NULL;
			}
			if( ri->scope.arrays ) {
				ri->scope.arrays->destroy(ri->scope.arrays,script->array_free_db);
				ri->scope.arrays = NULL;
			}
			if( data->ref )
				aFree(data->ref);
			aFree(ri);
		}
		data->type = C_NOP;
	}
	// move the rest of the elements
	if( stack->sp > end )
	{
		memmove(&stack->stack_data[start], &stack->stack_data[end], sizeof(stack->stack_data[0])*(stack->sp - end));
		for( i = start + stack->sp - end; i < stack->sp; ++i )
			stack->stack_data[i].type = C_NOP;
	}
	// adjust stack pointers
	if( st->start > end )
		st->start -= end - start;
	else if( st->start > start )
		st->start = start;
	if( st->end > end )
		st->end -= end - start;
	else if( st->end > start )
		st->end = start;
	if( stack->defsp > end )
		stack->defsp -= end - start;
	else if( stack->defsp > start )
		stack->defsp = start;
	stack->sp -= end - start;
}

///
///
///

/*==========================================
 * Release script dependent variable, dependent variable of function
 *------------------------------------------*/
static void script_free_vars(struct DBMap *var_storage)
{
	if( var_storage ) {
		// destroy the storage construct containing the variables
		db_destroy(var_storage);
	}
}

static void script_free_code(struct script_code *code)
{
	nullpo_retv(code);

	if (code->instances)
		script->stop_instances(code);
	script->free_vars(code->local.vars);
	if (code->local.arrays)
		code->local.arrays->destroy(code->local.arrays,script->array_free_db);
	VECTOR_CLEAR(code->script_buf);
	aFree(code);
}

/// Creates a new script state.
///
/// @param script Script code
/// @param pos Position in the code
/// @param rid Who is running the script (attached player)
/// @param oid Where the code is being run (npc 'object')
/// @return Script state
static struct script_state *script_alloc_state(struct script_code *rootscript, int pos, int rid, int oid)
{
	struct script_state* st;

	st = ers_alloc(script->st_ers, struct script_state);
	st->stack = ers_alloc(script->stack_ers, struct script_stack);
	st->pending_refs = NULL;
	st->pending_ref_count = 0;
	st->stack->sp = 0;
	st->stack->sp_max = 64;
	CREATE(st->stack->stack_data, struct script_data, st->stack->sp_max);
	st->stack->defsp = st->stack->sp;
	st->stack->scope.vars = i64db_alloc(DB_OPT_RELEASE_DATA);
	st->stack->scope.arrays = NULL;
	st->state = RUN;
	st->script = rootscript;
	st->pos = pos;
	st->rid = rid;
	st->oid = oid;
	st->sleep.timer = INVALID_TIMER;
	st->npc_item_flag = battle_config.item_enabled_npc;

	if( st->script->instances != USHRT_MAX )
		st->script->instances++;
	else {
		struct npc_data *nd = map->id2nd(oid);
		ShowError("over 65k instances of '%s' script are being run\n",nd ? nd->name : "unknown");
	}

	if( !st->script->local.vars )
		st->script->local.vars = i64db_alloc(DB_OPT_RELEASE_DATA);

	st->id = script->next_id++;
	script->active_scripts++;

	idb_put(script->st_db, st->id, st);

	return st;
}

/// Frees a script state.
///
/// @param st Script state
static void script_free_state(struct script_state *st)
{
	nullpo_retv(st);
	if( idb_exists(script->st_db,st->id) ) {
		struct map_session_data *sd = st->rid ? map->id2sd(st->rid) : NULL;

		if(st->bk_st) {// backup was not restored
			ShowDebug("script_free_state: Previous script state lost (rid=%d, oid=%d, state=%u, bk_npcid=%d).\n", st->bk_st->rid, st->bk_st->oid, st->bk_st->state, st->bk_npcid);
		}

		if(sd && sd->st == st) { //Current script is aborted.
			if(sd->state.using_fake_npc){
				clif->clearunit_single(sd->npc_id, CLR_OUTSIGHT, sd->fd);
				sd->state.using_fake_npc = 0;
			}
			sd->st = NULL;
			sd->npc_id = 0;
		}

		if( st->sleep.timer != INVALID_TIMER )
			timer->delete(st->sleep.timer, script->run_timer);
		if( st->stack ) {
			script->free_vars(st->stack->scope.vars);
			if( st->stack->scope.arrays )
				st->stack->scope.arrays->destroy(st->stack->scope.arrays,script->array_free_db);
			script->pop_stack(st, 0, st->stack->sp);
			aFree(st->stack->stack_data);
			ers_free(script->stack_ers, st->stack);
			st->stack = NULL;
		}
		if( st->script && st->script->instances != USHRT_MAX && --st->script->instances == 0 ) {
			if( st->script->local.vars && !db_size(st->script->local.vars) ) {
				script->free_vars(st->script->local.vars);
				st->script->local.vars = NULL;
			}
			if( st->script->local.arrays && !db_size(st->script->local.arrays) ) {
				st->script->local.arrays->destroy(st->script->local.arrays,script->array_free_db);
				st->script->local.arrays = NULL;
			}
		}
		st->pos = -1;
		if (st->pending_ref_count > 0) {
			while (st->pending_ref_count > 0)
				aFree(st->pending_refs[--st->pending_ref_count]);
			aFree(st->pending_refs);
			st->pending_refs = NULL;
		}
		idb_remove(script->st_db, st->id);
		ers_free(script->st_ers, st);
		if( --script->active_scripts == 0 ) {
			script->next_id = 0;
		}
	}
}

/**
 * Adds a pending reference entry to the current script.
 *
 * @see struct script_state::pending_refs
 *
 * @param st[in]  Script state.
 * @param ref[in] Reference to be added.
 */
static void script_add_pending_ref(struct script_state *st, struct reg_db *ref)
{
	nullpo_retv(st);
	RECREATE(st->pending_refs, struct reg_db*, ++st->pending_ref_count);
	st->pending_refs[st->pending_ref_count-1] = ref;
}

//
// Main execution unit
//
/*==========================================
 * Read command
 *------------------------------------------*/
static c_op get_com(const struct script_buf *scriptbuf, int *pos)
{
	int i = 0, j = 0;

	if (VECTOR_INDEX(*scriptbuf, *pos) >= 0x80) {
		return C_INT;
	}
	while (VECTOR_INDEX(*scriptbuf, *pos) >= 0x40) {
		i = VECTOR_INDEX(*scriptbuf, (*pos)++) << j;
		j+=6;
	}
	return (c_op)(i+(VECTOR_INDEX(*scriptbuf, (*pos)++)<<j));
}

/*==========================================
 *  Income figures
 *------------------------------------------*/
static int get_num(const struct script_buf *scriptbuf, int *pos)
{
	int i,j;
	i=0; j=0;
	while (VECTOR_INDEX(*scriptbuf, *pos) >= 0xc0) {
		i+= (VECTOR_INDEX(*scriptbuf, (*pos)++)&0x7f)<<j;
		j+=6;
	}
	return i+((VECTOR_INDEX(*scriptbuf, (*pos)++)&0x7f)<<j);
}

/// Ternary operators
/// test ? if_true : if_false
static void op_3(struct script_state *st, int op)
{
	struct script_data* data;
	int flag = 0;

	data = script_getdatatop(st, -3);
	script->get_val(st, data);

	if (data_isstring(data)) {
		flag = data->u.str[0]; // "" -> false
	} else if (data_isint(data)) {
		flag = data->u.num == 0 ? 0 : 1;// 0 -> false
	} else {
		ShowError("script:op_3: invalid data for the ternary operator test\n");
		script->reportdata(data);
		script->reportsrc(st);
		script_removetop(st, -3, 0);
		script_pushnil(st);
		return;
	}
	if( flag )
		script_pushcopytop(st, -2);
	else
		script_pushcopytop(st, -1);
	script_removetop(st, -4, -1);
}

/// Binary string operators
/// s1 EQ s2 -> i
/// s1 NE s2 -> i
/// s1 GT s2 -> i
/// s1 GE s2 -> i
/// s1 LT s2 -> i
/// s1 LE s2 -> i
/// s1 RE_EQ s2 -> i
/// s1 RE_NE s2 -> i
/// s1 ADD s2 -> s
static void op_2str(struct script_state *st, int op, const char *s1, const char *s2)
{
	int a = 0;

	switch(op) {
	case C_EQ: a = (strcmp(s1,s2) == 0); break;
	case C_NE: a = (strcmp(s1,s2) != 0); break;
	case C_GT: a = (strcmp(s1,s2) >  0); break;
	case C_GE: a = (strcmp(s1,s2) >= 0); break;
	case C_LT: a = (strcmp(s1,s2) <  0); break;
	case C_LE: a = (strcmp(s1,s2) <= 0); break;
	case C_RE_EQ:
	case C_RE_NE:
		{
			int inputlen = (int)strlen(s1);
			pcre *compiled_regex;
			pcre_extra *extra_regex;
			const char *pcre_error, *pcre_match;
			int pcre_erroroffset, offsetcount;
			int offsets[256*3]; // (max_capturing_groups+1)*3

			compiled_regex = libpcre->compile(s2, 0, &pcre_error, &pcre_erroroffset, NULL);

			if( compiled_regex == NULL ) {
				ShowError("script:op2_str: Invalid regex '%s'.\n", s2);
				script->reportsrc(st);
				script_pushnil(st);
				st->state = END;
				return;
			}

			extra_regex = libpcre->study(compiled_regex, 0, &pcre_error);

			if( pcre_error != NULL ) {
				libpcre->free(compiled_regex);
				ShowError("script:op2_str: Unable to optimize the regex '%s': %s\n", s2, pcre_error);
				script->reportsrc(st);
				script_pushnil(st);
				st->state = END;
				return;
			}

			offsetcount = libpcre->exec(compiled_regex, extra_regex, s1, inputlen, 0, 0, offsets, 256*3);

			if( offsetcount == 0 ) {
				offsetcount = 256;
			} else if( offsetcount == PCRE_ERROR_NOMATCH ) {
				offsetcount = 0;
			} else if( offsetcount < 0 ) {
				libpcre->free(compiled_regex);
				if( extra_regex != NULL )
					libpcre->free(extra_regex);
				ShowWarning("script:op2_str: Unable to process the regex '%s'.\n", s2);
				script->reportsrc(st);
				script_pushnil(st);
				st->state = END;
				return;
			}

			if (op == C_RE_EQ) {
				int i;
				for (i = 0; i < offsetcount; i++) {
					libpcre->get_substring(s1, offsets, offsetcount, i, &pcre_match);
					mapreg->setregstr(reference_uid(script->add_variable("$@regexmatch$"), i), pcre_match);
					libpcre->free_substring(pcre_match);
				}
				mapreg->setreg(script->add_variable("$@regexmatchcount"), i);
				a = offsetcount;
			} else { // C_RE_NE
				a = (offsetcount == 0);
			}
			libpcre->free(compiled_regex);
			if( extra_regex != NULL )
				libpcre->free(extra_regex);
		}
		break;
	case C_ADD:
		{
			char* buf = (char *)aMalloc((strlen(s1)+strlen(s2)+1)*sizeof(char));
			strcpy(buf, s1);
			strcat(buf, s2);
			script_pushstr(st, buf);
			return;
		}
	default:
		ShowError("script:op2_str: unexpected string operator %s\n", script->op2name(op));
		script->reportsrc(st);
		script_pushnil(st);
		st->state = END;
		return;
	}

	script_pushint(st,a);
}

/// Binary number operators
/// i OP i -> i
static void op_2num(struct script_state *st, int op, int i1, int i2)
{
	int ret;
	int64 ret64;

	switch( op ) {
	case C_AND:     ret = i1 & i2;    break;
	case C_OR:      ret = i1 | i2;    break;
	case C_XOR:     ret = i1 ^ i2;    break;
	case C_LAND:    ret = (i1 && i2); break;
	case C_LOR:     ret = (i1 || i2); break;
	case C_EQ:      ret = (i1 == i2); break;
	case C_NE:      ret = (i1 != i2); break;
	case C_GT:      ret = (i1 >  i2); break;
	case C_GE:      ret = (i1 >= i2); break;
	case C_LT:      ret = (i1 <  i2); break;
	case C_LE:      ret = (i1 <= i2); break;
	case C_R_SHIFT: ret = i1>>i2;     break;
	case C_L_SHIFT: ret = i1<<i2;     break;
	case C_DIV:
	case C_MOD:
		if( i2 == 0 )
		{
			ShowError("script:op_2num: division by zero detected op=%s i1=%d i2=%d\n", script->op2name(op), i1, i2);
			script->reportsrc(st);
			script_pushnil(st);
			st->state = END;
			return;
		}
		else if( op == C_DIV )
			ret = i1 / i2;
		else//if( op == C_MOD )
			ret = i1 % i2;
		break;
	default:
		switch (op) { // operators that can overflow/underflow
		case C_ADD: ret = i1 + i2; ret64 = (int64)i1 + i2; break;
		case C_SUB: ret = i1 - i2; ret64 = (int64)i1 - i2; break;
		case C_MUL: ret = i1 * i2; ret64 = (int64)i1 * i2; break;
		case C_POW: ret = (int)pow((double)i1, (double)i2); ret64 = (int64)pow((double)i1, (double)i2); break;
		default:
			ShowError("script:op_2num: unexpected number operator %s i1=%d i2=%d\n", script->op2name(op), i1, i2);
			script->reportsrc(st);
			script_pushnil(st);
			return;
		}
		if (ret64 < INT_MIN) {
			ShowWarning("script:op_2num: underflow detected op=%s i1=%d i2=%d\n", script->op2name(op), i1, i2);
			script->reportsrc(st);
			ret = INT_MIN;
		} else if (ret64 > INT_MAX) {
			ShowWarning("script:op_2num: overflow detected op=%s i1=%d i2=%d\n", script->op2name(op), i1, i2);
			script->reportsrc(st);
			ret = INT_MAX;
		}
	}
	script_pushint(st, ret);
}

/// Binary operators
static void op_2(struct script_state *st, int op)
{
	struct script_data* left, leftref;
	struct script_data* right;

	leftref.type = C_NOP;

	left = script_getdatatop(st, -2);
	right = script_getdatatop(st, -1);

	if (st->op2ref) {
		if (data_isreference(left)) {
			leftref = *left;
		}

		st->op2ref = 0;
	}

	script->get_val(st, left);
	script->get_val(st, right);

	// automatic conversions
	switch( op )
	{
	case C_ADD:
		if( data_isint(left) && data_isstring(right) )
		{// convert int-string to string-string
			script->conv_str(st, left);
		}
		else if( data_isstring(left) && data_isint(right) )
		{// convert string-int to string-string
			script->conv_str(st, right);
		}
		break;
	}

	if( data_isstring(left) && data_isstring(right) )
	{// ss => op_2str
		script->op_2str(st, op, left->u.str, right->u.str);
		script_removetop(st, leftref.type == C_NOP ? -3 : -2, -1);// pop the two values before the top one

		if (leftref.type != C_NOP) {
			if (left->type == C_STR) // don't free C_CONSTSTR
				aFree(left->u.mutstr);
			*left = leftref;
		}
	}
	else if( data_isint(left) && data_isint(right) )
	{// ii => op_2num
		int i1 = (int)left->u.num;
		int i2 = (int)right->u.num;

		script_removetop(st, leftref.type == C_NOP ? -2 : -1, 0);
		script->op_2num(st, op, i1, i2);

		if (leftref.type != C_NOP)
			*left = leftref;
	}
	else
	{// invalid argument
		ShowError("script:op_2: invalid data for operator %s\n", script->op2name(op));
		script->reportdata(left);
		script->reportdata(right);
		script->reportsrc(st);
		script_removetop(st, -2, 0);
		script_pushnil(st);
		st->state = END;
	}
}

/// Unary operators
/// NEG i -> i
/// NOT i -> i
/// LNOT i -> i
static void op_1(struct script_state *st, int op)
{
	struct script_data* data;
	int i1;

	data = script_getdatatop(st, -1);
	script->get_val(st, data);

	if( !data_isint(data) )
	{// not a number
		ShowError("script:op_1: argument is not a number (op=%s)\n", script->op2name(op));
		script->reportdata(data);
		script->reportsrc(st);
		script_pushnil(st);
		st->state = END;
		return;
	}

	i1 = (int)data->u.num;
	script_removetop(st, -1, 0);
	switch( op ) {
		case C_NEG: i1 = -i1; break;
		case C_NOT: i1 = ~i1; break;
		case C_LNOT: i1 = !i1; break;
		default:
			ShowError("script:op_1: unexpected operator %s i1=%d\n", script->op2name(op), i1);
			script->reportsrc(st);
			script_pushnil(st);
			st->state = END;
			return;
	}
	script_pushint(st, i1);
}

/// Checks the type of all arguments passed to a built-in function.
///
/// @param st Script state whose stack arguments should be inspected.
/// @param func Built-in function for which the arguments are intended.
static bool script_check_buildin_argtype(struct script_state *st, int func)
{
	int idx, invalid = 0;
	char* sf;
	if (script->str_data[func].val < 0 || script->str_data[func].val >= script->buildin_count) {
		ShowDebug("Function: %s\n", script->get_str(func));
		ShowError("Script data corruption detected!\n");
		script->reportsrc(st);
		return false;
	}
	sf = script->buildin[script->str_data[func].val];

	for (idx = 2; script_hasdata(st, idx); idx++) {
		struct script_data* data = script_getdata(st, idx);
		char type = sf[idx-2];
		const char* name = NULL;

		if (type == '?' || type == '*') {
			// optional argument or unknown number of optional parameters ( no types are after this )
			break;
		}
		if (type == 0) {
			// more arguments than necessary ( should not happen, as it is checked before )
			ShowWarning("Found more arguments than necessary. unexpected arg type %s\n",script->op2name(data->type));
			invalid++;
			break;
		}

		if (data_isreference(data)) {
			// get name for variables to determine the type they refer to
			name = reference_getname(data);
		}

		switch (type) {
			case 'v':
				if (!data_isstring(data) && !data_isint(data) && !data_isreference(data)) {
					// variant
					ShowWarning("Unexpected type for argument %d. Expected string, number or variable.\n", idx-1);
					script->reportdata(data);
					invalid++;
				}
				break;
			case 's':
				if (!data_isstring(data) && !(data_isreference(data) && is_string_variable(name))) {
					// string
					ShowWarning("Unexpected type for argument %d. Expected string.\n", idx-1);
					script->reportdata(data);
					invalid++;
				}
				break;
			case 'i':
				if (!data_isint(data) && !(data_isreference(data) && (reference_toparam(data) || reference_toconstant(data) || !is_string_variable(name)))) {
					// int ( params and constants are always int )
					ShowWarning("Unexpected type for argument %d. Expected number.\n", idx-1);
					script->reportdata(data);
					invalid++;
				}
				break;
			case 'r':
				if (!data_isreference(data) || reference_toconstant(data)) {
					// variables
					ShowWarning("Unexpected type for argument %d. Expected variable, got %s.\n", idx-1,script->op2name(data->type));
					script->reportdata(data);
					invalid++;
				}
				break;
			case 'l':
				if (!data_islabel(data) && !data_isfunclabel(data)) {
					// label
					ShowWarning("Unexpected type for argument %d. Expected label, got %s\n", idx-1,script->op2name(data->type));
					script->reportdata(data);
					invalid++;
				}
				break;
		}
	}

	if (invalid) {
		ShowDebug("Function: %s\n", script->get_str(func));
		script->reportsrc(st);
	}
	return true;
}

/// Executes a buildin command.
/// Stack: C_NAME(<command>) C_ARG <arg0> <arg1> ... <argN>
static int run_func(struct script_state *st)
{
	struct script_data* data;
	int i,start_sp,end_sp,func;

	nullpo_retr(1, st);
	end_sp = st->stack->sp;// position after the last argument
	for( i = end_sp-1; i > 0 ; --i )
		if( st->stack->stack_data[i].type == C_ARG )
			break;
	if( i == 0 )
	{
		ShowError("script:run_func: C_ARG not found. please report this!!!\n");
		st->state = END;
		script->reportsrc(st);
		return 1;
	}
	start_sp = i-1;// C_NAME of the command
	st->start = start_sp;
	st->end = end_sp;

	data = &st->stack->stack_data[st->start];
	if( data->type == C_NAME && script->str_data[data->u.num].type == C_FUNC )
		func = (int)data->u.num;
	else
	{
		ShowError("script:run_func: not a buildin command.\n");
		script->reportdata(data);
		script->reportsrc(st);
		st->state = END;
		return 1;
	}

	if( script->config.warn_func_mismatch_argtypes ) {
		if (script->check_buildin_argtype(st, func) == false)
		{
			st->state = END;
			return 1;
		}
	}

	if(script->str_data[func].func) {
		if (!(script->str_data[func].func(st))) //Report error
			script->reportsrc(st);
	} else {
		ShowError("script:run_func: '%s' (id=%d type=%s) has no C function. please report this!!!\n",
		          script->get_str(func), func, script->op2name(script->str_data[func].type));
		script->reportsrc(st);
		st->state = END;
	}

	// Stack's datum are used when re-running functions [Eoe]
	if( st->state == RERUNLINE )
		return 0;

	script->pop_stack(st, st->start, st->end);
	if( st->state == RETFUNC )
	{// return from a user-defined function
		struct script_retinfo* ri;
		int olddefsp = st->stack->defsp;
		int nargs;

		script->pop_stack(st, st->stack->defsp, st->start);// pop distractions from the stack
		if( st->stack->defsp < 1 || st->stack->stack_data[st->stack->defsp-1].type != C_RETINFO )
		{
			ShowWarning("script:run_func: return without callfunc or callsub!\n");
			script->reportsrc(st);
			st->state = END;
			return 1;
		}
		script->free_vars(st->stack->scope.vars);
		st->stack->scope.arrays->destroy(st->stack->scope.arrays,script->array_free_db);

		ri = st->stack->stack_data[st->stack->defsp-1].u.ri;
		nargs = ri->nargs;
		st->pos = ri->pos;
		st->script = ri->script;
		st->stack->scope.vars = ri->scope.vars;
		st->stack->scope.arrays = ri->scope.arrays;
		st->stack->defsp = ri->defsp;
		memset(ri, 0, sizeof(struct script_retinfo));

		script->pop_stack(st, olddefsp-nargs-1, olddefsp);// pop arguments and retinfo

		st->state = GOTO;
	}

	return 0;
}

/*==========================================
 * script execution
 *------------------------------------------*/
static void run_script(struct script_code *rootscript, int pos, int rid, int oid)
{
	struct script_state *st;

	if( rootscript == NULL || pos < 0 )
		return;

	// TODO In jAthena, this function can take over the pending script in the player. [FlavioJS]
	//      It is unclear how that can be triggered, so it needs the be traced/checked in more detail.
	// NOTE At the time of this change, this function wasn't capable of taking over the script state because st->scriptroot was never set.
	st = script->alloc_state(rootscript, pos, rid, oid);

	script->run_main(st);
}

static void script_stop_instances(struct script_code *code)
{
	struct DBIterator *iter;
	struct script_state* st;

	if( !script->active_scripts )
		return;//dont even bother.

	iter = db_iterator(script->st_db);

	for( st = dbi_first(iter); dbi_exists(iter); st = dbi_next(iter) ) {
		if( st->script == code ) {
			script->free_state(st);
		}
	}

	dbi_destroy(iter);
}

/*==========================================
 * Timer function for sleep
 *------------------------------------------*/
static int run_script_timer(int tid, int64 tick, int id, intptr_t data)
{
	struct script_state *st     = idb_get(script->st_db,(int)data);
	if( st ) {
		struct map_session_data *sd = map->id2sd(st->rid);

		if((sd && sd->status.char_id != id) || (st->rid && !sd)) { //Character mismatch. Cancel execution.
			st->rid = 0;
			st->state = END;
		}
		st->sleep.timer = INVALID_TIMER;
		if(st->state != RERUNLINE)
			st->sleep.tick = 0;
		script->run_main(st);
	}
	return 0;
}

/// Detaches script state from possibly attached character and restores it's previous script if any.
///
/// @param st Script state to detach.
/// @param dequeue_event Whether to schedule any queued events, when there was no previous script.
static void script_detach_state(struct script_state *st, bool dequeue_event)
{
	struct map_session_data* sd;

	nullpo_retv(st);
	if(st->rid && (sd = map->id2sd(st->rid))!=NULL) {
		sd->st = st->bk_st;
		sd->npc_id = st->bk_npcid;
		if(st->bk_st) {
			//Remove tag for removal.
			st->bk_st = NULL;
			st->bk_npcid = 0;
		} else if(dequeue_event) {
			// For the Secure NPC Timeout option (check config/Secure.h) [RR]
#ifdef SECURE_NPCTIMEOUT
			// We're done with this NPC session, so we cancel the timer (if existent) and move on
			if( sd->npc_idle_timer != INVALID_TIMER ) {
				timer->delete(sd->npc_idle_timer,npc->secure_timeout_timer);
				sd->npc_idle_timer = INVALID_TIMER;
			}
#endif
			npc->event_dequeue(sd);
		}
	} else if(st->bk_st) { // rid was set to 0, before detaching the script state
		ShowError("script_detach_state: Found previous script state without attached player (rid=%d, oid=%d, state=%u, bk_npcid=%d)\n", st->bk_st->rid, st->bk_st->oid, st->bk_st->state, st->bk_npcid);
		script->reportsrc(st->bk_st);

		script->free_state(st->bk_st);
		st->bk_st = NULL;
	}
}

/// Attaches script state to possibly attached character and backups it's previous script, if any.
///
/// @param st Script state to attach.
static void script_attach_state(struct script_state *st)
{
	struct map_session_data* sd;

	nullpo_retv(st);
	if(st->rid && (sd = map->id2sd(st->rid))!=NULL)
	{
		if(st!=sd->st)
		{
			if(st->bk_st)
			{// there is already a backup
				ShowDebug("script_free_state: Previous script state lost (rid=%d, oid=%d, state=%u, bk_npcid=%d).\n", st->bk_st->rid, st->bk_st->oid, st->bk_st->state, st->bk_npcid);
			}
			st->bk_st = sd->st;
			st->bk_npcid = sd->npc_id;
		}
		sd->st = st;
		sd->npc_id = st->oid;
		sd->npc_item_flag = st->npc_item_flag; // load default.
/**
 * For the Secure NPC Timeout option (check config/Secure.h) [RR]
 **/
#ifdef SECURE_NPCTIMEOUT
		if( sd->npc_idle_timer == INVALID_TIMER )
			sd->npc_idle_timer = timer->add(timer->gettick() + (SECURE_NPCTIMEOUT_INTERVAL*1000),npc->secure_timeout_timer,sd->bl.id,0);
		sd->npc_idle_tick = timer->gettick();
#endif
	}
}

/*==========================================
 * The main part of the script execution
 *------------------------------------------*/
static void run_script_main(struct script_state *st)
{
	int cmdcount = script->config.check_cmdcount;
	int gotocount = script->config.check_gotocount;
	struct map_session_data *sd;
	struct script_stack *stack = st->stack;
	struct npc_data *nd;

	nullpo_retv(st);
	script->attach_state(st);
	if (st->state != END && Assert_chk(st->state == RUN || st->state == STOP || st->state == RERUNLINE)) {
		st->state = END;
	}

	nd = map->id2nd(st->oid);
	if( nd && nd->bl.m >= 0 )
		st->instance_id = map->list[nd->bl.m].instance_id;
	else
		st->instance_id = -1;

	if(st->state == RERUNLINE) {
		script->run_func(st);
		if(st->state == GOTO)
			st->state = RUN;
	} else if(st->state != END)
		st->state = RUN;

	while( st->state == RUN ) {
		enum c_op c = script->get_com(&st->script->script_buf, &st->pos);
		switch(c) {
			case C_EOL:
				if( stack->defsp > stack->sp )
					ShowError("script:run_script_main: unexpected stack position (defsp=%d sp=%d). please report this!!!\n", stack->defsp, stack->sp);
				else
					script->pop_stack(st, stack->defsp, stack->sp);// pop unused stack data. (unused return value)
				break;
			case C_INT:
				script->push_val(stack,C_INT,script->get_num(&st->script->script_buf, &st->pos), NULL);
				break;
			case C_POS:
			case C_NAME:
				script->push_val(stack,c,GETVALUE(&st->script->script_buf, st->pos), NULL);
				st->pos+=3;
				break;
			case C_ARG:
				script->push_val(stack,c,0,NULL);
				break;
			case C_STR:
				script->push_conststr(stack, (const char *)&VECTOR_INDEX(st->script->script_buf, st->pos));
				while (VECTOR_INDEX(st->script->script_buf, st->pos++) != 0)
					(void)0; // Skip string
				break;
			case C_LSTR:
			{
				struct map_session_data *lsd = NULL;
				uint8 translations = 0;
				int string_id = *((int *)(&VECTOR_INDEX(st->script->script_buf, st->pos)));
				st->pos += sizeof(string_id);
				translations = *((uint8 *)(&VECTOR_INDEX(st->script->script_buf, st->pos)));
				st->pos += sizeof(translations);

				if( (!st->rid || !(lsd = map->id2sd(st->rid)) || !lsd->lang_id) && !map->default_lang_id )
					script->push_conststr(stack, script->string_list+string_id);
				else {
					uint8 k, wlang_id = lsd ? lsd->lang_id : map->default_lang_id;
					int offset = st->pos;

					for(k = 0; k < translations; k++) {
						uint8 lang_id = *(uint8 *)(&VECTOR_INDEX(st->script->script_buf, offset));
						offset += sizeof(uint8);
						if( lang_id == wlang_id )
							break;
						offset += sizeof(char*);
					}
					if (k == translations)
						script->push_conststr(stack, script->string_list+string_id);
					else
						script->push_conststr(stack, *(const char**)(&VECTOR_INDEX(st->script->script_buf, offset)));
				}
				st->pos += ( ( sizeof(char*) + sizeof(uint8) ) * translations );
			}
				break;
			case C_FUNC:
				script->run_func(st);
				if(st->state==GOTO) {
					st->state = RUN;
					if( !st->freeloop && gotocount>0 && (--gotocount)<=0 ) {
						ShowError("run_script: infinity loop !\n");
						script->reportsrc(st);
						st->state=END;
					}
				}
				break;

			case C_REF:
				st->op2ref = 1;
				break;

			case C_NEG:
			case C_NOT:
			case C_LNOT:
				script->op_1(st ,c);
				break;

			case C_ADD:
			case C_SUB:
			case C_MUL:
			case C_POW:
			case C_DIV:
			case C_MOD:
			case C_EQ:
			case C_NE:
			case C_GT:
			case C_GE:
			case C_LT:
			case C_LE:
			case C_AND:
			case C_OR:
			case C_XOR:
			case C_LAND:
			case C_LOR:
			case C_R_SHIFT:
			case C_L_SHIFT:
			case C_RE_EQ:
			case C_RE_NE:
				script->op_2(st, c);
				break;

			case C_OP3:
				script->op_3(st, c);
				break;

			case C_NOP:
				st->state=END;
				break;

			default:
				ShowError("unknown command : %u @ %d\n", c, st->pos);
				st->state=END;
				break;
		}
		if( !st->freeloop && cmdcount>0 && (--cmdcount)<=0 ) {
			ShowError("run_script: too many opeartions being processed non-stop !\n");
			script->reportsrc(st);
			st->state=END;
		}
	}

	if(st->sleep.tick > 0) {
		//Restore previous script
		script->detach_state(st, false);
		//Delay execution
		sd = map->id2sd(st->rid); // Get sd since script might have attached someone while running. [Inkfish]
		st->sleep.charid = sd?sd->status.char_id:0;
		st->sleep.timer  = timer->add(timer->gettick()+st->sleep.tick,
			script->run_timer, st->sleep.charid, (intptr_t)st->id);
	} else if(st->state != END && st->rid) {
		//Resume later (st is already attached to player).
		if(st->bk_st) {
			ShowWarning("Unable to restore stack! Double continuation!\n");
			//Report BOTH scripts to see if that can help somehow.
			ShowDebug("Previous script (lost):\n");
			script->reportsrc(st->bk_st);
			ShowDebug("Current script:\n");
			script->reportsrc(st);

			script->free_state(st->bk_st);
			st->bk_st = NULL;
		}
	} else {
		//Dispose of script.
		if ((sd = map->id2sd(st->rid))!=NULL) { //Restore previous stack and save char.
			if(sd->state.using_fake_npc) {
				clif->clearunit_single(sd->npc_id, CLR_OUTSIGHT, sd->fd);
				sd->state.using_fake_npc = 0;
			}
			//Restore previous script if any.
			script->detach_state(st, true);
			if (sd->vars_dirty)
				intif->saveregistry(sd);
		}
		script->free_state(st);
		st = NULL;
	}
}

/**
 * Reads 'script_configuration' and initializes required variables.
 *
 * @param filename Path to configuration file.
 * @param imported Whether the current config is imported from another file.
 *
 * @retval false in case of error.
 */
static bool script_config_read(const char *filename, bool imported)
{
	struct config_t config;
	struct config_setting_t * setting = NULL;
	const char *import = NULL;
	bool retval = true;

	nullpo_retr(false, filename);

	if (!libconfig->load_file(&config, filename))
		return false;

	if ((setting = libconfig->lookup(&config, "script_configuration")) == NULL) {
		libconfig->destroy(&config);
		if (imported)
			return true;
		ShowError("script_config_read: script_configuration was not found in %s!\n", filename);
		return false;
	}

	libconfig->setting_lookup_bool_real(setting, "warn_func_mismatch_paramnum", &script->config.warn_func_mismatch_paramnum);
	libconfig->setting_lookup_bool_real(setting, "warn_func_mismatch_argtypes", &script->config.warn_func_mismatch_argtypes);
	libconfig->setting_lookup_int(setting, "check_cmdcount", &script->config.check_cmdcount);
	libconfig->setting_lookup_int(setting, "check_gotocount", &script->config.check_gotocount);
	libconfig->setting_lookup_int(setting, "input_min_value", &script->config.input_min_value);
	libconfig->setting_lookup_int(setting, "input_max_value", &script->config.input_max_value);

	if (!HPM->parse_conf(&config, filename, HPCT_SCRIPT, imported))
		retval = false;

	// import should overwrite any previous configuration, so it should be called last
	if (libconfig->lookup_string(&config, "import", &import) == CONFIG_TRUE) {
		if (strcmp(import, filename) == 0 || strcmp(import, map->SCRIPT_CONF_NAME) == 0) {
			ShowWarning("script_config_read: Loop detected! Skipping 'import'...\n");
		} else {
			if (!script->config_read(import, true))
				retval = false;
		}
	}

	libconfig->destroy(&config);
	return retval;
}

/**
 * @see DBApply
 */
static int db_script_free_code_sub(union DBKey key, struct DBData *data, va_list ap)
{
	struct script_code *code = DB->data2ptr(data);
	if (code)
		script->free_code(code);
	return 0;
}

static void script_run_autobonus(const char *autobonus, int id, int pos)
{
	struct script_code *scriptroot = (struct script_code *)strdb_get(script->autobonus_db, autobonus);

	if( scriptroot ) {
		status->current_equip_item_index = pos;
		script->run(scriptroot,0,id,0);
	}
}

static void script_add_autobonus(const char *autobonus)
{
	if( strdb_get(script->autobonus_db, autobonus) == NULL ) {
		struct script_code *scriptroot = script->parse(autobonus, "autobonus", 0, 0, NULL);

		if( scriptroot )
			strdb_put(script->autobonus_db, autobonus, scriptroot);
	}
}

/// resets a temporary character array variable to given value
static void script_cleararray_pc(struct map_session_data *sd, const char *varname, void *value)
{
	struct script_array *sa = NULL;
	struct reg_db *src = NULL;
	unsigned int i, *list = NULL, size = 0;
	int key;

	key = script->add_variable(varname);

	if (script->str_data[key].type != C_NAME) {
		ShowError("script:cleararray_pc: `%s` is already used by something that is not a variable.\n", varname);
		return;
	}

	if( !(src = script->array_src(NULL,sd,varname,NULL) ) )
		return;

	if( value )
		script->array_ensure_zero(NULL,sd,reference_uid(key,0),NULL);

	if( !(sa = idb_get(src->arrays, key)) ) /* non-existent array, nothing to empty */
		return;

	size = sa->size;
	list = script->array_cpy_list(sa);

	for(i = 0; i < size; i++) {
		script->set_reg(NULL,sd,reference_uid(key, list[i]),varname,value,NULL);
	}
}

/// sets a temporary character array variable element idx to given value
/// @param refcache Pointer to an int variable, which keeps a copy of the reference to varname and must be initialized to 0. Can be NULL if only one element is set.
static void script_setarray_pc(struct map_session_data *sd, const char *varname, uint32 idx, void *value, int *refcache)
{
	int key;

	if (idx > SCRIPT_MAX_ARRAYSIZE) {
		ShowError("script_setarray_pc: Variable '%s' has invalid index '%u' (char_id=%d).\n", varname, idx, sd->status.char_id);
		return;
	}

	key = ( refcache && refcache[0] ) ? refcache[0] : script->add_variable(varname);

	if (script->str_data[key].type != C_NAME) {
		ShowError("script:setarray_pc: `%s` is already used by something that is not a variable.\n", varname);
		return;
	}

	script->set_reg(NULL,sd,reference_uid(key, idx),varname,value,NULL);

	if( refcache )
	{// save to avoid repeated script->add_str calls
		refcache[0] = key;
	}
}
/**
 * Clears persistent variables from memory
 **/
static int script_reg_destroy(union DBKey key, struct DBData *data, va_list ap)
{
	struct script_reg_state *src;

	nullpo_ret(data);
	if( data->type != DB_DATA_PTR )/* got no need for those! */
		return 0;

	src = DB->data2ptr(data);

	if( src->type ) {
		struct script_reg_str *p = (struct script_reg_str *)src;

		if( p->value )
			aFree(p->value);

		ers_free(pc->str_reg_ers,p);
	} else {
		ers_free(pc->num_reg_ers,(struct script_reg_num*)src);
	}
	return 0;
}
/**
 * Clears a single persistent variable
 **/
static void script_reg_destroy_single(struct map_session_data *sd, int64 reg, struct script_reg_state *data)
{
	nullpo_retv(sd);
	nullpo_retv(data);
	i64db_remove(sd->regs.vars, reg);

	if( data->type ) {
		struct script_reg_str *p = (struct script_reg_str*)data;

		if( p->value )
			aFree(p->value);

		ers_free(pc->str_reg_ers,p);
	} else {
		ers_free(pc->num_reg_ers,(struct script_reg_num*)data);
	}
}
static unsigned int *script_array_cpy_list(struct script_array *sa)
{
	nullpo_retr(NULL, sa);
	if( sa->size > script->generic_ui_array_size )
		script->generic_ui_array_expand(sa->size);
	memcpy(script->generic_ui_array, sa->members, sizeof(unsigned int)*sa->size);
	return script->generic_ui_array;
}
static void script_generic_ui_array_expand(unsigned int plus)
{
	script->generic_ui_array_size += plus + 100;
	RECREATE(script->generic_ui_array, unsigned int, script->generic_ui_array_size);
}
/*==========================================
 * Destructor
 *------------------------------------------*/
static void do_final_script(void)
{
	int i;
	struct DBIterator *iter;
	struct script_state *st;

#ifdef SCRIPT_DEBUG_HASH
	if (battle_config.etc_log)
	{
		FILE *fp = fopen("hash_dump.txt","wt");
		if(fp) {
			int count[SCRIPT_HASH_SIZE];
			int count2[SCRIPT_HASH_SIZE]; // number of buckets with a certain number of items
			int n=0;
			int min=INT_MAX,max=0,zero=0;
			double mean=0.0f;
			double median=0.0f;

			ShowNotice("Dumping script str hash information to hash_dump.txt\n");
			memset(count, 0, sizeof(count));
			fprintf(fp,"num : hash : data_name\n");
			fprintf(fp,"---------------------------------------------------------------\n");
			for(i=LABEL_START; i<script->str_num; i++) {
				unsigned int h = script->calc_hash(script->get_str(i));
				fprintf(fp,"%04d : %4u : %s\n",i,h, script->get_str(i));
				++count[h];
			}
			fprintf(fp,"--------------------\n\n");
			memset(count2, 0, sizeof(count2));
			for(i=0; i<SCRIPT_HASH_SIZE; i++) {
				fprintf(fp,"  hash %3d = %d\n",i,count[i]);
				if(min > count[i])
					min = count[i]; // minimun count of collision
				if(max < count[i])
					max = count[i]; // maximun count of collision
				if(count[i] == 0)
					zero++;
				++count2[count[i]];
			}
			fprintf(fp,"\n--------------------\n  items : buckets\n--------------------\n");
			for( i=min; i <= max; ++i ) {
				fprintf(fp,"  %5d : %7d\n",i,count2[i]);
				mean += 1.0f*i*count2[i]/SCRIPT_HASH_SIZE; // Note: this will always result in <nr labels>/<nr buckets>
			}
			for( i=min; i <= max; ++i ) {
				n += count2[i];
				if( n*2 >= SCRIPT_HASH_SIZE )
				{
					if( SCRIPT_HASH_SIZE%2 == 0 && SCRIPT_HASH_SIZE/2 == n )
						median = (i+i+1)/2.0f;
					else
						median = i;
					break;
				}
			}
			fprintf(fp,"--------------------\n    min = %d, max = %d, zero = %d\n    mean = %lf, median = %lf\n",min,max,zero,mean,median);
			fclose(fp);
		}
	}
#endif

	iter = db_iterator(script->st_db);

	for( st = dbi_first(iter); dbi_exists(iter); st = dbi_next(iter) ) {
		script->free_state(st);
	}

	dbi_destroy(iter);

	mapreg->final();

	script->userfunc_db->destroy(script->userfunc_db, script->db_free_code_sub);
	script->autobonus_db->destroy(script->autobonus_db, script->db_free_code_sub);

	if (script->str_data)
		aFree(script->str_data);
	if (script->str_buf)
		aFree(script->str_buf);

	for( i = 0; i < atcommand->binding_count; i++ ) {
		aFree(atcommand->binding[i]->at_groups);
		aFree(atcommand->binding[i]->char_groups);
		aFree(atcommand->binding[i]);
	}

	if( atcommand->binding_count != 0 )
		aFree(atcommand->binding);

	for( i = 0; i < script->buildin_count; i++) {
		if( script->buildin[i] ) {
			aFree(script->buildin[i]);
			script->buildin[i] = NULL;
		}
	}

	aFree(script->buildin);

	for (i = 0; i < VECTOR_LENGTH(script->hq); i++) {
		VECTOR_CLEAR(VECTOR_INDEX(script->hq, i).entries);
	}
	VECTOR_CLEAR(script->hq);

	for (i = 0; i < VECTOR_LENGTH(script->hqi); i++) {
		VECTOR_CLEAR(VECTOR_INDEX(script->hqi, i).entries);
	}
	VECTOR_CLEAR(script->hqi);

	if( script->word_buf != NULL )
		aFree(script->word_buf);

#ifdef ENABLE_CASE_CHECK
	script->global_casecheck.clear();
	script->local_casecheck.clear();
#endif // ENABLE_CASE_CHECK

	ers_destroy(script->st_ers);
	ers_destroy(script->stack_ers);

	db_destroy(script->st_db);

	if( script->labels != NULL )
		aFree(script->labels);

	ers_destroy(script->array_ers);

	if( script->generic_ui_array )
		aFree(script->generic_ui_array);

	script->clear_translations(false);
	script->parser_clean_leftovers();
}

/**
 *
 **/
static uint8 script_add_language(const char *name)
{
	uint8 lang_id = script->max_lang_id;
	nullpo_ret(name);

	RECREATE(script->languages, char *, ++script->max_lang_id);
	script->languages[lang_id] = aStrdup(name);

	return lang_id;
}
/**
 * Goes thru db/translations.conf file
 **/
static void script_load_translations(void)
{
	struct config_t translations_conf;
	char config_filename[256];
	libconfig->format_db_path("translations.conf", config_filename, sizeof(config_filename));
	struct config_setting_t *translations = NULL;
	int i, size;
	int total = 0;
	uint8 lang_id = 0, k;

	if (map->minimal) // No translations in minimal mode
		return;

	script->translation_db = strdb_alloc(DB_OPT_DUP_KEY, NAME_LENGTH*2+1);

	if( script->languages ) {
		for(i = 0; i < script->max_lang_id; i++)
			aFree(script->languages[i]);
		aFree(script->languages);
	}
	script->languages = NULL;
	script->max_lang_id = 0;

	script->add_language("English");/* 0 is default, which is whatever is in the npc files hardcoded (in our case, English) */

	if (!libconfig->load_file(&translations_conf, config_filename))
		return;

	if ((translations = libconfig->lookup(&translations_conf, "translations")) == NULL) {
		ShowError("load_translations: invalid format on '%s'\n",config_filename);
		return;
	}

	if( script->string_list )
		aFree(script->string_list);

	script->string_list = NULL;
	script->string_list_pos = 0;
	script->string_list_size = 0;

	size = libconfig->setting_length(translations);

	for(i = 0; i < size; i++) {
		const char *translation_dir = libconfig->setting_get_string_elem(translations, i);
		total += script->load_translation(translation_dir, ++lang_id);
	}
	libconfig->destroy(&translations_conf);

	if (total != 0) {
		struct DBIterator *main_iter;
		struct DBMap *string_db;
		struct string_translation *st = NULL;

		VECTOR_ENSURE(script->translation_buf, total, 1);

		main_iter = db_iterator(script->translation_db);
		for (string_db = dbi_first(main_iter); dbi_exists(main_iter); string_db = dbi_next(main_iter)) {
			struct DBIterator *sub_iter = db_iterator(string_db);
			for (st = dbi_first(sub_iter); dbi_exists(sub_iter); st = dbi_next(sub_iter)) {
				VECTOR_PUSH(script->translation_buf, st->buf);
			}
			dbi_destroy(sub_iter);
		}
		dbi_destroy(main_iter);
	}

	for(k = 0; k < script->max_lang_id; k++) {
		if( !strcmpi(script->languages[k],map->default_lang_str) ) {
			break;
		}
	}

	if( k == script->max_lang_id ) {
		ShowError("load_translations: map server default_language setting '%s' is not a loaded language\n",map->default_lang_str);
		map->default_lang_id = 0;
	} else {
		map->default_lang_id = k;
	}
}

/**
 * Generates a language name from a translation directory name.
 *
 * @param directory The directory name.
 * @return The corresponding translation name.
 */
static const char *script_get_translation_dir_name(const char *directory)
{
	const char *basename = NULL, *last_dot = NULL;

	nullpo_retr("Unknown", directory);

	basename = strrchr(directory, '/');
#ifdef WIN32
	{
		const char *basename_windows = strrchr(directory, '\\');
		if (basename_windows > basename)
			basename = basename_windows;
	}
#endif // WIN32
	if (basename == NULL)
		basename = directory;
	else
		basename++; // Skip slash
	Assert_retr("Unknown", *basename != '\0');

	last_dot = strrchr(basename, '.');
	if (last_dot != NULL) {
		static char dir_name[200];
		if (last_dot == basename)
			return basename + 1;

		safestrncpy(dir_name, basename, last_dot - basename + 1);
		return dir_name;
	}

	return basename;
}

/**
 * Parses and adds a translated string to the translations database.
 *
 * @param file    Translations file being parsed (for error messages).
 * @param lang_id Language ID being parsed.
 * @param msgctxt Message context (i.e. NPC name)
 * @param msgid   Message ID (source string)
 * @param msgstr  Translated message
 * @return success state
 * @retval true if a new string was added.
 */
static bool script_load_translation_addstring(const char *file, uint8 lang_id, const char *msgctxt, const struct script_string_buf *msgid, const struct script_string_buf *msgstr)
{
	nullpo_retr(false, file);
	nullpo_retr(false, msgctxt);
	nullpo_retr(false, msgid);
	nullpo_retr(false, msgstr);

	if (VECTOR_LENGTH(*msgid) <= 1) {
		// Empty ID (i.e. header) to be ignored
		return false;
	}

	if (VECTOR_LENGTH(*msgstr) <= 1) {
		// Empty (untranslated) string to be ignored
		return false;
	}

	if (msgctxt[0] == '\0') {
		// Missing context
		ShowWarning("script_load_translation: Missing context for msgid '%s' in '%s'. Skipping.\n",
				VECTOR_DATA(*msgid), file);
		return false;
	}

	if (strcasecmp(msgctxt, "messages.conf") == 0) {
		int i;
		for (i = 0; i < MAX_MSG; i++) {
			if (atcommand->msg_table[0][i] != NULL && strcmpi(atcommand->msg_table[0][i], VECTOR_DATA(*msgid)) == 0) {
				if (atcommand->msg_table[lang_id][i] != NULL)
					aFree(atcommand->msg_table[lang_id][i]);
				atcommand->msg_table[lang_id][i] = aStrdup(VECTOR_DATA(*msgstr));
				break;
			}
		}
	} else {
		int msgstr_len = VECTOR_LENGTH(*msgstr);
		int inner_len = 1 + msgstr_len + 1; //uint8 lang_id + msgstr_len + '\0'
		struct string_translation *st = NULL;
		struct DBMap *string_db;

		if ((string_db = strdb_get(script->translation_db, msgctxt)) == NULL) {
			string_db = strdb_alloc(DB_OPT_DUP_KEY, 0);
			strdb_put(script->translation_db, msgctxt, string_db);
		}

		if ((st = strdb_get(string_db, VECTOR_DATA(*msgid))) == NULL) {
			CREATE(st, struct string_translation, 1);
			st->string_id = script->string_dup(VECTOR_DATA(*msgid));
			strdb_put(string_db, VECTOR_DATA(*msgid), st);
		}
		RECREATE(st->buf, uint8, st->len + inner_len);

		WBUFB(st->buf, st->len) = lang_id;
		safestrncpy(WBUFP(st->buf, st->len + 1), VECTOR_DATA(*msgstr), msgstr_len + 1);

		st->translations++;
		st->len += inner_len;
	}
	return true;
}

/**
 * Parses an individual translation file.
 *
 * @param directory The directory structure to read.
 * @param lang_id The language identifier.
 * @return The amount of strings loaded.
 */
static int script_load_translation_file(const char *file, uint8 lang_id)
{
	char line[1024];
	char msgctxt[NAME_LENGTH*2+1] = "";
	struct script_string_buf msgid, msgstr;
	struct script_string_buf *msg_ptr;
	int translations = 0;
	int lineno = 0;
	FILE *fp;

	nullpo_ret(file);

	if ((fp = fopen(file,"rb")) == NULL) {
		ShowError("load_translation: failed to open '%s' for reading\n",file);
		return 0;
	}

	VECTOR_INIT(msgid);
	VECTOR_INIT(msgstr);

	while (fgets(line, sizeof(line), fp) != NULL) {
		int len = (int)strlen(line);
		int i;
		lineno++;

		if (len <= 1) {
			if (VECTOR_LENGTH(msgid) > 0 && VECTOR_LENGTH(msgstr) > 0) {
				// Add string
				if (script->load_translation_addstring(file, lang_id, msgctxt, &msgid, &msgstr))
					translations++;

				msgctxt[0] = '\0';
				VECTOR_TRUNCATE(msgid);
				VECTOR_TRUNCATE(msgstr);
			}
			continue;
		}

		if (line[0] == '#')
			continue;

		if (VECTOR_LENGTH(msgid) > 0) {
			if (VECTOR_LENGTH(msgstr) > 0) {
				msg_ptr = &msgstr;
			} else {
				msg_ptr = &msgid;
			}
			if (line[0] == '"') {
				// Continuation line
				(void)VECTOR_POP(*msg_ptr); // Pop final '\0'
				for (i = 1; i < len - 2; i++) {
					VECTOR_ENSURE(*msg_ptr, 1, 512);
					if (line[i] == '\\' && line[i+1] == '"') {
						VECTOR_PUSH(*msg_ptr, '"');
						i++;
					} else if (line[i] == '\\' && line[i+1] == 'r') {
						VECTOR_PUSH(*msg_ptr, '\r');
						i++;
					} else {
						VECTOR_PUSH(*msg_ptr, line[i]);
					}
				}
				VECTOR_ENSURE(*msg_ptr, 1, 512);
				VECTOR_PUSH(*msg_ptr, '\0');
				continue;
			}

		}

		if (strncasecmp(line,"msgctxt \"", 9) == 0) {
			int cursor = 0;
			msgctxt[0] = '\0';
			for (i = 9; i < len - 2; i++) {
				if (line[i] == '\\' && line[i+1] == '"') {
					msgctxt[cursor] = '"';
					i++;
				} else if (line[i] == '\\' && line[i+1] == 'r') {
					msgctxt[cursor] = '\r';
					i++;
				} else {
					msgctxt[cursor] = line[i];
				}
				if (++cursor >= (int)sizeof(msgctxt) - 1)
					break;
			}
			msgctxt[cursor] = '\0';

			// New context, reset everything
			VECTOR_TRUNCATE(msgid);
			VECTOR_TRUNCATE(msgstr);
			continue;
		}

		if (strncasecmp(line, "msgid \"", 7) == 0) {
			VECTOR_TRUNCATE(msgid);
			for (i = 7; i < len - 2; i++) {
				VECTOR_ENSURE(msgid, 1, 512);
				if (line[i] == '\\' && line[i+1] == '"') {
					VECTOR_PUSH(msgid, '"');
					i++;
				} else if (line[i] == '\\' && line[i+1] == 'r') {
					VECTOR_PUSH(msgid, '\r');
					i++;
				} else {
					VECTOR_PUSH(msgid, line[i]);
				}
			}
			VECTOR_ENSURE(msgid, 1, 512);
			VECTOR_PUSH(msgid, '\0');

			// New id, reset string if any
			VECTOR_TRUNCATE(msgstr);
			continue;
		}

		if (VECTOR_LENGTH(msgid) > 0 && strncasecmp(line, "msgstr \"", 8) == 0) {
			VECTOR_TRUNCATE(msgstr);
			for (i = 8; i < len - 2; i++) {
				VECTOR_ENSURE(msgstr, 1, 512);
				if (line[i] == '\\' && line[i+1] == '"') {
					VECTOR_PUSH(msgstr, '"');
					i++;
				} else if (line[i] == '\\' && line[i+1] == 'r') {
					VECTOR_PUSH(msgstr, '\r');
					i++;
				} else {
					VECTOR_PUSH(msgstr, line[i]);
				}
			}
			VECTOR_ENSURE(msgstr, 1, 512);
			VECTOR_PUSH(msgstr, '\0');

			continue;
		}

		ShowWarning("script_load_translation: Unexpected input at '%s' in file '%s' line %d. Skipping.\n",
				line, file, lineno);
	}

	// Add last string
	if (VECTOR_LENGTH(msgid) > 0 && VECTOR_LENGTH(msgstr) > 0) {
		if (script->load_translation_addstring(file, lang_id, msgctxt, &msgid, &msgstr))
			translations++;
	}

	fclose(fp);

	VECTOR_CLEAR(msgid);
	VECTOR_CLEAR(msgstr);

	return translations;
}

struct load_translation_data {
	uint8 lang_id;
	int translation_count;
};

static void script_load_translation_sub(const char *filename, void *context)
{
	nullpo_retv(context);

	struct load_translation_data *data = context;

	data->translation_count += script->load_translation_file(filename, data->lang_id);
}

/**
 * Loads a translations directory
 *
 * @param directory The directory structure to read.
 * @param lang_id The language identifier.
 * @return The amount of strings loaded.
 */
static int script_load_translation(const char *directory, uint8 lang_id)
{
	struct load_translation_data data = { 0 };
	data.lang_id = lang_id;

	nullpo_ret(directory);

	script->add_language(script->get_translation_dir_name(directory));
	if (lang_id >= atcommand->max_message_table)
		atcommand->expand_message_table();

	findfile(directory, ".po", script_load_translation_sub, &data);

	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' translations in '"CL_WHITE"%s"CL_RESET"'.\n", data.translation_count, directory);
	return data.translation_count;
}

/**
 *
 **/
static void script_clear_translations(bool reload)
{
	uint32 i;

	if( script->string_list )
		aFree(script->string_list);

	script->string_list = NULL;
	script->string_list_pos = 0;
	script->string_list_size = 0;

	while (VECTOR_LENGTH(script->translation_buf) > 0) {
		aFree(VECTOR_POP(script->translation_buf));
	}
	VECTOR_CLEAR(script->translation_buf);

	if( script->languages ) {
		for(i = 0; i < script->max_lang_id; i++)
			aFree(script->languages[i]);
		aFree(script->languages);
	}
	script->languages = NULL;
	script->max_lang_id = 0;

	if( script->translation_db ) {
		script->translation_db->clear(script->translation_db,script->translation_db_destroyer);
	}

	if( reload )
		script->load_translations();
}

/**
 *
 **/
static int script_translation_db_destroyer(union DBKey key, struct DBData *data, va_list ap)
{
	struct DBMap *string_db = DB->data2ptr(data);

	if( db_size(string_db) ) {
		struct string_translation *st = NULL;
		struct DBIterator *iter = db_iterator(string_db);

		for( st = dbi_first(iter); dbi_exists(iter); st = dbi_next(iter) ) {
			aFree(st);
		}
		dbi_destroy(iter);
	}

	db_destroy(string_db);
	return 0;
}

/**
 *
 **/
static void script_parser_clean_leftovers(void)
{
	VECTOR_CLEAR(script->buf);

	if( script->translation_db ) {
		script->translation_db->destroy(script->translation_db,script->translation_db_destroyer);
		script->translation_db = NULL;
	}

	VECTOR_CLEAR(script->parse_simpleexpr_strbuf);
}

/**
 * Performs cleanup after all parsing is processed
 **/
static int script_parse_cleanup_timer(int tid, int64 tick, int id, intptr_t data)
{
	script->parser_clean_leftovers();

	script->parse_cleanup_timer_id = INVALID_TIMER;

	return 0;
}

/*==========================================
 * Initialization
 *------------------------------------------*/
static void do_init_script(bool minimal)
{
	script->parse_cleanup_timer_id = INVALID_TIMER;
	VECTOR_INIT(script->parse_simpleexpr_strbuf);

	script->st_db = idb_alloc(DB_OPT_BASE);
	script->userfunc_db = strdb_alloc(DB_OPT_DUP_KEY,0);
	script->autobonus_db = strdb_alloc(DB_OPT_DUP_KEY,0);

	script->st_ers = ers_new(sizeof(struct script_state), "script.c::st_ers", ERS_OPT_CLEAN|ERS_OPT_FLEX_CHUNK);
	script->stack_ers = ers_new(sizeof(struct script_stack), "script.c::script_stack", ERS_OPT_NONE|ERS_OPT_FLEX_CHUNK);
	script->array_ers = ers_new(sizeof(struct script_array), "script.c::array_ers", ERS_OPT_CLEAN|ERS_OPT_CLEAR);

	ers_chunk_size(script->st_ers, 10);
	ers_chunk_size(script->stack_ers, 10);

	VECTOR_INIT(script->hq);
	VECTOR_INIT(script->hqi);

	script->parse_builtin();
	script->read_constdb(false);
	script->load_parameters();
	script->hardcoded_constants();

	if (minimal)
		return;

	mapreg->init();
	script->load_translations();
}

static int script_reload(void)
{
	int i;
	struct DBIterator *iter;
	struct script_state *st;

#ifdef ENABLE_CASE_CHECK
	script->global_casecheck.clear();
#endif // ENABLE_CASE_CHECK

	iter = db_iterator(script->st_db);

	for( st = dbi_first(iter); dbi_exists(iter); st = dbi_next(iter) ) {
		script->free_state(st);
	}

	dbi_destroy(iter);

	script->userfunc_db->clear(script->userfunc_db, script->db_free_code_sub);
	script->label_count = 0;

	for( i = 0; i < atcommand->binding_count; i++ ) {
		aFree(atcommand->binding[i]->at_groups);
		aFree(atcommand->binding[i]->char_groups);
		aFree(atcommand->binding[i]);
	}

	if( atcommand->binding_count != 0 )
		aFree(atcommand->binding);

	atcommand->binding_count = 0;

	db_clear(script->st_db);

	script->clear_translations(true);

	if( script->parse_cleanup_timer_id != INVALID_TIMER ) {
		timer->delete(script->parse_cleanup_timer_id,script->parse_cleanup_timer);
		script->parse_cleanup_timer_id = INVALID_TIMER;
	}

	script->read_constdb(true);
	itemdb->name_constants();
	clan->set_constants();

	mapreg->reload();
	sysinfo->vcsrevision_reload();

	return 0;
}
/* returns name of current function being run, from within the stack [Ind/Hercules] */
static const char *script_getfuncname(struct script_state *st)
{
	struct script_data *data;

	nullpo_retr(NULL, st);
	data = &st->stack->stack_data[st->start];

	if( data->type == C_NAME && script->str_data[data->u.num].type == C_FUNC )
		return script->get_str(script_getvarid(data->u.num));

	return NULL;
}

/**
 * Writes a string to a StringBuf by combining a format string and a set of
 * arguments taken from the current script state (caller script function
 * arguments).
 *
 * @param[in]  st    Script state (must have at least a string at index
 *                   'start').
 * @param[in]  start Index of the format string argument.
 * @param[out] out   Output string buffer (managed by the caller, must be
 *                   already initialized)
 * @retval false if an error occurs.
 */
static bool script_sprintf_helper(struct script_state *st, int start, struct StringBuf *out)
{
	const char *format = NULL;
	const char *p = NULL, *np = NULL;
	char *buf = NULL;
	int buf_len = 0;
	int lastarg = start;
	int argc = script_lastdata(st) + 1;

	nullpo_retr(-1, out);
	Assert_retr(-1, start >= 2 && start <= argc);
	Assert_retr(-1, script_hasdata(st, start));

	p = format = script_getstr(st, start);

	/*
	 * format-string = "" / *(text / placeholder)
	 * placeholder = "%%" / "%n" / std-placeholder
	 * std-placeholder = "%" [pos-parameter] [flags] [width] [precision] [length] type
	 * pos-parameter = number "$"
	 * flags = *("-" / "+" / "0" / SP)
	 * width = number / ("*" [pos-parameter])
	 * precision = "." (number / ("*" [pos-parameter]))
	 * length = "hh" / "h" / "l" / "ll" / "L" / "z" / "j" / "t"
	 * type = "d" / "i" / "u" / "f" / "F" / "e" / "E" / "g" / "G" / "x" / "X" / "o" / "s" / "c" / "p" / "a" / "A"
	 * number = digit-nonzero *DIGIT
	 * digit-nonzero = "1" / "2" / "3" / "4" / "5" / "6" / "7" / "8" / "9"
	 */

	while ((np = strchr(p, '%')) != NULL) {
		bool flag_plus = false, flag_minus = false, flag_zero = false, flag_space = false;
		bool positional_arg = false;
		int width = 0, nextarg = lastarg + 1, thisarg = nextarg;

		if (p != np) {
			int len = (int)(np - p + 1);
			if (buf_len < len) {
				RECREATE(buf, char, len);
				buf_len = len;
			}
			safestrncpy(buf, p, len);
			StrBuf->AppendStr(out, buf);
		}
		np++;

		// placeholder = "%%" ; (special case)
		if (*np == '%') {
			StrBuf->AppendStr(out, "%");
			p = np + 1;
			continue;
		}
		// placeholder = "%n" ; (ignored)
		if (*np == 'n') {
			ShowWarning("script_sprintf_helper: Format %%n not supported! Skipping...\n");
			script->reportsrc(st);
			lastarg = nextarg;
			p = np + 1;
			continue;
		}

		// std-placeholder = "%" [pos-parameter] [flags] [width] [precision] [length] type

		// pos-parameter = number "$"
		if (ISDIGIT(*np) && *np != '0') {
			const char *pp = np;
			while (ISDIGIT(*pp))
				pp++;
			if (*pp == '$') {
				thisarg = atoi(np) + start;
				positional_arg = true;
				np = pp + 1;
			}
		}

		if (thisarg >= argc) {
			ShowError("buildin_sprintf: Not enough arguments passed!\n");
			if (buf != NULL)
				aFree(buf);
			return false;
		}

		// flags = *("-" / "+" / "0" / SP)
		while (true) {
			if (*np == '-') {
				flag_minus = true;
			} else if (*np == '+') {
				flag_plus = true;
			} else if (*np == ' ') {
				flag_space = true;
			} else if (*np == '0') {
				flag_zero = true;
			} else {
				break;
			}
			np++;
		}

		// width = number / ("*" [pos-parameter])
		if (ISDIGIT(*np)) {
			width = atoi(np);
			while (ISDIGIT(*np))
				np++;
		} else if (*np == '*') {
			bool positional_widtharg = false;
			int width_arg;
			np++;
			// pos-parameter = number "$"
			if (ISDIGIT(*np) && *np != '0') {
				const char *pp = np;
				while (ISDIGIT(*pp))
					pp++;
				if (*pp == '$') {
					width_arg = atoi(np) + start;
					positional_widtharg = true;
					np = pp + 1;
				}
			}
			if (!positional_widtharg) {
				width_arg = nextarg;
				nextarg++;
				if (!positional_arg)
					thisarg++;
			}

			if (width_arg >= argc || thisarg >= argc) {
				ShowError("buildin_sprintf: Not enough arguments passed!\n");
				if (buf != NULL)
					aFree(buf);
				return false;
			}
			width = script_getnum(st, width_arg);
		}

		// precision = "." (number / ("*" [pos-parameter])) ; (not needed/implemented)

		// length = "hh" / "h" / "l" / "ll" / "L" / "z" / "j" / "t" ; (not needed/implemented)

		// type = "d" / "i" / "u" / "f" / "F" / "e" / "E" / "g" / "G" / "x" / "X" / "o" / "s" / "c" / "p" / "a" / "A"
		if (buf_len < 16) {
			RECREATE(buf, char, 16);
			buf_len = 16;
		}
		{
			int i = 0;
			memset(buf, '\0', buf_len);
			buf[i++] = '%';
			if (flag_minus)
				buf[i++] = '-';
			if (flag_plus)
				buf[i++] = '+';
			else if (flag_space) // ignored if '+' is specified
				buf[i++] = ' ';
			if (flag_zero)
				buf[i++] = '0';
			if (width > 0)
				safesnprintf(buf + i, buf_len - i - 1, "%d", width);
		}
		buf[(int)strlen(buf)] = *np;
		switch (*np) {
		case 'd':
		case 'i':
		case 'u':
		case 'x':
		case 'X':
		case 'o':
			// Piggyback printf
			StrBuf->Printf(out, buf, script_getnum(st, thisarg));
			break;
		case 's':
			// Piggyback printf
			StrBuf->Printf(out, buf, script_getstr(st, thisarg));
			break;
		case 'c':
		{
			const char *str = script_getstr(st, thisarg);
			// Piggyback printf
			StrBuf->Printf(out, buf, str[0]);
		}
			break;
		case 'f':
		case 'F':
		case 'e':
		case 'E':
		case 'g':
		case 'G':
		case 'p':
		case 'a':
		case 'A':
			ShowWarning("buildin_sprintf: Format %%%c not supported! Skipping...\n", *np);
			script->reportsrc(st);
			lastarg = nextarg;
			p = np + 1;
			continue;
		default:
			ShowError("buildin_sprintf: Invalid format string.\n");
			if (buf != NULL)
				aFree(buf);
			return false;
		}
		lastarg = nextarg;
		p = np + 1;
	}

	// Append the remaining part
	if (p != NULL)
		StrBuf->AppendStr(out, p);

	if (buf != NULL)
		aFree(buf);

	return true;
}

//-----------------------------------------------------------------------------
// buildin functions
//

/////////////////////////////////////////////////////////////////////
// NPC interaction
//

/// Appends a message to the npc dialog.
/// If a dialog doesn't exist yet, one is created.
///
/// mes "<message>";
static BUILDIN(mes)
{
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	if (script_hasdata(st, 2))
		clif->scriptmes(sd, st->oid, script_getstr(st, 2));
	else
		clif->scriptmes(sd, st->oid, "");

	return true;
}

/**
 * Appends a message to the npc dialog, applying format string conversions (see
 * sprintf).
 *
 * If a dialog doesn't exist yet, one is created.
 *
 * @code
 *    mes "<message>";
 * @endcode
 */
static BUILDIN(mesf)
{
	struct map_session_data *sd = script->rid2sd(st);
	struct StringBuf buf;

	if (sd == NULL)
		return true;

	StrBuf->Init(&buf);

	if (!script->sprintf_helper(st, 2, &buf)) {
		StrBuf->Destroy(&buf);
		return false;
	}

	clif->scriptmes(sd, st->oid, StrBuf->Value(&buf));
	StrBuf->Destroy(&buf);

	return true;
}

/// Displays the button 'next' in the npc dialog.
/// The dialog text is cleared and the script continues when the button is pressed.
///
/// next;
static BUILDIN(next)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;
#ifdef SECURE_NPCTIMEOUT
	sd->npc_idle_type = NPCT_WAIT;
#endif
	st->state = STOP;
	clif->scriptnext(sd, st->oid);
	return true;
}

/// Clears the NPC dialog and continues the script without press next button.
///
/// mesclear();
static BUILDIN(mesclear)
{
	struct map_session_data *sd = script->rid2sd(st);

	if (sd != NULL)
		clif->scriptclear(sd, st->oid);

	return true;
}

/// Ends the script and displays the button 'close' on the npc dialog.
/// The dialog is closed when the button is pressed.
///
/// close;
static BUILDIN(close)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	st->state = sd->state.dialog == 1 ? CLOSE : END;
	clif->scriptclose(sd, st->oid);
	return true;
}

/// Displays the button 'close' on the npc dialog.
/// The dialog is closed and the script continues when the button is pressed.
///
/// close2;
static BUILDIN(close2)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	if( sd->state.dialog == 1 )
		st->state = STOP;
	else {
		ShowWarning("misuse of 'close2'! trying to use it without prior dialog! skipping...\n");
		script->reportsrc(st);
	}

	clif->scriptclose(sd, st->oid);
	return true;
}

/// Counts the number of valid and total number of options in 'str'
/// If max_count > 0 the counting stops when that valid option is reached
/// total is incremented for each option (NULL is supported)
static int menu_countoptions(const char *str, int max_count, int *total)
{
	int count = 0;
	int bogus_total;

	nullpo_ret(str);
	if( total == NULL )
		total = &bogus_total;
	++(*total);

	// initial empty options
	while( *str == ':' )
	{
		++str;
		++(*total);
	}
	// count menu options
	while( *str != '\0' )
	{
		++count;
		--max_count;
		if( max_count == 0 )
			break;
		while( *str != ':' && *str != '\0' )
			++str;
		while( *str == ':' )
		{
			++str;
			++(*total);
		}
	}
	return count;
}

/// Displays a menu with options and goes to the target label.
/// The script is stopped if cancel is pressed.
/// Options with no text are not displayed in the client.
///
/// Options can be grouped together, separated by the character ':' in the text:
///   ex: menu "A:B:C",L_target;
/// All these options go to the specified target label.
///
/// The index of the selected option is put in the variable @menu.
/// Indexes start with 1 and are consistent with grouped and empty options.
///   ex: menu "A::B",-,"",L_Impossible,"C",-;
///       // displays "A", "B" and "C", corresponding to indexes 1, 3 and 5
///
/// NOTE: the client closes the npc dialog when cancel is pressed
///
/// menu "<option_text>",<target_label>{,"<option_text>",<target_label>,...};
static BUILDIN(menu)
{
	int i;
	const char* text;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

#ifdef SECURE_NPCTIMEOUT
	sd->npc_idle_type = NPCT_MENU;
#endif

	// TODO detect multiple scripts waiting for input at the same time, and what to do when that happens
	if (sd->state.menu_or_input == 0) {
		struct StringBuf buf;

		if (script_lastdata(st) % 2 == 0) {
			// argument count is not even (1st argument is at index 2)
			ShowError("script:menu: illegal number of arguments (%d).\n", (script_lastdata(st) - 1));
			st->state = END;
			return false;
		}

		StrBuf->Init(&buf);
		sd->npc_menu = 0;
		for (i = 2; i < script_lastdata(st); i += 2) {
			// target label
			struct script_data* data = script_getdata(st, i+1);
			// menu options
			text = script_getstr(st, i);

			if( !data_islabel(data) )
			{// not a label
				StrBuf->Destroy(&buf);
				ShowError("script:menu: argument #%d (from 1) is not a label or label not found.\n", i);
				script->reportdata(data);
				st->state = END;
				return false;
			}

			// append option(s)
			if( text[0] == '\0' )
				continue;// empty string, ignore
			if( sd->npc_menu > 0 )
				StrBuf->AppendStr(&buf, ":");
			StrBuf->AppendStr(&buf, text);
			sd->npc_menu += script->menu_countoptions(text, 0, NULL);
		}
		st->state = RERUNLINE;
		sd->state.menu_or_input = 1;

		/* menus beyond this length crash the client (see bugreport:6402) */
		if( StrBuf->Length(&buf) >= MAX_MENU_LENGTH - 1 ) {
			struct npc_data * nd = map->id2nd(st->oid);
			char* menu;
			CREATE(menu, char, MAX_MENU_LENGTH);
			safestrncpy(menu, StrBuf->Value(&buf), MAX_MENU_LENGTH - 1);
			ShowWarning("NPC Menu too long! (source:%s / length:%d)\n",nd?nd->name:"Unknown",StrBuf->Length(&buf));
			clif->scriptmenu(sd, st->oid, menu);
			aFree(menu);
		} else
			clif->scriptmenu(sd, st->oid, StrBuf->Value(&buf));

		StrBuf->Destroy(&buf);

		if( sd->npc_menu >= MAX_MENU_OPTIONS )
		{// client supports only up to 254 entries; 0 is not used and 255 is reserved for cancel; excess entries are displayed but cause 'uint8' overflow
			ShowWarning("buildin_menu: Too many options specified (current=%d, max=%d).\n", sd->npc_menu, MAX_MENU_OPTIONS - 1);
			script->reportsrc(st);
		}
	}
	else if( sd->npc_menu == MAX_MENU_OPTIONS )
	{// Cancel was pressed
		sd->state.menu_or_input = 0;
		st->state = END;
	}
	else
	{// goto target label
		int menu = 0;

		sd->state.menu_or_input = 0;
		if( sd->npc_menu <= 0 )
		{
			ShowDebug("script:menu: unexpected selection (%d)\n", sd->npc_menu);
			st->state = END;
			return false;
		}

		// get target label
		for( i = 2; i < script_lastdata(st); i += 2 )
		{
			text = script_getstr(st, i);
			sd->npc_menu -= script->menu_countoptions(text, sd->npc_menu, &menu);
			if( sd->npc_menu <= 0 )
				break;// entry found
		}
		if( sd->npc_menu > 0 )
		{// Invalid selection
			ShowDebug("script:menu: selection is out of range (%d pairs are missing?) - please report this\n", sd->npc_menu);
			st->state = END;
			return false;
		}
		if( !data_islabel(script_getdata(st, i + 1)) )
		{// TODO remove this temporary crash-prevention code (fallback for multiple scripts requesting user input)
			ShowError("script:menu: unexpected data in label argument\n");
			script->reportdata(script_getdata(st, i + 1));
			st->state = END;
			return false;
		}
		pc->setreg(sd, script->add_variable("@menu"), menu);
		st->pos = script_getnum(st, i + 1);
		st->state = GOTO;
	}
	return true;
}

/// Displays a menu with options and returns the selected option.
/// Behaves like 'menu' without the target labels.
///
/// select(<option_text>{,<option_text>,...}) -> <selected_option>
///
/// @see menu
static BUILDIN(select)
{
	int i;
	const char* text;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

#ifdef SECURE_NPCTIMEOUT
	sd->npc_idle_type = NPCT_MENU;
#endif

	if( sd->state.menu_or_input == 0 ) {
		struct StringBuf buf;

		StrBuf->Init(&buf);
		sd->npc_menu = 0;
		for( i = 2; i <= script_lastdata(st); ++i ) {
			text = script_getstr(st, i);

			if( sd->npc_menu > 0 )
				StrBuf->AppendStr(&buf, ":");

			StrBuf->AppendStr(&buf, text);
			sd->npc_menu += script->menu_countoptions(text, 0, NULL);
		}

		st->state = RERUNLINE;
		sd->state.menu_or_input = 1;

		/* menus beyond this length crash the client (see bugreport:6402) */
		if( StrBuf->Length(&buf) >= MAX_MENU_LENGTH - 1 ) {
			struct npc_data * nd = map->id2nd(st->oid);
			char* menu;
			CREATE(menu, char, MAX_MENU_LENGTH);
			safestrncpy(menu, StrBuf->Value(&buf), MAX_MENU_LENGTH - 1);
			ShowWarning("NPC Menu too long! (source:%s / length:%d)\n",nd?nd->name:"Unknown",StrBuf->Length(&buf));
			clif->scriptmenu(sd, st->oid, menu);
			aFree(menu);
		} else
			clif->scriptmenu(sd, st->oid, StrBuf->Value(&buf));
		StrBuf->Destroy(&buf);

		if( sd->npc_menu >= MAX_MENU_OPTIONS ) {
			ShowWarning("buildin_select: Too many options specified (current=%d, max=%d).\n", sd->npc_menu, MAX_MENU_OPTIONS - 1);
			script->reportsrc(st);
		}
	} else if(sd->npc_menu == MAX_MENU_OPTIONS) { // Cancel was pressed
		sd->state.menu_or_input = 0;

		if (strncmp(get_buildin_name(st), "prompt", 6) == 0) {
			pc->setreg(sd, script->add_variable("@menu"), MAX_MENU_OPTIONS);
			script_pushint(st, MAX_MENU_OPTIONS); // XXX: we should really be pushing -1 instead
			st->state = RUN;
		} else {
			st->state = END;
		}
	} else {// return selected option
		int menu = 0;

		sd->state.menu_or_input = 0;
		for( i = 2; i <= script_lastdata(st); ++i ) {
			text = script_getstr(st, i);
			sd->npc_menu -= script->menu_countoptions(text, sd->npc_menu, &menu);
			if( sd->npc_menu <= 0 )
				break;// entry found
		}
		pc->setreg(sd, script->add_variable("@menu"), menu); // TODO: throw a deprecation warning for scripts using @menu
		script_pushint(st, menu);
		st->state = RUN;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////
// ...
//

/// Jumps to the target script label.
///
/// goto <label>;
static BUILDIN(goto)
{
	if( !data_islabel(script_getdata(st,2)) )
	{
		ShowError("script:goto: not a label\n");
		script->reportdata(script_getdata(st,2));
		st->state = END;
		return false;
	}

	st->pos = script_getnum(st,2);
	st->state = GOTO;
	return true;
}

/*==========================================
 * user-defined function call
 *------------------------------------------*/
static BUILDIN(callfunc)
{
	int i, j;
	struct script_retinfo* ri;
	struct script_code* scr;
	const char* str = script_getstr(st,2);
	struct reg_db *ref = NULL;

	scr = (struct script_code*)strdb_get(script->userfunc_db, str);
	if( !scr )
	{
		ShowError("script:callfunc: function not found! [%s]\n", str);
		st->state = END;
		return false;
	}

	ref = (struct reg_db *)aCalloc(sizeof(struct reg_db), 2);
	ref[0].vars = st->stack->scope.vars;
	if (!st->stack->scope.arrays)
		st->stack->scope.arrays = idb_alloc(DB_OPT_BASE); // TODO: Can this happen? when?
	ref[0].arrays = st->stack->scope.arrays;
	ref[1].vars = st->script->local.vars;
	if (!st->script->local.arrays)
		st->script->local.arrays = idb_alloc(DB_OPT_BASE); // TODO: Can this happen? when?
	ref[1].arrays = st->script->local.arrays;

	for( i = st->start+3, j = 0; i < st->end; i++, j++ ) {
		struct script_data* data = script->push_copy(st->stack,i);
		if( data_isreference(data) && !data->ref ) {
			const char* name = reference_getname(data);
			if( name[0] == '.' ) {
				data->ref = (name[1] == '@' ? &ref[0] : &ref[1]);
			}
		}
	}

	CREATE(ri, struct script_retinfo, 1);
	ri->script       = st->script;              // script code
	ri->scope.vars   = st->stack->scope.vars;   // scope variables
	ri->scope.arrays = st->stack->scope.arrays; // scope arrays
	ri->pos          = st->pos;                 // script location
	ri->nargs        = j;                       // argument count
	ri->defsp        = st->stack->defsp;        // default stack pointer
	script->push_retinfo(st->stack, ri, ref);

	st->pos = 0;
	st->script = scr;
	st->stack->defsp = st->stack->sp;
	st->state = GOTO;
	st->stack->scope.vars = i64db_alloc(DB_OPT_RELEASE_DATA);
	st->stack->scope.arrays = idb_alloc(DB_OPT_BASE);

	if( !st->script->local.vars )
		st->script->local.vars = i64db_alloc(DB_OPT_RELEASE_DATA);

	return true;
}
/*==========================================
 * subroutine call
 *------------------------------------------*/
static BUILDIN(callsub)
{
	int i,j;
	struct script_retinfo* ri;
	int pos = script_getnum(st,2);
	struct reg_db *ref = NULL;

	if( !data_islabel(script_getdata(st,2)) && !data_isfunclabel(script_getdata(st,2)) )
	{
		ShowError("script:callsub: argument is not a label\n");
		script->reportdata(script_getdata(st,2));
		st->state = END;
		return false;
	}

	ref = (struct reg_db *)aCalloc(sizeof(struct reg_db), 1);
	ref[0].vars = st->stack->scope.vars;
	if (!st->stack->scope.arrays)
		st->stack->scope.arrays = idb_alloc(DB_OPT_BASE); // TODO: Can this happen? when?
	ref[0].arrays = st->stack->scope.arrays;

	for( i = st->start+3, j = 0; i < st->end; i++, j++ ) {
		struct script_data* data = script->push_copy(st->stack,i);
		if( data_isreference(data) && !data->ref ) {
			const char* name = reference_getname(data);
			if( name[0] == '.' && name[1] == '@' ) {
				data->ref = &ref[0];
			}
		}
	}

	CREATE(ri, struct script_retinfo, 1);
	ri->script       = st->script;              // script code
	ri->scope.vars   = st->stack->scope.vars;   // scope variables
	ri->scope.arrays = st->stack->scope.arrays; // scope arrays
	ri->pos          = st->pos;                 // script location
	ri->nargs        = j;                       // argument count
	ri->defsp        = st->stack->defsp;        // default stack pointer
	script->push_retinfo(st->stack, ri, ref);

	st->pos = pos;
	st->stack->defsp = st->stack->sp;
	st->state = GOTO;
	st->stack->scope.vars = i64db_alloc(DB_OPT_RELEASE_DATA);
	st->stack->scope.arrays = idb_alloc(DB_OPT_BASE);

	return true;
}

/// Retrieves an argument provided to callfunc/callsub.
/// If the argument doesn't exist
///
/// getarg(<index>{,<default_value>}) -> <value>
static BUILDIN(getarg)
{
	struct script_retinfo* ri;
	int idx;

	if( st->stack->defsp < 1 || st->stack->stack_data[st->stack->defsp - 1].type != C_RETINFO )
	{
		ShowError("script:getarg: no callfunc or callsub!\n");
		st->state = END;
		return false;
	}
	ri = st->stack->stack_data[st->stack->defsp - 1].u.ri;

	idx = script_getnum(st,2);

	if( idx >= 0 && idx < ri->nargs )
		script->push_copy(st->stack, st->stack->defsp - 1 - ri->nargs + idx);
	else if( script_hasdata(st,3) )
		script_pushcopy(st, 3);
	else
	{
		ShowError("script:getarg: index (idx=%d) out of range (nargs=%d) and no default value found\n", idx, ri->nargs);
		st->state = END;
		return false;
	}

	return true;
}

/// Returns from the current function, optionaly returning a value from the functions.
/// Don't use outside script functions.
///
/// return;
/// return <value>;
static BUILDIN(return)
{
	st->state = RETFUNC;

	if( st->stack->defsp < 1 || st->stack->stack_data[st->stack->defsp-1].type != C_RETINFO ) {
		// Incorrect usage of return outside the scope of a function or subroutine.
		return true; // No need for further processing, running script is about to be aborted.
	}

	if( script_hasdata(st,2) )
	{// return value
		struct script_data* data;
		script_pushcopy(st, 2);
		data = script_getdatatop(st, -1);
		if( data_isreference(data) ) {
			const char* name = reference_getname(data);
			if( name[0] == '.' && name[1] == '@' ) {
				// scope variable
				if( !data->ref || data->ref->vars == st->stack->scope.vars )
					script->get_val(st, data);// current scope, convert to value
				if( data->ref && data->ref->vars == st->stack->stack_data[st->stack->defsp-1].u.ri->scope.vars )
					data->ref = NULL; // Reference to the parent scope, remove reference pointer
			} else if( name[0] == '.' ) {
				// npc variable
				if( !data->ref ) {
					// npc variable without a reference set, link to current script
					data->ref = (struct reg_db *)aCalloc(sizeof(struct reg_db), 1);
					script->add_pending_ref(st, data->ref);
					data->ref->vars = st->script->local.vars;
					if( !st->script->local.arrays )
						st->script->local.arrays = idb_alloc(DB_OPT_BASE);
					data->ref->arrays = st->script->local.arrays;
				} else if( data->ref->vars == st->stack->stack_data[st->stack->defsp-1].u.ri->script->local.vars ) {
					data->ref = NULL; // Reference to the parent scope's script, remove reference pointer.
				}
			}
		}
	}
	else
	{// no return value
		script_pushnil(st);
	}

	return true;
}

/// Returns a random number from 0 to <range>-1.
/// Or returns a random number from <min> to <max>.
/// If <min> is greater than <max>, their numbers are switched.
/// rand(<range>) -> <int>
/// rand(<min>,<max>) -> <int>
static BUILDIN(rand)
{
	int range;
	int min;

	if (script_hasdata(st,3)) {
		// min,max
		int max;
		min = script_getnum(st,2);
		max = script_getnum(st,3);
		if( max < min )
			swap(min, max);
		range = max - min + 1;
	} else {
		// range
		min = 0;
		range = script_getnum(st,2);
	}
	if (range <= 1)
		script_pushint(st, min);
	else
		script_pushint(st, rnd()%range + min);

	return true;
}

/*==========================================
 * Warp sd to str,x,y or Random or SavePoint/Save
 *------------------------------------------*/
static BUILDIN(warp)
{
	int ret;
	int x,y;
	int warp_clean = 1;
	const char* str;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	str = script_getstr(st,2);
	x = script_getnum(st,3);
	y = script_getnum(st,4);

	if (script_hasdata(st, 5)) {
		warp_clean = script_getnum(st, 5);
	}

	sd->state.warp_clean = warp_clean;
	if(strcmp(str,"Random")==0)
		ret = pc->randomwarp(sd,CLR_TELEPORT);
	else if(strcmp(str,"SavePoint")==0 || strcmp(str,"Save")==0)
		ret = pc->setpos(sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,CLR_TELEPORT);
	else
		ret = pc->setpos(sd,script->mapindexname2id(st,str),x,y,CLR_OUTSIGHT);

	if( ret ) {
		ShowError("buildin_warp: moving player '%s' to \"%s\",%d,%d failed.\n", sd->status.name, str, x, y);
		script->reportsrc(st);
	}

	return true;
}
/*==========================================
 * Warp a specified area
 *------------------------------------------*/
static int buildin_areawarp_sub(struct block_list *bl, va_list ap)
{
	int x2,y2,x3,y3;
	unsigned int index;
	struct map_session_data *sd = NULL;

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_PC);
	sd = BL_UCAST(BL_PC, bl);

	index = va_arg(ap,unsigned int);
	x2 = va_arg(ap,int);
	y2 = va_arg(ap,int);
	x3 = va_arg(ap,int);
	y3 = va_arg(ap,int);

	if (index == 0) {
		pc->randomwarp(sd, CLR_TELEPORT);
	} else if (x3 != 0 && y3 != 0) {
		int max, tx, ty, j = 0;
		int16 m;

		m = map->mapindex2mapid(index);

		// choose a suitable max number of attempts
		if( (max = (y3-y2+1)*(x3-x2+1)*3) > 1000 )
			max = 1000;

		// find a suitable map cell
		do {
			tx = rnd()%(x3-x2+1)+x2;
			ty = rnd()%(y3-y2+1)+y2;
			j++;
		} while (map->getcell(m, bl, tx, ty, CELL_CHKNOPASS) && j < max);

		pc->setpos(sd, index, tx, ty, CLR_OUTSIGHT);
	} else {
		pc->setpos(sd, index, x2, y2, CLR_OUTSIGHT);
	}
	return 0;
}
static BUILDIN(areawarp)
{
	int16 m, x0,y0,x1,y1, x2,y2,x3=0,y3=0;
	unsigned int index;
	const char *str;
	const char *mapname;

	mapname = script_getstr(st,2);
	x0  = script_getnum(st,3);
	y0  = script_getnum(st,4);
	x1  = script_getnum(st,5);
	y1  = script_getnum(st,6);
	str = script_getstr(st,7);
	x2  = script_getnum(st,8);
	y2  = script_getnum(st,9);

	if( script_hasdata(st,10) && script_hasdata(st,11) ) { // Warp area to area
		if( (x3 = script_getnum(st,10)) < 0 || (y3 = script_getnum(st,11)) < 0 ) {
			x3 = 0;
			y3 = 0;
		} else if( x3 && y3 ) {
			// normalize x3/y3 coordinates
			if( x3 < x2 ) swap(x3,x2);
			if( y3 < y2 ) swap(y3,y2);
		}
	}

	if( (m = map->mapname2mapid(mapname)) < 0 )
		return true;

	if( strcmp(str,"Random") == 0 )
		index = 0;
	else if( !(index=script->mapindexname2id(st,str)) )
		return true;

	map->foreachinarea(script->buildin_areawarp_sub, m,x0,y0,x1,y1, BL_PC, index,x2,y2,x3,y3);
	return true;
}

/*==========================================
 * areapercentheal <map>,<x1>,<y1>,<x2>,<y2>,<hp>,<sp>
 *------------------------------------------*/
static int buildin_areapercentheal_sub(struct block_list *bl, va_list ap)
{
	int hp = va_arg(ap, int);
	int sp = va_arg(ap, int);
	struct map_session_data *sd = NULL;

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_PC);
	sd = BL_UCAST(BL_PC, bl);

	pc->percentheal(sd, hp, sp);
	return 0;
}

static BUILDIN(areapercentheal)
{
	int hp,sp,m;
	const char *mapname;
	int x0,y0,x1,y1;

	mapname=script_getstr(st,2);
	x0=script_getnum(st,3);
	y0=script_getnum(st,4);
	x1=script_getnum(st,5);
	y1=script_getnum(st,6);
	hp=script_getnum(st,7);
	sp=script_getnum(st,8);

	if( (m=map->mapname2mapid(mapname))< 0)
		return true;

	map->foreachinarea(script->buildin_areapercentheal_sub,m,x0,y0,x1,y1,BL_PC,hp,sp);
	return true;
}

/*==========================================
 * warpchar [LuzZza]
 * Useful for warp one player from
 * another player npc-session.
 * Using: warpchar "mapname",x,y,Char_ID;
 *------------------------------------------*/
static BUILDIN(warpchar)
{
	int x,y,a;
	const char *str;
	struct map_session_data *sd;

	str=script_getstr(st,2);
	x=script_getnum(st,3);
	y=script_getnum(st,4);
	a=script_getnum(st,5);

	sd = script->charid2sd(st, a);
	if (sd == NULL)
		return true;

	if(strcmp(str, "Random") == 0)
		pc->randomwarp(sd, CLR_TELEPORT);
	else
		if(strcmp(str, "SavePoint") == 0)
			pc->setpos(sd, sd->status.save_point.map,sd->status.save_point.x, sd->status.save_point.y, CLR_TELEPORT);
		else
			pc->setpos(sd, script->mapindexname2id(st,str), x, y, CLR_TELEPORT);

	return true;
}

/**
 * Warps a party to a specific/random map or save point.
 *
 * @code{.herc}
 *	warpparty("<to map name>", <x>, <y>, <party id>{{, <ignore mapflags>}, "<from map name>"{, <include leader>}});
 * @endcode
 *
 **/
static BUILDIN(warpparty)
{
	const int p_id = script_getnum(st, 5);
	struct party_data *p = party->search(p_id);

	if (p == NULL) {
		ShowError("script:%s: Party not found! (%d)\n", script->getfuncname(st), p_id);
		script_pushint(st, 0);
		return false;
	}

	const char *m_name_to = script_getstr(st, 2);
	const int type = (strcmp(m_name_to, "Random") == 0) ? 0 :
			 (strcmp(m_name_to, "SavePointAll") == 0) ? 1 :
			 (strcmp(m_name_to, "SavePoint") == 0) ? 2 :
			 (strcmp(m_name_to, "Leader") == 0) ? 3 : 4;
	int map_index = 0;

	if (type == 4 && (map_index = script->mapindexname2id(st, m_name_to)) == 0) {
		ShowError("script:%s: Target map not found! (%s)\n", script->getfuncname(st), m_name_to);
		script_pushint(st, 0);
		return false;
	}

	int x = script_getnum(st, 3);
	int y = script_getnum(st, 4);
	struct map_session_data *p_sd;

	if (type == 3) {
		int idx;

		ARR_FIND(0, MAX_PARTY, idx, p->party.member[idx].leader == 1);

		if (idx == MAX_PARTY || (p_sd = p->data[idx].sd) == NULL) {
			ShowError("script:%s: Party leader not found!\n", script->getfuncname(st));
			script_pushint(st, 0);
			return false;
		}

		map_index = p_sd->mapindex;
		x = p_sd->bl.x;
		y = p_sd->bl.y;
	} else if (type == 2) {
		struct map_session_data *sd = script->rid2sd(st);

		if (sd == NULL) {
			ShowError("script:%s: No character attached for warp to save point!\n",
				  script->getfuncname(st));
			script_pushint(st, 0);
			return false;
		}

		map_index = sd->status.save_point.map;
		x = sd->status.save_point.x;
		y = sd->status.save_point.y;
	}

	const int offset = (script_hasdata(st, 6) && script_isinttype(st, 6)) ? 1 : 0;
	const bool ignore_mapflags = (offset == 1 && script_getnum(st, 6) != 0);
	const char *m_name_from = script_hasdata(st, 6 + offset) ? script_getstr(st, 6 + offset) : NULL;
	const bool include_leader = script_hasdata(st, 7 + offset) ? script_getnum(st, 7 + offset) : true;

	if (m_name_from != NULL && script->mapindexname2id(st, m_name_from) == 0) {
		ShowError("script:%s: Source map not found! (%s)\n", script->getfuncname(st), m_name_from);
		script_pushint(st, 0);
		return false;
	}

	for (int i = 0; i < MAX_PARTY; i++) {
		if ((p_sd = p->data[i].sd) == NULL || p_sd->status.party_id != p_id || pc_isdead(p_sd))
			continue;

		if (p->party.member[i].online == 0 || (!include_leader && p->party.member[i].leader == 1))
			continue;

		if (m_name_from != NULL && strcmp(m_name_from, map->list[p_sd->bl.m].name) != 0)
			continue;

		if (!ignore_mapflags) {
			if (((type == 0 || type > 2) && map->list[p_sd->bl.m].flag.nowarp == 1) ||
			    (type > 0 && map->list[p_sd->bl.m].flag.noreturn == 1))
				continue;
		}

		if (type == 1) {
			map_index = p_sd->status.save_point.map;
			x = p_sd->status.save_point.x;
			y = p_sd->status.save_point.y;
		}

		if (type > 0)
			pc->setpos(p_sd, map_index, x, y, CLR_TELEPORT);
		else
			pc->randomwarp(p_sd, CLR_TELEPORT);
	}

	script_pushint(st, 1);
	return true;
}

/**
 * Warps a guild to a specific/random map or save point.
 *
 * @code{.herc}
 *	warpguild("<to map name>", <x>, <y>, <guild id>{{, <ignore mapflags>}, "<from map name>"});
 * @endcode
 *
 **/
static BUILDIN(warpguild)
{
	const int g_id = script_getnum(st, 5);
	struct guild *g = guild->search(g_id);

	if (g == NULL) {
		ShowError("script:%s: Guild not found! (%d)\n", script->getfuncname(st), g_id);
		script_pushint(st, 0);
		return false;
	}

	const char *m_name_to = script_getstr(st, 2);
	const int type = (strcmp(m_name_to, "Random") == 0) ? 0 :
			 (strcmp(m_name_to, "SavePointAll") == 0) ? 1 :
			 (strcmp(m_name_to, "SavePoint") == 0) ? 2 : 3;
	int map_index = 0;

	if (type == 3 && (map_index = script->mapindexname2id(st, m_name_to)) == 0) {
		ShowError("script:%s: Target map not found! (%s)\n", script->getfuncname(st), m_name_to);
		script_pushint(st, 0);
		return false;
	}

	int x = script_getnum(st, 3);
	int y = script_getnum(st, 4);

	if (type == 2) {
		struct map_session_data *sd = script->rid2sd(st);

		if (sd == NULL) {
			ShowError("script:%s: No character attached for warp to save point!\n",
				  script->getfuncname(st));
			script_pushint(st, 0);
			return false;
		}

		map_index = sd->status.save_point.map;
		x = sd->status.save_point.x;
		y = sd->status.save_point.y;
	}

	const int offset = (script_hasdata(st, 6) && script_isinttype(st, 6)) ? 1 : 0;
	const bool ignore_mapflags = (offset == 1 && script_getnum(st, 6) != 0);
	const char *m_name_from = script_hasdata(st, 6 + offset) ? script_getstr(st, 6 + offset) : NULL;

	if (m_name_from != NULL && script->mapindexname2id(st, m_name_from) == 0) {
		ShowError("script:%s: Source map not found! (%s)\n", script->getfuncname(st), m_name_from);
		script_pushint(st, 0);
		return false;
	}

	for (int i = 0; i < MAX_GUILD; i++) {
		struct map_session_data *g_sd = g->member[i].sd;

		if (g->member[i].online == 0 || g_sd == NULL || g_sd->status.guild_id != g_id || pc_isdead(g_sd))
			continue;

		if (m_name_from != NULL && strcmp(m_name_from, map->list[g_sd->bl.m].name) != 0)
			continue;

		if (!ignore_mapflags) {
			if (((type == 0 || type > 2) && map->list[g_sd->bl.m].flag.nowarp == 1) ||
			    (type > 0 && map->list[g_sd->bl.m].flag.noreturn == 1))
				continue;
		}

		if (type == 1) {
			map_index = g_sd->status.save_point.map;
			x = g_sd->status.save_point.x;
			y = g_sd->status.save_point.y;
		}

		if (type > 0)
			pc->setpos(g_sd, map_index, x, y, CLR_TELEPORT);
		else
			pc->randomwarp(g_sd, CLR_TELEPORT);
	}

	script_pushint(st, 1);
	return true;
}

/*==========================================
 * Force Heal a player (hp and sp)
 *------------------------------------------*/
static BUILDIN(heal)
{
	int hp,sp;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	hp=script_getnum(st,2);
	sp=script_getnum(st,3);
	status->heal(&sd->bl, hp, sp, STATUS_HEAL_FORCED);
	return true;
}
/*==========================================
 * Heal a player by item (get vit bonus etc)
 *------------------------------------------*/
static BUILDIN(itemheal)
{
	struct map_session_data *sd;
	int hp,sp;

	hp=script_getnum(st,2);
	sp=script_getnum(st,3);

	if(script->potion_flag==1) {
		script->potion_hp = hp;
		script->potion_sp = sp;
		return true;
	}

	sd = script->rid2sd(st);
	if (sd == NULL)
		return true;
	pc->itemheal(sd,sd->itemid,hp,sp);
	return true;
}
/*==========================================
 *
 *------------------------------------------*/
static BUILDIN(percentheal)
{
	int hp,sp;
	struct map_session_data *sd;

	hp=script_getnum(st,2);
	sp=script_getnum(st,3);

	if(script->potion_flag==1) {
		script->potion_per_hp = hp;
		script->potion_per_sp = sp;
		return true;
	}

	sd = script->rid2sd(st);
	if (sd == NULL)
		return true;
#ifdef RENEWAL
	if( sd->sc.data[SC_EXTREMITYFIST2] )
		sp = 0;
#endif
	if (sd->sc.data[SC_BITESCAR]) {
		hp = 0;
	}
	pc->percentheal(sd, hp, sp);
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
static BUILDIN(jobchange)
{
	int class, upper=-1;

	class = script_getnum(st,2);
	if( script_hasdata(st,3) )
		upper=script_getnum(st,3);

	if (pc->db_checkid(class)) {
		struct map_session_data *sd = script->rid2sd(st);
		if (sd == NULL)
			return true;

		pc->jobchange(sd, class, upper);
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
static BUILDIN(jobname)
{
	int class = script_getnum(st,2);
	script_pushconststr(st, pc->job_name(class));
	return true;
}

/*
 * Get input from the player.
 * For numeric inputs the value is capped to the range [min,max]. Returns 1 if
 * the value was higher than 'max', -1 if lower than 'min' and 0 otherwise.
 * For string inputs it returns 1 if the string was longer than 'max', -1 is
 * shorter than 'min' and 0 otherwise.
 *
 * input(<var>{,<min>{,<max>}}) -> <int>
 */
static BUILDIN(input)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	struct script_data *data = script_getdata(st, 2);
	if (!data_isreference(data)) {
		ShowError("script:input: not a variable\n");
		script->reportdata(data);
		st->state = END;
		return false;
	}

	int64 uid  = reference_getuid(data);
	const char *name = reference_getname(data);
	int min = (script_hasdata(st, 3) ? script_getnum(st, 3) : script->config.input_min_value);
	int max = (script_hasdata(st, 4) ? script_getnum(st, 4) : script->config.input_max_value);

#ifdef SECURE_NPCTIMEOUT
	sd->npc_idle_type = NPCT_WAIT;
#endif

	if (!sd->state.menu_or_input) {
		// first invocation, display npc input box
		sd->state.menu_or_input = 1;
		st->state = RERUNLINE;
		if (is_string_variable(name)) {
			clif->scriptinputstr(sd, st->oid);
		} else {
			sd->npc_amount_min = min;
			sd->npc_amount_max = max;
			clif->scriptinput(sd, st->oid);
		}
	} else {
		// take received text/value and store it in the designated variable
		sd->state.menu_or_input = 0;
		if (is_string_variable(name)) {
			int len = (int)strlen(sd->npc_str);
			script->set_reg(st, sd, uid, name, sd->npc_str, script_getref(st, 2));
			script_pushint(st, (len > max ? 1 : len < min ? -1 : 0));
		} else {
			int amount = sd->npc_amount;
			script->set_reg(st, sd, uid, name, (const void *)h64BPTRSIZE(cap_value(amount,min,max)), script_getref(st,2));
			script_pushint(st, sd->npc_input_capped_range);
		}
		st->state = RUN;
	}
	return true;
}

// declare the copyarray method here for future reference
static BUILDIN(copyarray);

/// Sets the value of a variable.
/// The value is converted to the type of the variable.
///
/// set(<variable>,<value>) -> <variable>
static BUILDIN(__setr)
{
	struct map_session_data *sd = NULL;
	struct script_data* data;
	//struct script_data* datavalue;
	int64 num;
	const char* name;
	char prefix;
	struct reg_db *ref;

	data = script_getdata(st,2);
	//datavalue = script_getdata(st,3);
	if (!data_isreference(data) || reference_toconstant(data)) {
		ShowError("script:set: not a variable\n");
		script->reportdata(script_getdata(st,2));
		st->state = END;
		return false;
	}

	num = reference_getuid(data);
	name = reference_getname(data);
	ref = reference_getref(data);
	prefix = *name;

	if (not_server_variable(prefix)) {
		if (ref == NULL && (sd = script->rid2sd(st)) == NULL) {
			ShowError("script:set: no player attached for player variable '%s'\n", name);
			return true;
		}
	}

#if 0
	// TODO: see de43fa0f73be01080bd11c08adbfb7c158324c81
	if (data_isreference(datavalue)) {
		// the value being referenced is a variable
		const char* namevalue = reference_getname(datavalue);

		if (!not_array_variable(*namevalue)) {
			// array variable being copied into another array variable
			if (sd == NULL && not_server_variable(*namevalue) && (sd = script->rid2sd(st)) == NULL) {
				// player must be attached in order to copy a player variable
				ShowError("script:set: no player attached for player variable '%s'\n", namevalue);
				return true;
			}

			if (is_string_variable(namevalue) != is_string_variable(name)) {
				// non-matching array value types
				ShowWarning("script:set: two array variables do not match in type.\n");
				return true;
			}

			// push the maximum number of array values to the stack
			script->push_val(st->stack, C_INT, SCRIPT_MAX_ARRAYSIZE,NULL);

			// call the copy array method directly
			return buildin_copyarray(st);
		}
	}
#endif

	if (script_hasdata(st, 4)) {
		// Optional argument used by post-increment/post-decrement constructs to return the previous value
		if (is_string_variable(name)) {
			script_pushstrcopy(st, script_getstr(st, 4));
		} else {
			script_pushint(st, script_getnum(st, 4));
		}
	} else {
		// return a copy of the variable reference
		script_pushcopy(st,2);
	}

	if (is_string_variable(name))
		script->set_reg(st, sd, num, name, script_getstr(st, 3), ref);
	else
		script->set_reg(st, sd, num, name, (const void *)h64BPTRSIZE(script_getnum(st, 3)), ref);

	return true;
}

/////////////////////////////////////////////////////////////////////
/// Array variables
///

/// Sets values of an array, from the starting index.
/// ex: setarray arr[1],1,2,3;
///
/// setarray <array variable>,<value1>{,<value2>...};
static BUILDIN(setarray)
{
	struct script_data* data;
	const char* name;
	uint32 start;
	uint32 end;
	int32 id;
	int32 i;
	struct map_session_data *sd = NULL;

	data = script_getdata(st, 2);
	if( !data_isreference(data) || reference_toconstant(data) )
	{
		ShowError("script:setarray: not a variable\n");
		script->reportdata(data);
		st->state = END;
		return false;// not a variable
	}

	id = reference_getid(data);
	start = reference_getindex(data);
	name = reference_getname(data);

	if( not_server_variable(*name) )
	{
		sd = script->rid2sd(st);
		if( sd == NULL )
			return true;// no player attached
	}

	end = start + script_lastdata(st) - 2;
	if( end > SCRIPT_MAX_ARRAYSIZE )
		end = SCRIPT_MAX_ARRAYSIZE;

	if (is_string_variable(name)) {
		// string array
		for (i = 3; start < end; ++start, ++i)
			script->set_reg(st, sd, reference_uid(id, start), name, script_getstr(st, i), reference_getref(data));
	} else {
		// int array
		for (i = 3; start < end; ++start, ++i)
			script->set_reg(st, sd, reference_uid(id, start), name, (const void *)h64BPTRSIZE(script_getnum(st, i)), reference_getref(data));
	}
	return true;
}

/// Sets count values of an array, from the starting index.
/// ex: cleararray arr[0],0,1;
///
/// cleararray <array variable>,<value>,<count>;
static BUILDIN(cleararray)
{
	struct script_data* data;
	const char* name;
	uint32 start;
	uint32 end;
	int32 id;
	const void *v = NULL;
	struct map_session_data *sd = NULL;

	data = script_getdata(st, 2);
	if( !data_isreference(data) )
	{
		ShowError("script:cleararray: not a variable\n");
		script->reportdata(data);
		st->state = END;
		return false;// not a variable
	}

	id = reference_getid(data);
	start = reference_getindex(data);
	name = reference_getname(data);

	if( not_server_variable(*name) )
	{
		sd = script->rid2sd(st);
		if( sd == NULL )
			return true;// no player attached
	}

	if (is_string_variable(name))
		v = script_getstr(st, 3);
	else
		v = (const void *)h64BPTRSIZE(script_getnum(st, 3));

	end = start + script_getnum(st, 4);
	if( end > SCRIPT_MAX_ARRAYSIZE )
		end = SCRIPT_MAX_ARRAYSIZE;

	for( ; start < end; ++start )
		script->set_reg(st, sd, reference_uid(id, start), name, v, script_getref(st,2));
	return true;
}

/// Copies data from one array to another.
/// ex: copyarray arr[0],arr[2],2;
///
/// copyarray <destination array variable>,<source array variable>,<count>;
static BUILDIN(copyarray)
{
	struct script_data* data1;
	struct script_data* data2;
	const char* name1;
	const char* name2;
	int32 idx1;
	int32 idx2;
	int32 id1;
	int32 id2;
	int32 i;
	uint32 count;
	struct map_session_data *sd = NULL;

	data1 = script_getdata(st, 2);
	data2 = script_getdata(st, 3);
	if( !data_isreference(data1) || !data_isreference(data2) )
	{
		ShowError("script:copyarray: not a variable\n");
		script->reportdata(data1);
		script->reportdata(data2);
		st->state = END;
		return false;// not a variable
	}

	id1 = reference_getid(data1);
	id2 = reference_getid(data2);
	idx1 = reference_getindex(data1);
	idx2 = reference_getindex(data2);
	name1 = reference_getname(data1);
	name2 = reference_getname(data2);

	if( is_string_variable(name1) != is_string_variable(name2) )
	{
		ShowError("script:copyarray: type mismatch\n");
		script->reportdata(data1);
		script->reportdata(data2);
		st->state = END;
		return false;// data type mismatch
	}

	if( not_server_variable(*name1) || not_server_variable(*name2) )
	{
		sd = script->rid2sd(st);
		if( sd == NULL )
			return true;// no player attached
	}

	count = script_getnum(st, 4);
	if( count > SCRIPT_MAX_ARRAYSIZE - idx1 )
		count = SCRIPT_MAX_ARRAYSIZE - idx1;
	if( count <= 0 || (idx1 == idx2 && is_same_reference(data1, data2)) )
		return true;// nothing to copy

	if( is_same_reference(data1, data2) && idx1 > idx2 ) {
		// destination might be overlapping the source - copy in reverse order
		for( i = count - 1; i >= 0; --i ) {
			const void *value = script->get_val2(st, reference_uid(id2, idx2 + i), reference_getref(data2));
			script->set_reg(st, sd, reference_uid(id1, idx1 + i), name1, value, reference_getref(data1));
			script_removetop(st, -1, 0);
		}
	} else {
		// normal copy
		for( i = 0; i < count; ++i ) {
			if( idx2 + i < SCRIPT_MAX_ARRAYSIZE ) {
				const void *value = script->get_val2(st, reference_uid(id2, idx2 + i), reference_getref(data2));
				script->set_reg(st, sd, reference_uid(id1, idx1 + i), name1, value, reference_getref(data1));
				script_removetop(st, -1, 0);
			} else {
				// out of range - assume ""/0
				const void *value;
				if (is_string_variable(name1))
					value = "";
				else
					value = (const void *)0;
				script->set_reg(st, sd, reference_uid(id1, idx1 + i), name1, value, reference_getref(data1));
			}
		}
	}
	return true;
}

/// Returns the size of the array.
/// Assumes that everything before the starting index exists.
/// ex: getarraysize(arr[3])
///
/// getarraysize(<array variable>) -> <int>
static BUILDIN(getarraysize)
{
	struct script_data* data;

	data = script_getdata(st, 2);
	if( !data_isreference(data) )
	{
		ShowError("script:getarraysize: not a variable\n");
		script->reportdata(data);
		script_pushnil(st);
		st->state = END;
		return false;// not a variable
	}

	script_pushint(st, script->array_highest_key(st,st->rid ? script->rid2sd(st) : NULL,reference_getname(data),reference_getref(data)));
	return true;
}
static int script_array_index_cmp(const void *a, const void *b)
{
	return (*(const unsigned int *)a - *(const unsigned int *)b); // FIXME: Is the unsigned difference really intended here?
}

static BUILDIN(getarrayindex)
{
	struct script_data *data = script_getdata(st, 2);

	if (!data_isreference(data) || reference_toconstant(data))
	{
		ShowError("script:getarrayindex: not a variable\n");
		script->reportdata(data);
		st->state = END;
		return false;// not a variable
	}

	script_pushint(st, reference_getindex(data));
	return true;
}

/// Deletes count or all the elements in an array, from the starting index.
/// ex: deletearray arr[4],2;
///
/// deletearray <array variable>;
/// deletearray <array variable>,<count>;
static BUILDIN(deletearray)
{
	struct script_data* data;
	const char* name;
	unsigned int start, end, i;
	int id;
	struct map_session_data *sd = NULL;
	struct script_array *sa = NULL;
	struct reg_db *src = NULL;
	void *value;

	data = script_getdata(st, 2);
	if( !data_isreference(data) )
	{
		ShowError("script:deletearray: not a variable\n");
		script->reportdata(data);
		st->state = END;
		return false;// not a variable
	}

	id = reference_getid(data);
	start = reference_getindex(data);
	name = reference_getname(data);

	if( not_server_variable(*name) )
	{
		sd = script->rid2sd(st);
		if( sd == NULL )
			return true;// no player attached
	}

	if( !(src = script->array_src(st,sd,name, reference_getref(data)) ) ) {
		ShowError("script:deletearray: not a array\n");
		script->reportdata(data);
		st->state = END;
		return false;// not a variable
	}

	script->array_ensure_zero(st,NULL,data->u.num,reference_getref(data));

	if ( !(sa = idb_get(src->arrays, id)) ) { /* non-existent array, nothing to empty */
		return true;// not a variable
	}

	end = script->array_highest_key(st,sd,name,reference_getref(data));

	if( start >= end )
		return true;// nothing to free

	if( is_string_variable(name) )
		value = (void *)"";
	else
		value = (void *)0;

	if( script_hasdata(st,3) ) {
		unsigned int count = script_getnum(st, 3);
		if( count > end - start )
			count = end - start;
		if( count <= 0 )
			return true;// nothing to free

		if( end - start < sa->size ) {
			// Better to iterate directly on the array, no speed-up from using sa
			for( ; start + count < end; ++start ) {
				// Compact and overwrite
				const void *v = script->get_val2(st, reference_uid(id, start + count), reference_getref(data));
				script->set_reg(st, sd, reference_uid(id, start), name, v, reference_getref(data));
				script_removetop(st, -1, 0);
			}
			for( ; start < end; start++ ) {
				// Clean up any leftovers that weren't overwritten
				script->set_reg(st, sd, reference_uid(id, start), name, value, reference_getref(data));
			}
		} else {
			// using sa to speed up
			unsigned int *list = NULL, size = 0;
			list = script->array_cpy_list(sa);
			size = sa->size;
			qsort(list, size, sizeof(unsigned int), script_array_index_cmp);

			ARR_FIND(0, size, i, list[i] >= start);

			for( ; i < size && list[i] < start + count; i++ ) {
				// Clear any entries between start and start+count, if they exist
				script->set_reg(st, sd, reference_uid(id, list[i]), name, value, reference_getref(data));
			}

			for( ; i < size && list[i] < end; i++ ) {
				// Move back count positions any entries between start+count to fill the gaps
				const void *v = script->get_val2(st, reference_uid(id, list[i]), reference_getref(data));
				script->set_reg(st, sd, reference_uid(id, list[i]-count), name, v, reference_getref(data));
				script_removetop(st, -1, 0);
				// Clear their originals
				script->set_reg(st, sd, reference_uid(id, list[i]), name, value, reference_getref(data));
			}
		}
	} else {
		unsigned int *list = NULL, size = 0;
		list = script->array_cpy_list(sa);
		size = sa->size;

		for(i = 0; i < size; i++) {
			if( list[i] >= start ) // Less expensive than sorting it, most likely
				script->set_reg(st, sd, reference_uid(id, list[i]), name, value, reference_getref(data));
		}
	}

	return true;
}

/// Returns a reference to the target index of the array variable.
/// Equivalent to var[index].
///
/// getelementofarray(<array variable>,<index>) -> <variable reference>
static BUILDIN(getelementofarray)
{
	struct script_data* data;
	int32 id;
	int64 i;

	data = script_getdata(st, 2);
	if( !data_isreference(data) )
	{
		ShowError("script:getelementofarray: not a variable\n");
		script->reportdata(data);
		script_pushnil(st);
		st->state = END;
		return false;// not a variable
	}

	id = reference_getid(data);

	i = script_getnum(st, 3);
	if (i < 0 || i > SCRIPT_MAX_ARRAYSIZE) {
		ShowWarning("script:getelementofarray: index out of range (%"PRId64")\n", i);
		script->reportdata(data);
		script_pushnil(st);
		st->state = END;
		return false;// out of range
	}

	script->push_val(st->stack, C_NAME, reference_uid(id, (unsigned int)i), reference_getref(data));
	return true;
}

/////////////////////////////////////////////////////////////////////
/// ...
///

/*==========================================
 *
 *------------------------------------------*/
static BUILDIN(setlook)
{
	int type,val;
	struct map_session_data *sd;

	type=script_getnum(st,2);
	val=script_getnum(st,3);

	sd = script->rid2sd(st);
	if( sd == NULL )
		return true;

	pc->changelook(sd,type,val);

	return true;
}

static BUILDIN(changelook)
{ // As setlook but only client side
	int type,val;
	struct map_session_data *sd;

	type=script_getnum(st,2);
	val=script_getnum(st,3);

	sd = script->rid2sd(st);
	if( sd == NULL )
		return true;

	clif->changelook(&sd->bl,type,val);

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
static BUILDIN(cutin)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	clif->cutin(sd,script_getstr(st,2),script_getnum(st,3));
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
static BUILDIN(viewpoint)
{
	int type,x,y,id,color;
	struct map_session_data *sd;

	type=script_getnum(st,2);
	x=script_getnum(st,3);
	y=script_getnum(st,4);
	id=script_getnum(st,5);
	color=script_getnum(st,6);

	sd = script->rid2sd(st);
	if( sd == NULL )
		return true;

	clif->viewpoint(sd,st->oid,type,x,y,id,color);

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
static BUILDIN(countitem)
{
	int count = 0;
	struct item_data* id = NULL;

	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	if( script_isstringtype(st, 2) ) {
		// item name
		id = itemdb->search_name(script_getstr(st, 2));
	} else {
		// item id
		id = itemdb->exists(script_getnum(st, 2));
	}

	if( id == NULL ) {
		ShowError("buildin_countitem: Invalid item '%s'.\n", script_getstr(st,2));  // returns string, regardless of what it was
		script_pushint(st,0);
		return false;
	}

	int nameid = id->nameid;

	for (int i = 0; i < sd->status.inventorySize; i++) {
		if (sd->status.inventory[i].nameid == nameid)
			count += sd->status.inventory[i].amount;
	}

	script_pushint(st,count);
	return true;
}

/*==========================================
 * countitem2(nameID,Identified,Refine,Attribute,Card0,Card1,Card2,Card3) [Lupus]
 * returns number of items that meet the conditions
 *------------------------------------------*/
static BUILDIN(countitem2)
{
	int nameid, iden, ref, attr, c1, c2, c3, c4;
	int count = 0;
	struct item_data* id = NULL;

	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	if( script_isstringtype(st, 2) ) {
		// item name
		id = itemdb->search_name(script_getstr(st, 2));
	} else {
		// item id
		id = itemdb->exists(script_getnum(st, 2));
	}

	if( id == NULL ) {
		ShowError("buildin_countitem2: Invalid item '%s'.\n", script_getstr(st,2));  // returns string, regardless of what it was
		script_pushint(st,0);
		return false;
	}

	nameid = id->nameid;
	iden = script_getnum(st,3);
	ref  = script_getnum(st,4);
	attr = script_getnum(st,5);
	c1 = script_getnum(st,6);
	c2 = script_getnum(st,7);
	c3 = script_getnum(st,8);
	c4 = script_getnum(st,9);

	for (int i = 0; i < sd->status.inventorySize; i++)
		if (sd->status.inventory[i].nameid > 0 && sd->inventory_data[i] != NULL &&
			sd->status.inventory[i].amount > 0 && sd->status.inventory[i].nameid == nameid &&
			sd->status.inventory[i].identify == iden && sd->status.inventory[i].refine == ref &&
			sd->status.inventory[i].attribute == attr && sd->status.inventory[i].card[0] == c1 &&
			sd->status.inventory[i].card[1] == c2 && sd->status.inventory[i].card[2] == c3 &&
			sd->status.inventory[i].card[3] == c4
			)
			count += sd->status.inventory[i].amount;

	script_pushint(st,count);
	return true;
}

/*==========================================
 * countnameditem(item ID, { <Char Name / ID> })
 * returns number of named items.
 *------------------------------------------*/
static BUILDIN(countnameditem)
{
	int count = 0;
	struct item_data* id = NULL;
	struct map_session_data *sd;

	if (script_hasdata(st, 3)) {
		if (script_isstringtype(st, 3)) {
			// Character name was given
			sd = script->nick2sd(st, script_getstr(st, 3));
		} else {
			// Character ID was given
			sd = script->charid2sd(st, script_getnum(st, 3));
		}
	} else {
		// Use RID by default if no name was provided
		sd = script->rid2sd(st);
	}

	// Player not attached
	if (sd == NULL) {
		return true;
	}

	if (script_isstringtype(st, 2)) {
		// Get item from DB via item name
		id = itemdb->search_name(script_getstr(st, 2));
	} else {
		// Get item from DB via item ID
		id = itemdb->exists(script_getnum(st, 2));
	}

	if (id == NULL) {
		ShowError("buildin_countnameditem: Invalid item '%s'.\n", script_getstr(st, 2));  // returns string, regardless of what it was
		script_pushint(st, 0);
		return false;
	}

	for (int i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].nameid > 0 &&
			sd->inventory_data[i] != NULL &&
			sd->status.inventory[i].amount > 0 &&
			sd->status.inventory[i].nameid == id->nameid &&
			sd->status.inventory[i].card[0] == CARD0_CREATE &&
			sd->status.inventory[i].card[2] == sd->status.char_id &&
			sd->status.inventory[i].card[3] == sd->status.char_id >> 16)
		{
			count += sd->status.inventory[i].amount;
		}
	}

	script_pushint(st, count);
	return true;
}

/*==========================================
 * Check if item with this amount can fit in inventory
 * Checking : weight, stack amount >32k, slots amount >(MAX_INVENTORY)
 * Return
 * 0 : fail
 * 1 : success (npc side only)
 *------------------------------------------*/
static BUILDIN(checkweight)
{
	int slots, amount2=0;
	unsigned int weight=0, i, nbargs;
	struct item_data* id = NULL;
	struct map_session_data* sd;

	if( ( sd = script->rid2sd(st) ) == NULL ) {
		return true;
	}
	nbargs = script_lastdata(st)+1;
	if(nbargs%2) {
		ShowError("buildin_checkweight: Invalid nb of args should be a multiple of 2.\n");
		script_pushint(st,0);
		return false;
	}
	slots = pc->inventoryblank(sd); //nb of empty slot

	for (i = 2; i < nbargs; i += 2) {
		int nameid, amount;
		if (script_isstringtype(st, i)) {
			// item name
			id = itemdb->search_name(script_getstr(st, i));
		} else if (script_isinttype(st, i)) {
			// item id
			id = itemdb->exists(script_getnum(st, i));
		} else {
			ShowError("buildin_checkweight: invalid type for argument '%u'.\n", i);
			script_pushint(st,0);
			return false;
		}
		if( id == NULL ) {
			ShowError("buildin_checkweight: Invalid item '%s'.\n", script_getstr(st,i));  // returns string, regardless of what it was
			script_pushint(st,0);
			return false;
		}
		nameid = id->nameid;

		amount = script_getnum(st,i+1);
		if( amount < 1 ) {
			ShowError("buildin_checkweight: Invalid amount '%d'.\n", amount);
			script_pushint(st,0);
			return false;
		}

		weight += itemdb_weight(nameid)*amount; //total weight for all chk
		if( weight + sd->weight > sd->max_weight )
		{// too heavy
			script_pushint(st,0);
			return true;
		}

		switch( pc->checkadditem(sd, nameid, amount) ) {
			case ADDITEM_EXIST:
				// item is already in inventory, but there is still space for the requested amount
				break;
			case ADDITEM_NEW:
				if( itemdb->isstackable(nameid) ) {
					// stackable
					amount2++;
					if( slots < amount2 ) {
						script_pushint(st,0);
						return true;
					}
				} else {
					// non-stackable
					amount2 += amount;
					if( slots < amount2) {
						script_pushint(st,0);
						return true;
					}
				}
				break;
			case ADDITEM_OVERAMOUNT:
				script_pushint(st,0);
				return true;
		}
	}
	script_pushint(st,1);
	return true;
}

static BUILDIN(checkweight2)
{
	//variable sub checkweight
	int i=0, amount2=0, slots=0, weight=0;
	short fail=0;

	//variable for array parsing
	struct script_data* data_it;
	struct script_data* data_nb;
	const char* name_it;
	const char* name_nb;
	int32 id_it, id_nb;
	int32 idx_it, idx_nb;
	int nb_it, nb_nb; //array size

	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	data_it = script_getdata(st, 2);
	data_nb = script_getdata(st, 3);

	if( !data_isreference(data_it) || !data_isreference(data_nb))
	{
		ShowError("script:checkweight2: parameter not a variable\n");
		script_pushint(st,0);
		return false;// not a variable
	}
	id_it = reference_getid(data_it);
	id_nb = reference_getid(data_nb);
	idx_it = reference_getindex(data_it);
	idx_nb = reference_getindex(data_nb);
	name_it = reference_getname(data_it);
	name_nb = reference_getname(data_nb);

	if(is_string_variable(name_it) || is_string_variable(name_nb)) {
		ShowError("script:checkweight2: illegal type, need int\n");
		script_pushint(st,0);
		return false;// not supported
	}
	nb_it = script->array_highest_key(st,sd,reference_getname(data_it),reference_getref(data_it));
	nb_nb = script->array_highest_key(st,sd,reference_getname(data_nb),reference_getref(data_nb));
	if(nb_it != nb_nb) {
		ShowError("Size mistmatch: nb_it=%d, nb_nb=%d\n",nb_it,nb_nb);
		fail = 1;
	}

	slots = pc->inventoryblank(sd);
	for(i=0; i<nb_it; i++) {
		int32 nameid = (int32)h64BPTRSIZE(script->get_val2(st,reference_uid(id_it,idx_it+i),reference_getref(data_it)));
		int32 amount;
		script_removetop(st, -1, 0);
		amount = (int32)h64BPTRSIZE(script->get_val2(st,reference_uid(id_nb,idx_nb+i),reference_getref(data_nb)));
		script_removetop(st, -1, 0);
		if(fail) continue; //cpntonie to depop rest

		if(itemdb->exists(nameid) == NULL ) {
			ShowError("buildin_checkweight2: Invalid item '%d'.\n", nameid);
			fail=1;
			continue;
		}
		if(amount < 0 ) {
			ShowError("buildin_checkweight2: Invalid amount '%d'.\n", amount);
			fail = 1;
			continue;
		}
		weight += itemdb_weight(nameid)*amount;
		if( weight + sd->weight > sd->max_weight ) {
			fail = 1;
			continue;
		}
		switch( pc->checkadditem(sd, nameid, amount) ) {
			case ADDITEM_EXIST:
				// item is already in inventory, but there is still space for the requested amount
				break;
			case ADDITEM_NEW:
				if( itemdb->isstackable(nameid) ) {// stackable
					amount2++;
					if( slots < amount2 )
						fail = 1;
				}
				else {// non-stackable
					amount2 += amount;
					if( slots < amount2 ) {
						fail = 1;
					}
				}
				break;
			case ADDITEM_OVERAMOUNT:
				fail = 1;
		} //end switch
	} //end loop DO NOT break it prematurly we need to depop all stack

	fail?script_pushint(st,0):script_pushint(st,1);
	return true;
}

/*==========================================
 * getitem <item id>,<amount>{,<account ID>};
 * getitem "<item name>",<amount>{,<account ID>};
 *
 * getitembound <item id>,<amount>,<type>{,<account ID>};
 * getitembound "<item id>",<amount>,<type>{,<account ID>};
 *------------------------------------------*/
static BUILDIN(getitem)
{
	int nameid,amount,get_count,i,flag = 0, offset = 0;
	struct item it;
	struct map_session_data *sd;
	struct item_data *item_data;

	if( script_isstringtype(st, 2) ) {
		// "<item name>"
		const char *name = script_getstr(st, 2);
		if( (item_data = itemdb->search_name(name)) == NULL ) {
			ShowError("buildin_%s: Nonexistant item %s requested.\n", script->getfuncname(st), name);
			return false; //No item created.
		}
		nameid=item_data->nameid;
	} else {
		// <item id>
		nameid = script_getnum(st, 2);
		//Violet Box, Blue Box, etc - random item pick
		if( nameid < 0 ) {
			nameid = -nameid;
			flag = 1;
		}
		if( nameid <= 0 || !(item_data = itemdb->exists(nameid)) ) {
			ShowError("buildin_%s: Nonexistant item %d requested.\n", script->getfuncname(st), nameid);
			return false; //No item created.
		}
	}

	// <amount>
	if( (amount=script_getnum(st,3)) <= 0)
		return true; //return if amount <=0, skip the useles iteration

	memset(&it,0,sizeof(it));
	it.nameid=nameid;

	if(!flag)
		it.identify=1;
	else
		it.identify=itemdb->isidentified2(item_data);

	if( !strcmp(script->getfuncname(st),"getitembound") ) {
		int bound = script_getnum(st,4);
		if( bound < IBT_MIN || bound > IBT_MAX ) { //Not a correct bound type
			ShowError("script_getitembound: Not a correct bound type! Type=%d\n",bound);
			return false;
		}
		if( item_data->type == IT_PETEGG || item_data->type == IT_PETARMOR ) {
			ShowError("script_getitembound: can't bind a pet egg/armor! Type=%d\n",bound);
			return false;
		}
		it.bound = (unsigned char)bound;
		offset += 1;
	}

	if (script_hasdata(st,4+offset))
		sd = script->id2sd(st, script_getnum(st,4+offset)); // <Account ID>
	else
		sd=script->rid2sd(st); // Attached player

	if (sd == NULL) // no target
		return true;

	//Check if it's stackable.
	if (!itemdb->isstackable(nameid))
		get_count = 1;
	else
		get_count = amount;

	for (i = 0; i < amount; i += get_count) {
		// if not pet egg
		if (!pet->create_egg(sd, nameid)) {
			if ((flag = pc->additem(sd, &it, get_count, LOG_TYPE_SCRIPT))) {
				clif->additem(sd, 0, 0, flag);
				if( pc->candrop(sd,&it) )
					map->addflooritem(&sd->bl, &it, get_count, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0, false);
			}
		}
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
static BUILDIN(getitem2)
{
	int nameid,amount,flag = 0, offset = 0;
	int iden,ref,attr,c1,c2,c3,c4, bound = 0;
	struct map_session_data *sd;

	if( !strcmp(script->getfuncname(st),"getitembound2") ) {
		bound = script_getnum(st,11);
		if( bound < IBT_MIN || bound > IBT_MAX ) { //Not a correct bound type
			ShowError("script_getitembound2: Not a correct bound type! Type=%d\n",bound);
			return false;
		}
		offset += 1;
	}

	if (script_hasdata(st,11+offset))
		sd = script->id2sd(st, script_getnum(st,11+offset)); // <Account ID>
	else
		sd=script->rid2sd(st); // Attached player

	if (sd == NULL) // no target
		return true;

	if( script_isstringtype(st, 2) ) {
		const char *name = script_getstr(st, 2);
		struct item_data *item_data = itemdb->search_name(name);
		if( item_data )
			nameid=item_data->nameid;
		else
			nameid=UNKNOWN_ITEM_ID;
	} else {
		nameid = script_getnum(st, 2);
	}

	amount=script_getnum(st,3);
	iden=script_getnum(st,4);
	ref=script_getnum(st,5);
	attr=script_getnum(st,6);
	c1 = script_getnum(st,7);
	c2 = script_getnum(st,8);
	c3 = script_getnum(st,9);
	c4 = script_getnum(st,10);

	if (bound && (itemdb_type(nameid) == IT_PETEGG || itemdb_type(nameid) == IT_PETARMOR)) {
		ShowError("script_getitembound2: can't bind a pet egg/armor! Type=%d\n",bound);
		return false;
	}

	if (nameid < 0) { // Invalide nameid
		nameid = -nameid;
		flag = 1;
	}

	if (nameid > 0) {
		struct item item_tmp;
		struct item_data *item_data = itemdb->exists(nameid);
		int get_count, i;
		memset(&item_tmp,0,sizeof(item_tmp));
		if (item_data == NULL)
			return false;
		if(item_data->type==IT_WEAPON || item_data->type==IT_ARMOR) {
			ref = cap_value(ref, 0, MAX_REFINE);
		}
		else if(item_data->type==IT_PETEGG) {
			iden = 1;
			ref = 0;
		}
		else {
			iden = 1;
			ref = attr = 0;
		}

		item_tmp.nameid=nameid;
		if(!flag)
			item_tmp.identify=iden;
		else if(item_data->type==IT_WEAPON || item_data->type==IT_ARMOR)
			item_tmp.identify=0;
		item_tmp.refine=ref;
		item_tmp.attribute=attr;
		item_tmp.bound=(unsigned char)bound;
		item_tmp.card[0] = c1;
		item_tmp.card[1] = c2;
		item_tmp.card[2] = c3;
		item_tmp.card[3] = c4;

		//Check if it's stackable.
		if (!itemdb->isstackable(nameid))
			get_count = 1;
		else
			get_count = amount;

		for (i = 0; i < amount; i += get_count) {
			// if not pet egg
			if (!pet->create_egg(sd, nameid)) {
				if ((flag = pc->additem(sd, &item_tmp, get_count, LOG_TYPE_SCRIPT))) {
					clif->additem(sd, 0, 0, flag);
					if( pc->candrop(sd,&item_tmp) )
						map->addflooritem(&sd->bl, &item_tmp, get_count, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0, false);
				}
			}
		}
	}

	return true;
}

/*==========================================
 * rentitem <item id>,<seconds>
 * rentitem "<item name>",<seconds>
 *------------------------------------------*/
static BUILDIN(rentitem)
{
	struct map_session_data *sd;
	struct item it;
	int seconds;
	int nameid = 0, flag;

	if( (sd = script->rid2sd(st)) == NULL )
		return true;

	if( script_isstringtype(st, 2) ) {
		const char *name = script_getstr(st, 2);
		struct item_data *itd = itemdb->search_name(name);
		if( itd == NULL )
		{
			ShowError("buildin_rentitem: Nonexistant item %s requested.\n", name);
			return false;
		}
		nameid = itd->nameid;
	} else {
		nameid = script_getnum(st, 2);
		if( nameid <= 0 || !itemdb->exists(nameid) ) {
			ShowError("buildin_rentitem: Nonexistant item %d requested.\n", nameid);
			return false;
		}
	}

	seconds = script_getnum(st,3);
	memset(&it, 0, sizeof(it));
	it.nameid = nameid;
	it.identify = 1;
	it.expire_time = (unsigned int)(time(NULL) + seconds);
	it.bound = 0;

	if( (flag = pc->additem(sd, &it, 1, LOG_TYPE_SCRIPT)) )
	{
		clif->additem(sd, 0, 0, flag);
		return false;
	}

	return true;
}

/*==========================================
 * gets an item with someone's name inscribed [Skotlex]
 * getinscribeditem item_num, character_name
 * Returned Qty is always 1, only works on equip-able
 * equipment
 *------------------------------------------*/
static BUILDIN(getnameditem)
{
	int nameid;
	struct item item_tmp;
	struct map_session_data *sd, *tsd;

	sd = script->rid2sd(st);
	if (sd == NULL) // Player not attached!
		return true;

	if( script_isstringtype(st, 2) ) {
		const char *name = script_getstr(st, 2);
		struct item_data *item_data = itemdb->search_name(name);
		if( item_data == NULL) {
			//Failed
			script_pushint(st,0);
			return true;
		}
		nameid = item_data->nameid;
	} else {
		nameid = script_getnum(st, 2);
	}

	if(!itemdb->exists(nameid)/* || itemdb->isstackable(nameid)*/) {
		//Even though named stackable items "could" be risky, they are required for certain quests.
		script_pushint(st,0);
		return true;
	}

	if (script_isstringtype(st, 3)) //Char Name
		tsd = script->nick2sd(st, script_getstr(st, 3));
	else //Char Id was given
		tsd = script->charid2sd(st, script_getnum(st, 3));

	if (tsd == NULL) {
		//Failed
		script_pushint(st,0);
		return true;
	}

	memset(&item_tmp,0,sizeof(item_tmp));
	item_tmp.nameid = nameid;
	item_tmp.amount = 1;
	item_tmp.identify = 1;
	item_tmp.card[0] = CARD0_CREATE; //we don't use 255! because for example SIGNED WEAPON shouldn't get TOP10 BS Fame bonus [Lupus]
	item_tmp.card[2] = GetWord(tsd->status.char_id, 0);
	item_tmp.card[3] = GetWord(tsd->status.char_id, 1);
	if(pc->additem(sd,&item_tmp,1,LOG_TYPE_SCRIPT)) {
		script_pushint(st,0);
		return true; //Failed to add item, we will not drop if they don't fit
	}

	script_pushint(st,1);
	return true;
}

/*==========================================
 * gets a random item ID from an item group [Skotlex]
 * groupranditem group_num
 *------------------------------------------*/
static BUILDIN(grouprandomitem)
{
	struct item_data *data;
	int nameid;

	if( script_hasdata(st, 2) )
		nameid = script_getnum(st, 2);
	else if ( script->current_item_id )
		nameid = script->current_item_id;
	else {
		ShowWarning("buildin_grouprandomitem: no item id provided and no item attached\n");
		script_pushint(st, 0);
		return true;
	}

	if( !(data = itemdb->exists(nameid)) ) {
		ShowWarning("buildin_grouprandomitem: unknown item id %d\n",nameid);
		script_pushint(st, 0);
	} else if ( !data->group ) {
		ShowWarning("buildin_grouprandomitem: item '%s' (%d) isn't a group!\n",data->name,nameid);
		script_pushint(st, 0);
	} else {
		script_pushint(st, itemdb->group_item(data->group));
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
static BUILDIN(makeitem)
{
	int nameid,amount;
	int x,y,m;
	const char *mapname;
	struct item item_tmp;

	if( script_isstringtype(st, 2) ) {
		const char *name = script_getstr(st, 2);
		struct item_data *item_data = itemdb->search_name(name);
		if (item_data)
			nameid = item_data->nameid;
		else
			nameid = UNKNOWN_ITEM_ID;
	} else {
		nameid = script_getnum(st, 2);
		if( nameid <= 0 || !itemdb->exists(nameid)) {
			ShowError("makeitem: Nonexistant item %d requested.\n", nameid);
			return false; //No item created.
		}
	}
	amount  = script_getnum(st,3);
	mapname = script_getstr(st,4);
	x       = script_getnum(st,5);
	y       = script_getnum(st,6);

	if(strcmp(mapname,"this")==0) {
		struct map_session_data *sd = script->rid2sd(st);
		if (sd == NULL)
			return true; //Failed...
		m=sd->bl.m;
	} else
		m=map->mapname2mapid(mapname);

	if( m == -1 ) {
		ShowError("makeitem: creating map on unexistent map '%s'!\n", mapname);
		return false;
	}

	memset(&item_tmp,0,sizeof(item_tmp));
	item_tmp.nameid = nameid;
	item_tmp.identify=1;

	map->addflooritem(NULL, &item_tmp, amount, m, x, y, 0, 0, 0, 0, false);

	return true;
}

/*==========================================
 * makeitem2 <item id>, <amount>, <identify>, <refine>, <attribute>, <card1>, <card2>, <card3>, <card4>, {"<map name>", <X>, <Y>, <range>};
 *------------------------------------------*/
static BUILDIN(makeitem2)
{
	struct map_session_data *sd = NULL;
	struct item_data *i_data;
	int nameid = 0, amount;
	int16 x, y, m = -1, range;
	struct item item_tmp;

	if (script_isstringtype(st, 2)) {
		const char *name = script_getstr(st, 2);
		struct item_data *item_data = itemdb->search_name(name);
		if (item_data != NULL)
			nameid = item_data->nameid;
	} else {
		nameid = script_getnum(st, 2);
	}

	i_data = itemdb->exists(nameid);
	if (i_data == NULL) {
		ShowError("makeitem2: Unknown item %d requested.\n", nameid);
		return true;
	}

	if (script_hasdata(st, 11)) {
		m = map->mapname2mapid(script_getstr(st, 11));
	} else {
		sd = script->rid2sd(st);
		if (sd == NULL)
			return true;
		m = sd->bl.m;
	}

	if (m == -1) {
		ShowError("makeitem2: Nonexistant map requested.\n");
		return true;
	}

	x = (script_hasdata(st, 12) ? script_getnum(st, 12) : 0);
	y = (script_hasdata(st, 13) ? script_getnum(st, 13) : 0);

	// pick random position on map
	if (x <= 0 || x >= map->list[m].xs || y <= 0 || y >= map->list[m].ys) {
		sd = map->id2sd(st->rid);
		if ((x < 0 || y < 0) && sd == NULL) {
			x = 0;
			y = 0;
			map->search_freecell(NULL, m, &x, &y, -1, -1, 1);
		} else {
			range = (script_hasdata(st, 14) ? cap_value(script_getnum(st, 14), 1, battle_config.area_size) : 3);
			map->search_freecell(&sd->bl, sd->bl.m, &x, &y, range, range, 0); // Locate spot next to player.
		}
	}

	// if equip or weapon or egg type only drop one.
	switch (i_data->type) {
	case IT_ARMOR:
	case IT_WEAPON:
	case IT_PETARMOR:
	case IT_PETEGG:
		amount = 1;
		break;
	default:
		amount = cap_value(script_getnum(st, 3), 1, MAX_AMOUNT);
		break;
	}

	memset(&item_tmp, 0, sizeof(item_tmp));
	item_tmp.nameid = nameid;
	item_tmp.identify = script_getnum(st, 4);
	item_tmp.refine = cap_value(script_getnum(st, 5), 0, MAX_REFINE);
	item_tmp.attribute = script_getnum(st, 6);
	item_tmp.card[0] = script_getnum(st, 7);
	item_tmp.card[1] = script_getnum(st, 8);
	item_tmp.card[2] = script_getnum(st, 9);
	item_tmp.card[3] = script_getnum(st, 10);

	map->addflooritem(NULL, &item_tmp, amount, m, x, y, 0, 0, 0, 0, false);

	return true;
}

/// Counts / deletes the current item given by idx.
/// Used by buildin_delitem_search
/// Relies on all input data being already fully valid.
static void buildin_delitem_delete(struct map_session_data *sd, int idx, int *amount, bool delete_items)
{
	int delamount;
	struct item* inv;

	nullpo_retv(sd);
	nullpo_retv(amount);
	inv = &sd->status.inventory[idx];

	delamount = ( amount[0] < inv->amount ) ? amount[0] : inv->amount;

	if( delete_items )
	{
		if( sd->inventory_data[idx]->type == IT_PETEGG && inv->card[0] == CARD0_PET )
		{// delete associated pet
			intif->delete_petdata(MakeDWord(inv->card[1], inv->card[2]));
		}
		pc->delitem(sd, idx, delamount, 0, DELITEM_NORMAL, LOG_TYPE_SCRIPT);
	}

	amount[0]-= delamount;
}

/// Searches for item(s) and checks, if there is enough of them.
/// Used by delitem and delitem2
/// Relies on all input data being already fully valid.
/// @param exact_match will also match item attributes and cards, not just name id
/// @return true when all items could be deleted, false when there were not enough items to delete
static bool buildin_delitem_search(struct map_session_data *sd, struct item *it, bool exact_match)
{
	bool delete_items = false;
	int i, amount;
	struct item* inv;

	nullpo_retr(false, sd);
	nullpo_retr(false, it);
	// prefer always non-equipped items
	it->equip = 0;

	// when searching for nameid only, prefer additionally
	if (!exact_match) {
		// non-refined items
		it->refine = 0;
		// card-less items
		memset(it->card, 0, sizeof(it->card));
	}

	for (;;) {
		int important = 0;
		amount = it->amount;

		// 1st pass -- less important items / exact match
		for (i = 0; amount && i < ARRAYLENGTH(sd->status.inventory); i++) {
			inv = &sd->status.inventory[i];

			if (!inv->nameid || !sd->inventory_data[i] || inv->nameid != it->nameid) {
				// wrong/invalid item
				continue;
			}

			if (inv->equip != it->equip || inv->refine != it->refine) {
				// not matching attributes
				important++;
				continue;
			}

			if (exact_match) {
				if (inv->identify != it->identify || inv->attribute != it->attribute || memcmp(inv->card, it->card, sizeof(inv->card))) {
					// not matching exact attributes
					continue;
				}
			} else {
				if (sd->inventory_data[i]->type == IT_PETEGG) {
					if (inv->card[0] == CARD0_PET && intif->CheckForCharServer()) {
						// pet which cannot be deleted
						continue;
					}
				} else if (memcmp(inv->card, it->card, sizeof(inv->card))) {
					// named/carded item
					important++;
					continue;
				}
			}

			// count / delete item
			script->buildin_delitem_delete(sd, i, &amount, delete_items);
		}

		// 2nd pass -- any matching item
		if (amount == 0 || important == 0) {
			// either everything was already consumed or no items were skipped
			;
		} else {
			for (i = 0; amount && i < ARRAYLENGTH(sd->status.inventory); i++) {
				inv = &sd->status.inventory[i];

				if (!inv->nameid || !sd->inventory_data[i] || inv->nameid != it->nameid) {
					// wrong/invalid item
					continue;
				}

				if (sd->inventory_data[i]->type == IT_PETEGG && inv->card[0] == CARD0_PET && intif->CheckForCharServer()) {
					// pet which cannot be deleted
					continue;
				}

				if (exact_match) {
					if (inv->refine != it->refine || inv->identify != it->identify || inv->attribute != it->attribute
					 || memcmp(inv->card, it->card, sizeof(inv->card))) {
						// not matching attributes
						continue;
					}
				}

				// count / delete item
				script->buildin_delitem_delete(sd, i, &amount, delete_items);
			}
		}

		if (amount) {
			// not enough items
			return false;
		} else if(delete_items) {
			// we are done with the work
			return true;
		} else {
			// get rid of the items now
			delete_items = true;
		}
	}
}

/// Deletes items from the target/attached player.
/// Prioritizes ordinary items.
///
/// delitem <item id>,<amount>{,<account id>}
/// delitem "<item name>",<amount>{,<account id>}
static BUILDIN(delitem)
{
	struct map_session_data *sd;
	struct item it;

	if (script_hasdata(st,4)) {
		int account_id = script_getnum(st,4);
		sd = script->id2sd(st, account_id); // <account id>
		if (sd == NULL) {
			st->state = END;
			return true;
		}
	} else {
		sd = script->rid2sd(st);// attached player
		if (sd == NULL)
			return true;
	}

	memset(&it, 0, sizeof(it));
	if (script_isstringtype(st, 2)) {
		const char* item_name = script_getstr(st, 2);
		struct item_data* id = itemdb->search_name(item_name);
		if (id == NULL) {
			ShowError("script:delitem: unknown item \"%s\".\n", item_name);
			st->state = END;
			return false;
		}
		it.nameid = id->nameid;// "<item name>"
	} else {
		it.nameid = script_getnum(st, 2);// <item id>
		if (!itemdb->exists(it.nameid)) {
			ShowError("script:delitem: unknown item \"%d\".\n", it.nameid);
			st->state = END;
			return false;
		}
	}

	it.amount=script_getnum(st,3);

	if( it.amount <= 0 )
		return true;// nothing to do

	if( script->buildin_delitem_search(sd, &it, false) )
	{// success
		return true;
	}

	ShowError("script:delitem: failed to delete %d items (AID=%d item_id=%d).\n", it.amount, sd->status.account_id, it.nameid);
	st->state = END;
	clif->scriptclose(sd, st->oid);
	return false;
}

/// Deletes items from the target/attached player.
///
/// delitem2 <item id>,<amount>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>{,<account ID>}
/// delitem2 "<Item name>",<amount>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>{,<account ID>}
static BUILDIN(delitem2)
{
	struct map_session_data *sd;
	struct item it;

	if (script_hasdata(st,11)) {
		int account_id = script_getnum(st,11);
		sd = script->id2sd(st, account_id); // <account id>
		if (sd == NULL) {
			st->state = END;
			return true;
		}
	} else {
		sd = script->rid2sd(st);// attached player
		if( sd == NULL )
			return true;
	}

	memset(&it, 0, sizeof(it));
	if (script_isstringtype(st, 2)) {
		const char* item_name = script_getstr(st, 2);
		struct item_data* id = itemdb->search_name(item_name);
		if (id == NULL) {
			ShowError("script:delitem2: unknown item \"%s\".\n", item_name);
			st->state = END;
			return false;
		}
		it.nameid = id->nameid;// "<item name>"
	} else {
		it.nameid = script_getnum(st, 2);// <item id>
		if( !itemdb->exists( it.nameid ) ) {
			ShowError("script:delitem: unknown item \"%d\".\n", it.nameid);
			st->state = END;
			return false;
		}
	}

	it.amount=script_getnum(st,3);
	it.identify=script_getnum(st,4);
	it.refine=script_getnum(st,5);
	it.attribute=script_getnum(st,6);
	it.card[0] = script_getnum(st, 7);
	it.card[1] = script_getnum(st, 8);
	it.card[2] = script_getnum(st, 9);
	it.card[3] = script_getnum(st, 10);

	if( it.amount <= 0 )
		return true;// nothing to do

	if( script->buildin_delitem_search(sd, &it, true) )
	{// success
		return true;
	}

	ShowError("script:delitem2: failed to delete %d items (AID=%d item_id=%d).\n", it.amount, sd->status.account_id, it.nameid);
	st->state = END;
	clif->scriptclose(sd, st->oid);
	return false;
}

/**
 * Deletes item at given index.
 * delitem(<index>{, <amount{, <account id>}});
 */
static BUILDIN(delitemidx)
{
	struct map_session_data *sd;

	if (script_hasdata(st, 4)) {
		if ((sd = script->id2sd(st, script_getnum(st, 4))) == NULL) {
			st->state = END;
			return true;
		}
	} else {
		if ((sd = script->rid2sd(st)) == NULL)
			return true;
	}

	int i = script_getnum(st, 2);
	if (i < 0 || i >= sd->status.inventorySize) {
		ShowError("buildin_delitemidx: Index (%d) should be from 0-%d.\n", i, sd->status.inventorySize - 1);
		st->state = END;
		return false;
	}

	if (itemdb->exists(sd->status.inventory[i].nameid) == NULL)
		ShowWarning("buildin_delitemidx: Deleting invalid Item ID (%d).\n", sd->status.inventory[i].nameid);

	int amount = 0;
	if (script_hasdata(st, 3)) {
		if ((amount = script_getnum(st, 3)) > sd->status.inventory[i].amount)
			amount = sd->status.inventory[i].amount;
	} else {
		amount = sd->status.inventory[i].amount;
	}

	if (amount > 0)
		script->buildin_delitem_delete(sd, i, &amount, true);

	return true;
}

/*==========================================
 * Enables/Disables use of items while in an NPC [Skotlex]
 *------------------------------------------*/
static BUILDIN(enableitemuse)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd != NULL)
		st->npc_item_flag = sd->npc_item_flag = 1;
	return true;
}

static BUILDIN(disableitemuse)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd != NULL)
		st->npc_item_flag = sd->npc_item_flag = 0;
	return true;
}

/*==========================================
 * return the basic stats of sd
 * chk pc->readparam for available type
 *------------------------------------------*/
static BUILDIN(readparam)
{
	int type;
	struct map_session_data *sd;
	struct script_data *data = script_getdata(st, 2);

	if (reference_toparam(data)) {
		type = reference_getparamtype(data);
	} else {
		type = script->conv_num(st, data);
	}

	if (script_hasdata(st, 3)) {
		if (script_isstringtype(st, 3)) {
			sd = script->nick2sd(st, script_getstr(st, 3));
		} else {
			sd = script->id2sd(st, script_getnum(st, 3));
		}
	} else {
		sd = script->rid2sd(st);
	}

	if (sd == NULL) {
		script_pushint(st, -1);
		return true;
	}

	script_pushint(st, pc->readparam(sd, type));
	return true;
}

static BUILDIN(setparam)
{
	int type;
	struct map_session_data *sd;
	struct script_data *data = script_getdata(st, 2);
	int val = script_getnum(st, 3);

	if (data_isreference(data) && reference_toparam(data)) {
		type = reference_getparamtype(data);
	} else {
		type = script->conv_num(st, data);
	}

	if (script_hasdata(st, 4)) {
		if (script_isstringtype(st, 4)) {
			sd = script->nick2sd(st, script_getstr(st, 4));
		} else {
			sd = script->id2sd(st, script_getnum(st, 4));
		}
	} else {
		sd = script->rid2sd(st);
	}

	if (sd == NULL) {
		script_pushint(st, 0);
		return true;
	}

	if (pc->setparam(sd, type, val) == 0) {
		script_pushint(st, 0);
		return false;
	}

	script_pushint(st, 1);
	return true;
}

/*==========================================
 * Return charid identification
 * return by @num :
 * 0 : char_id
 * 1 : party_id
 * 2 : guild_id
 * 3 : account_id
 * 4 : bg_id
 * 5 : clan_id
 *------------------------------------------*/
static BUILDIN(getcharid)
{
	int num = script_getnum(st, 2);
	struct map_session_data *sd;

	if (script_hasdata(st, 3))
		sd = map->nick2sd(script_getstr(st, 3), false);
	else
		sd = script->rid2sd(st);

	if (sd == NULL) {
		script_pushint(st, 0); //return 0, according docs
		return true;
	}

	switch (num) {
	case 0:
		script_pushint(st, sd->status.char_id);
		break;
	case 1:
		script_pushint(st, sd->status.party_id);
		break;
	case 2:
		script_pushint(st, sd->status.guild_id);
		break;
	case 3:
		script_pushint(st, sd->status.account_id);
		break;
	case 4:
		script_pushint(st, sd->bg_id);
		break;
	case 5:
		script_pushint(st, sd->status.clan_id);
		break;
	default:
		ShowError("buildin_getcharid: invalid parameter (%d).\n", num);
		script_pushint(st, 0);
		break;
	}

	return true;
}

/*==========================================
 * returns the GID of an NPC
 *------------------------------------------*/
static BUILDIN(getnpcid)
{
	if (script_hasdata(st, 2)) {
		if (script_isinttype(st, 2)) {
			// Deprecate old form - getnpcid(<type>{, <"npc name">})
			ShowWarning("buildin_getnpcid: Use of type is deprecated. Format - getnpcid({<\"npc name\">})\n");
			script_pushint(st, 0);
		} else {
			struct npc_data *nd = npc->name2id(script_getstr(st, 2));
			script_pushint(st, (nd != NULL) ? nd->bl.id : 0);
		}
	} else {
		script_pushint(st, st->oid);
	}

	return true;
}

/*==========================================
 * Return the name of the party_id
 * null if not found
 *------------------------------------------*/
static BUILDIN(getpartyname)
{
	int party_id;
	struct party_data* p;

	party_id = script_getnum(st,2);

	if( ( p = party->search(party_id) ) != NULL )
	{
		script_pushstrcopy(st,p->party.name);
	}
	else
	{
		script_pushconststr(st,"null");
	}
	return true;
}

/*==========================================
 * Get the information of the members of a party by type
 * @party_id, @type
 * return by @type :
 * - : nom des membres
 * 1 : char_id des membres
 * 2 : account_id des membres
 *------------------------------------------*/
static BUILDIN(getpartymember)
{
	struct party_data *p;
	int j=0,type=0;

	p=party->search(script_getnum(st,2));

	if (script_hasdata(st,3))
		type=script_getnum(st,3);

	if ( p != NULL) {
		int i;
		for (i = 0; i < MAX_PARTY; i++) {
			if(p->party.member[i].account_id) {
				switch (type) {
					case 2:
						mapreg->setreg(reference_uid(script->add_variable("$@partymemberaid"), j),p->party.member[i].account_id);
						break;
					case 1:
						mapreg->setreg(reference_uid(script->add_variable("$@partymembercid"), j),p->party.member[i].char_id);
						break;
					default:
						mapreg->setregstr(reference_uid(script->add_variable("$@partymembername$"), j),p->party.member[i].name);
				}
				j++;
			}
		}
	}
	mapreg->setreg(script->add_variable("$@partymembercount"),j);

	return true;
}

/*==========================================
 * Retrieves party leader. if flag is specified,
 * return some of the leader data. Otherwise, return name.
 *------------------------------------------*/
static BUILDIN(getpartyleader)
{
	int party_id, type = 0, i=0;
	struct party_data *p;

	party_id=script_getnum(st,2);
	if( script_hasdata(st,3) )
		type=script_getnum(st,3);

	p=party->search(party_id);

	if (p) //Search leader
		for(i = 0; i < MAX_PARTY && !p->party.member[i].leader; i++);

	if (!p || i == MAX_PARTY) { //leader not found
		if (type)
			script_pushint(st,-1);
		else
			script_pushconststr(st,"null");
		return true;
	}

	switch (type) {
		case 1: script_pushint(st,p->party.member[i].account_id); break;
		case 2: script_pushint(st,p->party.member[i].char_id); break;
		case 3: script_pushint(st,p->party.member[i].class); break;
		case 4: script_pushstrcopy(st,mapindex_id2name(p->party.member[i].map)); break;
		case 5: script_pushint(st,p->party.member[i].lv); break;
		default: script_pushstrcopy(st,p->party.member[i].name); break;
	}
	return true;
}

enum guildinfo_type {
	GUILDINFO_NAME,
	GUILDINFO_ID,
	GUILDINFO_LEVEL,
	GUILDINFO_ONLINE,
	GUILDINFO_AV_LEVEL,
	GUILDINFO_MAX_MEMBERS,
	GUILDINFO_EXP,
	GUILDINFO_NEXT_EXP,
	GUILDINFO_SKILL_POINTS,
	GUILDINFO_MASTER_NAME,
	GUILDINFO_MASTER_CID,
};

static BUILDIN(getguildinfo)
{
	struct guild *g = NULL;

	if (script_hasdata(st, 3)) {
		if (script_isstringtype(st, 3)) {
			const char *guild_name = script_getstr(st, 3);
			g = guild->searchname(guild_name);
		} else {
			int guild_id = script_getnum(st, 3);
			g = guild->search(guild_id);
		}
	} else {
		struct map_session_data *sd = script->rid2sd(st);
		g = sd ? sd->guild : NULL;
	}

	enum guildinfo_type type = script_getnum(st, 2);

	if (g == NULL) {
		// guild does not exist
		switch (type) {
		case GUILDINFO_NAME:
		case GUILDINFO_MASTER_NAME:
			script_pushconststr(st, "");
			break;
		default:
			script_pushint(st, -1);
		}
	} else {
		switch (type) {
		case GUILDINFO_NAME:
			script_pushstrcopy(st, g->name);
			break;
		case GUILDINFO_ID:
			script_pushint(st, g->guild_id);
			break;
		case GUILDINFO_LEVEL:
			script_pushint(st, g->guild_lv);
			break;
		case GUILDINFO_ONLINE:
			script_pushint(st, g->connect_member);
			break;
		case GUILDINFO_AV_LEVEL:
			script_pushint(st, g->average_lv);
			break;
		case GUILDINFO_MAX_MEMBERS:
			script_pushint(st, g->max_member);
			break;
		case GUILDINFO_EXP:
			script_pushint(st, g->exp);
			break;
		case GUILDINFO_NEXT_EXP:
			script_pushint(st, g->next_exp);
			break;
		case GUILDINFO_SKILL_POINTS:
			script_pushint(st, g->skill_point);
			break;
		case GUILDINFO_MASTER_NAME:
			script_pushstrcopy(st, g->member[0].name);
			break;
		case GUILDINFO_MASTER_CID:
			script_pushint(st, g->member[0].char_id);
			break;
		default:
			ShowError("script:getguildinfo: unknown info type!\n");
			st->state = END;
			return false;
		}
	}
	return true;
}

/*==========================================
 * Return the name of the @guild_id
 * null if not found
 *------------------------------------------*/
static BUILDIN(getguildname)
{
	int guild_id;
	struct guild* g;

	guild_id = script_getnum(st,2);

	if( ( g = guild->search(guild_id) ) != NULL )
	{
		script_pushstrcopy(st,g->name);
	}
	else
	{
		script_pushconststr(st,"null");
	}
	return true;
}

/*==========================================
 * Return the name of the guild master of @guild_id
 * null if not found
 *------------------------------------------*/
static BUILDIN(getguildmaster)
{
	int guild_id;
	struct guild* g;

	guild_id = script_getnum(st,2);

	if( ( g = guild->search(guild_id) ) != NULL )
	{
		script_pushstrcopy(st,g->member[0].name);
	}
	else
	{
		script_pushconststr(st,"null");
	}
	return true;
}

static BUILDIN(getguildmasterid)
{
	int guild_id;
	struct guild* g;

	guild_id = script_getnum(st,2);

	if( ( g = guild->search(guild_id) ) != NULL )
	{
		script_pushint(st,g->member[0].char_id);
	}
	else
	{
		script_pushint(st,0);
	}
	return true;
}

/*==========================================
 * Get the information of the members of a guild by type.
 * getguildmember <guild_id>{,<type>};
 * @param guild_id: ID of guild
 * @param type:
 * 0 : name (default)
 * 1 : character ID
 * 2 : account ID
 *------------------------------------------*/
static BUILDIN(getguildmember)
{
	struct guild *g = NULL;
	int j = 0;

	g = guild->search(script_getnum(st,2));

	if (g) {
		int i, type = 0;

		if (script_hasdata(st,3))
			type = script_getnum(st,3);

		for ( i = 0; i < MAX_GUILD; i++ ) {
			if ( g->member[i].account_id ) {
				switch (type) {
				case 2:
					mapreg->setreg(reference_uid(script->add_variable("$@guildmemberaid"), j),g->member[i].account_id);
					break;
				case 1:
					mapreg->setreg(reference_uid(script->add_variable("$@guildmembercid"), j), g->member[i].char_id);
					break;
				default:
					mapreg->setregstr(reference_uid(script->add_variable("$@guildmembername$"), j), g->member[i].name);
					break;
				}
				j++;
			}
		}
	}
	mapreg->setreg(script->add_variable("$@guildmembercount"), j);
	return true;
}

/**
 * getguildonline(<Guild ID>{, type})
 * Returns amount of guild members online.
**/

enum script_getguildonline_types {
	GUILD_ONLINE_ALL = 0,
	GUILD_ONLINE_VENDOR,
	GUILD_ONLINE_NO_VENDOR
};

BUILDIN(getguildonline)
{
	struct guild *g;
	int guild_id = script_getnum(st, 2);
	int type = GUILD_ONLINE_ALL, j = 0;

	if ((g = guild->search(guild_id)) == NULL) {
		script_pushint(st, -1);
		return true;
	}

	if (script_hasdata(st, 3)) {
		type = script_getnum(st, 3);

		if (type < GUILD_ONLINE_ALL || type > GUILD_ONLINE_NO_VENDOR) {
			ShowWarning("buildin_getguildonline: Invalid type specified. Defaulting to GUILD_ONLINE_ALL.\n");
			type = GUILD_ONLINE_ALL;
		}
	}

	struct map_session_data *sd;
	for (int i = 0; i < MAX_GUILD; i++) {
		if (g->member[i].online && (sd = g->member[i].sd) != NULL) {
			switch (type) {
			case GUILD_ONLINE_VENDOR:
				if (sd->state.vending > 0)
					j++;
				break;

			case GUILD_ONLINE_NO_VENDOR:
				if (sd->state.vending == 0)
					j++;
				break;

			default:
				j++;
				break;
			}
		}
	}

	script_pushint(st, j);

	return true;
}

/*==========================================
 * Get char string information by type :
 * Return by @type :
 * 0 : char_name
 * 1 : party_name or ""
 * 2 : guild_name or ""
 * 3 : map_name
 * 4 : clan_name or ""
 * - : ""
 *------------------------------------------*/
static BUILDIN(strcharinfo)
{
  struct clan *c;
	struct guild* g;
	struct party_data* p;
	struct map_session_data *sd;

	if (script_hasdata(st, 4))
		sd = map->id2sd(script_getnum(st, 4));
	else
		sd = script->rid2sd(st);

	if (sd == NULL) {
		if(script_hasdata(st, 3)) {
			script_pushcopy(st, 3);
		} else {
			script_pushconststr(st, "");
		}
		return true;
	}

	switch (script_getnum(st, 2)) {
	case 0:
		script_pushstrcopy(st, sd->status.name);
		break;
	case 1:
		if ((p = party->search(sd->status.party_id)) != NULL) {
			script_pushstrcopy(st, p->party.name);
		} else {
			script_pushconststr(st, "");
		}
		break;
	case 2:
		if ((g = sd->guild) != NULL) {
			script_pushstrcopy(st, g->name);
		} else {
			script_pushconststr(st, "");
		}
		break;
	case 3:
		script_pushconststr(st, map->list[sd->bl.m].name);
		break;
	case 4:
		if ((c = sd->clan) != NULL) {
			script_pushstrcopy(st, c->name);
		} else {
			script_pushconststr(st, "");
		}
		break;
	default:
		ShowWarning("script:strcharinfo: unknown parameter.\n");
		script_pushconststr(st, "");
	}

	return true;
}

/*==========================================
 * Get npc string information by type
 * return by @type:
 * 0 : name
 * 1 : str#
 * 2 : #str
 * 3 : ::str
 * 4 : map name
 *------------------------------------------*/
static BUILDIN(strnpcinfo)
{
	char *buf,*name=NULL;
	struct npc_data *nd;

	if (script_hasdata(st, 4))
		nd = map->id2nd(script_getnum(st, 4));
	else
		nd = map->id2nd(st->oid);

	if (nd == NULL) {
		if (script_hasdata(st, 3)) {
			script_pushcopy(st, 3);
		} else {
			script_pushconststr(st, "");
		}
		return true;
	}

	switch (script_getnum(st,2)) {
	case 0: // display name
		name = aStrdup(nd->name);
		break;
	case 1: // visible part of display name
		if ((buf = strchr(nd->name,'#')) != NULL) {
			name = aStrdup(nd->name);
			name[buf - nd->name] = 0;
		} else { // Return the name, there is no '#' present
			name = aStrdup(nd->name);
		}
		break;
	case 2: // # fragment
		if ((buf = strchr(nd->name,'#')) != NULL) {
			name = aStrdup(buf+1);
		}
		break;
	case 3: // unique name
		name = aStrdup(nd->exname);
		break;
	case 4: // map name
		if (nd->bl.m >= 0) { // Only valid map indexes allowed (issue:8034)
			name = aStrdup(map->list[nd->bl.m].name);
		}
		break;
	}

	if (name)
		script_pushstr(st, name);
	else
		script_pushconststr(st, "");

	return true;
}

/**
 * charid2rid: Returns the RID associated to the given character ID
 */
static BUILDIN(charid2rid)
{
	int cid = script_getnum(st, 2);
	struct map_session_data *sd = map->charid2sd(cid);

	if (sd == NULL) {
		script_pushint(st, 0);
		return true;
	}

	script_pushint(st, sd->status.account_id);
	return true;
}

/*==========================================
 * GetEquipID(Pos);     Pos: 1-SCRIPT_EQUIP_TABLE_SIZE
 *------------------------------------------*/
static BUILDIN(getequipid)
{
	int i, num;
	struct item_data* item;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	num = script_getnum(st,2) - 1;
	if( num < 0 || num >= ARRAYLENGTH(script->equip) )
	{
		script_pushint(st,-1);
		return true;
	}

	// get inventory position of item
	i = pc->checkequip(sd,script->equip[num]);
	if( i < 0 )
	{
		script_pushint(st,-1);
		return true;
	}

	item = sd->inventory_data[i];
	if( item != 0 )
		script_pushint(st,item->nameid);
	else
		script_pushint(st,0);

	return true;
}

/*==========================================
 * Get the equipement name at pos
 * return item jname or ""
 *------------------------------------------*/
static BUILDIN(getequipname)
{
	int i, num;
	struct item_data* item;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	num = script_getnum(st,2) - 1;
	if( num < 0 || num >= ARRAYLENGTH(script->equip) )
	{
		script_pushconststr(st,"");
		return true;
	}

	// get inventory position of item
	i = pc->checkequip(sd,script->equip[num]);
	if( i < 0 )
	{
		script_pushconststr(st,"");
		return true;
	}

	item = sd->inventory_data[i];
	if( item != 0 )
		script_pushstrcopy(st,item->jname);
	else
		script_pushconststr(st,"");

	return true;
}

/*==========================================
 * getbrokenid [Valaris]
 *------------------------------------------*/
static BUILDIN(getbrokenid)
{
	int num,id=0,brokencounter=0;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	num=script_getnum(st,2);
	for (int i = 0; i < sd->status.inventorySize; i++) {
		if (sd->status.inventory[i].card[0] == CARD0_PET)
			continue;
		if ((sd->status.inventory[i].attribute & ATTR_BROKEN) != 0) {
			brokencounter++;
			if(num==brokencounter) {
				id=sd->status.inventory[i].nameid;
				break;
			}
		}
	}

	script_pushint(st,id);

	return true;
}

/*==========================================
 * getbrokencount
 *------------------------------------------*/
static BUILDIN(getbrokencount)
{
	int i, counter = 0;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	for (i = 0; i < sd->status.inventorySize; i++) {
		if (sd->status.inventory[i].card[0] == CARD0_PET)
			continue;
		if ((sd->status.inventory[i].attribute & ATTR_BROKEN) != 0)
			counter++;
	}

	script_pushint(st, counter);

	return true;
}

/*==========================================
 * repair [Valaris]
 *------------------------------------------*/
static BUILDIN(repair)
{
	int repaircounter=0;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	int num = script_getnum(st, 2);
	for(int i = 0; i < sd->status.inventorySize; i++) {
		if (sd->status.inventory[i].card[0] == CARD0_PET)
			continue;
		if ((sd->status.inventory[i].attribute & ATTR_BROKEN) != 0) {
			repaircounter++;
			if(num==repaircounter) {
				sd->status.inventory[i].attribute |= ATTR_BROKEN;
				sd->status.inventory[i].attribute ^= ATTR_BROKEN;
				clif->equipList(sd);
				clif->produce_effect(sd, 0, sd->status.inventory[i].nameid);
				clif->misceffect(&sd->bl, 3);
				break;
			}
		}
	}

	return true;
}

/*==========================================
 * repairall
 *------------------------------------------*/
static BUILDIN(repairall)
{
	int repaircounter = 0;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	for (int i = 0; i < sd->status.inventorySize; i++)
	{
		if (sd->status.inventory[i].card[0] == CARD0_PET)
			continue;
		if (sd->status.inventory[i].nameid && (sd->status.inventory[i].attribute & ATTR_BROKEN) != 0)
		{
			sd->status.inventory[i].attribute |= ATTR_BROKEN;
			sd->status.inventory[i].attribute ^= ATTR_BROKEN;
			clif->produce_effect(sd,0,sd->status.inventory[i].nameid);
			repaircounter++;
		}
	}

	if(repaircounter)
	{
		clif->misceffect(&sd->bl, 3);
		clif->equipList(sd);
	}

	return true;
}

/*==========================================
 * Chk if player have something equiped at pos
 *------------------------------------------*/
static BUILDIN(getequipisequiped)
{
	int i = -1,num;
	struct map_session_data *sd;

	num = script_getnum(st,2);
	sd = script->rid2sd(st);
	if( sd == NULL )
		return true;

	if (num > 0 && num <= ARRAYLENGTH(script->equip))
		i=pc->checkequip(sd,script->equip[num-1]);

	if(i >= 0)
		script_pushint(st,1);
	else
		script_pushint(st,0);
	return true;
}

/*==========================================
 * Chk if the player have something equiped at pos
 * if so chk if this item ain't marked not refinable or rental
 * return (npc)
 * 1 : true
 * 0 : false
 *------------------------------------------*/
static BUILDIN(getequipisenableref)
{
	int i = -1,num;
	struct map_session_data *sd;

	num = script_getnum(st,2);
	sd = script->rid2sd(st);
	if( sd == NULL )
		return true;

	if( num > 0 && num <= ARRAYLENGTH(script->equip) )
		i = pc->checkequip(sd,script->equip[num-1]);
	if( i >= 0 && sd->inventory_data[i] && !sd->inventory_data[i]->flag.no_refine && !sd->status.inventory[i].expire_time )
		script_pushint(st,1);
	else
		script_pushint(st,0);

	return true;
}

/**
 * Checks if the equipped item allows options.
 * *getequipisenableopt(<equipment_index>);
 *
 * @param equipment_index as the inventory index of the equipment.
 * @return 1 on enabled 0 on disabled.
 */
static BUILDIN(getequipisenableopt)
{
	int i = -1, index = script_getnum(st, 2);
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL) {
		script_pushint(st, -1);
		ShowError("buildin_getequipisenableopt: player is not attached!");
		return false;
	}

	if (index > 0 && index <= ARRAYLENGTH(script->equip))
		i = pc->checkequip(sd, script->equip[index - 1]);

	if (i >=0 && sd->inventory_data[i] && !sd->inventory_data[i]->flag.no_options && !sd->status.inventory[i].expire_time)
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return true;
}

/*==========================================
 * Chk if the item equiped at pos is identify (huh ?)
 * return (npc)
 * 1 : true
 * 0 : false
 *------------------------------------------*/
static BUILDIN(getequipisidentify)
{
	int i = -1,num;
	struct map_session_data *sd;

	num = script_getnum(st,2);
	sd = script->rid2sd(st);
	if( sd == NULL )
		return true;

	if (num > 0 && num <= ARRAYLENGTH(script->equip))
		i=pc->checkequip(sd,script->equip[num-1]);
	if(i >= 0)
		script_pushint(st,sd->status.inventory[i].identify);
	else
		script_pushint(st,0);

	return true;
}

/*==========================================
 * Get the total refine amount of equip at given pos
 * return total refine amount
 *------------------------------------------*/
static BUILDIN(getequiprefinerycnt)
{
	int total_refine = 0;
	struct map_session_data* sd = script->rid2sd(st);

	if (sd != NULL)
	{
		int count = script_lastdata(st);
		int script_equip_size = ARRAYLENGTH(script->equip);
		for (int n = 2; n <= count; n++) {
			int i = -1;
			int num = script_getnum(st, n);
			if (num > 0 && num <= script_equip_size)
				i = pc->checkequip(sd, script->equip[num - 1]);

			if (i >= 0)
				total_refine += sd->status.inventory[i].refine;
		}
	}
	script_pushint(st, total_refine);

	return true;
}

/*==========================================
 * Get the weapon level value at pos
 * (pos should normally only be EQI_HAND_L or EQI_HAND_R)
 * return (npc)
 * x : weapon level
 * 0 : false
 *------------------------------------------*/
static BUILDIN(getequipweaponlv)
{
	int i = -1,num;
	struct map_session_data *sd;

	num = script_getnum(st,2);
	sd = script->rid2sd(st);
	if( sd == NULL )
		return true;

	if (num > 0 && num <= ARRAYLENGTH(script->equip))
		i=pc->checkequip(sd,script->equip[num-1]);
	if(i >= 0 && sd->inventory_data[i])
		script_pushint(st,sd->inventory_data[i]->wlv);
	else
		script_pushint(st,0);

	return true;
}

/*==========================================
 * Get the item refine chance (from refine.txt) for item at pos
 * return (npc)
 * x : refine chance
 * 0 : false (max refine level or unequip..)
 *------------------------------------------*/
static BUILDIN(getequippercentrefinery)
{
	int i = -1, num;
	struct map_session_data *sd;
	int type = 0;

	num = script_getnum(st, 2);
	type = (script_hasdata(st, 3)) ? script_getnum(st, 3) : REFINE_CHANCE_TYPE_NORMAL;

	sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	if (type < REFINE_CHANCE_TYPE_NORMAL || type >= REFINE_CHANCE_TYPE_MAX) {
		ShowError("buildin_getequippercentrefinery: Invalid type (%d) provided!\n", type);
		script_pushint(st, 0);
		return false;
	}


	if (num > 0 && num <= ARRAYLENGTH(script->equip))
		i = pc->checkequip(sd, script->equip[num - 1]);

	if (i >= 0 && sd->status.inventory[i].nameid != 0 && sd->status.inventory[i].refine < MAX_REFINE)
		script_pushint(st,
			refine->get_refine_chance(itemdb_wlv(sd->status.inventory[i].nameid), (int) sd->status.inventory[i].refine, (enum refine_chance_type) type));
	else
		script_pushint(st, 0);

	return true;
}

/*==========================================
 * Refine +1 item at pos and log and display refine
 *------------------------------------------*/
static BUILDIN(successrefitem)
{
	int i = -1 , num, up = 1;
	struct map_session_data *sd;

	num = script_getnum(st,2);
	sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	if (script_hasdata(st, 3))
		up = script_getnum(st, 3);

	if (num > 0 && num <= ARRAYLENGTH(script->equip))
		i=pc->checkequip(sd,script->equip[num-1]);
	if (i >= 0) {
		int ep = sd->status.inventory[i].equip;

		//Logs items, got from (N)PC scripts [Lupus]
		logs->pick_pc(sd, LOG_TYPE_SCRIPT, -1, &sd->status.inventory[i],sd->inventory_data[i]);

		if (sd->status.inventory[i].refine >= MAX_REFINE)
			return true;

		sd->status.inventory[i].refine += up;
		sd->status.inventory[i].refine = cap_value( sd->status.inventory[i].refine, 0, MAX_REFINE);
		pc->unequipitem(sd, i, PCUNEQUIPITEM_FORCE); // status calc will happen in pc->equipitem() below

		clif->refine(sd->fd,0,i,sd->status.inventory[i].refine);
		clif->delitem(sd, i, 1, DELITEM_MATERIALCHANGE);

		//Logs items, got from (N)PC scripts [Lupus]
		logs->pick_pc(sd, LOG_TYPE_SCRIPT, 1, &sd->status.inventory[i],sd->inventory_data[i]);

		clif->additem(sd,i,1,0);
		pc->equipitem(sd,i,ep);
		clif->misceffect(&sd->bl,3);

		achievement->validate_refine(sd, i, true); // Achievements [Smokexyz/Hercules]

		/* The following check is exclusive to characters (possibly only whitesmiths)
		 * that create equipments and refine them to level 10. */
		if(sd->status.inventory[i].refine == 10 &&
		   sd->status.inventory[i].card[0] == CARD0_FORGE &&
		   sd->status.char_id == (int)MakeDWord(sd->status.inventory[i].card[2],sd->status.inventory[i].card[3])
		  ) { // Fame point system [DracoRPG]
			switch (sd->inventory_data[i]->wlv) {
			case 1:
				pc->addfame(sd, RANKTYPE_BLACKSMITH, 1); // Success to refine to +10 a lv1 weapon you forged = +1 fame point
				break;
			case 2:
				pc->addfame(sd, RANKTYPE_BLACKSMITH, 25); // Success to refine to +10 a lv2 weapon you forged = +25 fame point
				break;
			case 3:
				pc->addfame(sd, RANKTYPE_BLACKSMITH, 1000); // Success to refine to +10 a lv3 weapon you forged = +1000 fame point
				break;
			}
		}
	}

	return true;
}

/*==========================================
 * Show a failed Refine +1 attempt
 *------------------------------------------*/
static BUILDIN(failedrefitem)
{
	int i=-1,num;
	struct map_session_data *sd;

	num = script_getnum(st,2);
	sd = script->rid2sd(st);
	if( sd == NULL )
		return true;

	if (num > 0 && num <= ARRAYLENGTH(script->equip))
		i=pc->checkequip(sd,script->equip[num-1]);
	if(i >= 0) {
		// Call before changing refine to 0.
		achievement->validate_refine(sd, i, false);

		sd->status.inventory[i].refine = 0;
		pc->unequipitem(sd, i, PCUNEQUIPITEM_RECALC|PCUNEQUIPITEM_FORCE); //recalculate bonus
		clif->refine(sd->fd,1,i,sd->status.inventory[i].refine); //notify client of failure

		pc->delitem(sd, i, 1, 0, DELITEM_FAILREFINE, LOG_TYPE_SCRIPT);

		clif->misceffect(&sd->bl,2); // display failure effect
	}

	return true;
}

/*==========================================
 * Downgrades an Equipment Part by -1 . [Masao]
 *------------------------------------------*/
static BUILDIN(downrefitem)
{
	int i = -1, num, down = 1;
	struct map_session_data *sd;

	sd = script->rid2sd(st);
	if (sd == NULL)
		return true;
	num = script_getnum(st,2);
	if (script_hasdata(st, 3))
		down = script_getnum(st, 3);

	if (num > 0 && num <= ARRAYLENGTH(script->equip))
		i = pc->checkequip(sd,script->equip[num-1]);
	if (i >= 0) {
		int ep = sd->status.inventory[i].equip;

		//Logs items, got from (N)PC scripts [Lupus]
		logs->pick_pc(sd, LOG_TYPE_SCRIPT, -1, &sd->status.inventory[i],sd->inventory_data[i]);

		pc->unequipitem(sd, i, PCUNEQUIPITEM_FORCE); // status calc will happen in pc->equipitem() below
		sd->status.inventory[i].refine -= down;
		sd->status.inventory[i].refine = cap_value( sd->status.inventory[i].refine, 0, MAX_REFINE);

		clif->refine(sd->fd,2,i,sd->status.inventory[i].refine);
		clif->delitem(sd, i, 1, DELITEM_MATERIALCHANGE);

		//Logs items, got from (N)PC scripts [Lupus]
		logs->pick_pc(sd, LOG_TYPE_SCRIPT, 1, &sd->status.inventory[i],sd->inventory_data[i]);

		clif->additem(sd,i,1,0);
		pc->equipitem(sd,i,ep);

		achievement->validate_refine(sd, i, false); // Achievements [Smokexyz/Hercules]

		clif->misceffect(&sd->bl,2);
	}

	return true;
}

/*==========================================
 * Delete the item equipped at pos.
 *------------------------------------------*/
static BUILDIN(delequip)
{
	int i=-1,num;
	struct map_session_data *sd;

	num = script_getnum(st,2);
	sd = script->rid2sd(st);
	if( sd == NULL )
		return true;

	if (num > 0 && num <= ARRAYLENGTH(script->equip))
		i=pc->checkequip(sd,script->equip[num-1]);
	if(i >= 0) {
		pc->unequipitem(sd, i, PCUNEQUIPITEM_RECALC|PCUNEQUIPITEM_FORCE); //recalculate bonus
		pc->delitem(sd, i, 1, 0, DELITEM_FAILREFINE, LOG_TYPE_SCRIPT);
		return true;
	}

	ShowError("script:delequip: no item found in position '%d' for player '%s' (AID:%d/CID:%d).\n", num, sd->status.name,sd->status.account_id, sd->status.char_id);
	st->state = END;
	clif->scriptclose(sd, st->oid);

	return false;
}

/*==========================================
 *
 *------------------------------------------*/
static BUILDIN(statusup)
{
	int type;
	struct map_session_data *sd;

	type=script_getnum(st,2);
	sd = script->rid2sd(st);
	if( sd == NULL )
		return true;

	pc->statusup(sd, type, 1);

	return true;
}
/*==========================================
 *
 *------------------------------------------*/
static BUILDIN(statusup2)
{
	int type,val;
	struct map_session_data *sd;

	type=script_getnum(st,2);
	val=script_getnum(st,3);
	sd = script->rid2sd(st);
	if( sd == NULL )
		return true;

	pc->statusup2(sd,type,val);

	return true;
}


/*==========================================
* Returns the number of stat points needed to change the specified stat by val.
* needed_status_point(<type>,<val>); [secretdataz]
*------------------------------------------*/
static BUILDIN(needed_status_point)
{
	int type = script_getnum(st, 2);
	int val = script_getnum(st, 3);;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		script_pushint(st, 0);
	else
		script_pushint(st, pc->need_status_point(sd, type, val));

	return true;
}

/// See 'doc/item_bonus.txt'
///
/// bonus <bonus type>,<val1>;
/// bonus2 <bonus type>,<val1>,<val2>;
/// bonus3 <bonus type>,<val1>,<val2>,<val3>;
/// bonus4 <bonus type>,<val1>,<val2>,<val3>,<val4>;
/// bonus5 <bonus type>,<val1>,<val2>,<val3>,<val4>,<val5>;
static BUILDIN(bonus)
{
	int type;
	int val1;
	int val2 = 0;
	int val3 = 0;
	int val4 = 0;
	int val5 = 0;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true; // no player attached

	type = script_getnum(st,2);
	switch( type ) {
		case SP_AUTOSPELL:
		case SP_AUTOSPELL_WHENHIT:
		case SP_AUTOSPELL_ONSKILL:
		case SP_SKILL_ATK:
		case SP_SKILL_HEAL:
		case SP_SKILL_HEAL2:
		case SP_ADD_SKILL_BLOW:
		case SP_CASTRATE:
		case SP_ADDEFF_ONSKILL:
		case SP_SKILL_USE_SP_RATE:
		case SP_SKILL_COOLDOWN:
		case SP_SKILL_FIXEDCAST:
		case SP_SKILL_VARIABLECAST:
		case SP_VARCASTRATE:
		case SP_FIXCASTRATE:
		case SP_SKILL_USE_SP:
			// these bonuses support skill names
			if (script_isstringtype(st, 3)) {
				val1 = skill->name2id(script_getstr(st, 3));
				break;
			}
			FALLTHROUGH
		default:
			val1 = script_getnum(st,3);
			break;
	}

	switch( script_lastdata(st)-2 ) {
		case 1:
			pc->bonus(sd, type, val1);
			break;
		case 2:
			val2 = script_getnum(st,4);
			pc->bonus2(sd, type, val1, val2);
			break;
		case 3:
			val2 = script_getnum(st,4);
			val3 = script_getnum(st,5);
			pc->bonus3(sd, type, val1, val2, val3);
			break;
		case 4:
			if( type == SP_AUTOSPELL_ONSKILL && script_isstringtype(st,4) )
				val2 = skill->name2id(script_getstr(st,4)); // 2nd value can be skill name
			else
				val2 = script_getnum(st,4);

			val3 = script_getnum(st,5);
			val4 = script_getnum(st,6);
			pc->bonus4(sd, type, val1, val2, val3, val4);
			break;
		case 5:
			if( type == SP_AUTOSPELL_ONSKILL && script_isstringtype(st,4) )
				val2 = skill->name2id(script_getstr(st,4)); // 2nd value can be skill name
			else
				val2 = script_getnum(st,4);

			val3 = script_getnum(st,5);
			val4 = script_getnum(st,6);
			val5 = script_getnum(st,7);
			pc->bonus5(sd, type, val1, val2, val3, val4, val5);
			break;
		default:
			ShowDebug("buildin_bonus: unexpected number of arguments (%d)\n", (script_lastdata(st) - 1));
			break;
	}

	return true;
}

static BUILDIN(autobonus)
{
	unsigned int dur;
	short rate;
	short atk_type = 0;
	const char *bonus_script, *other_script = NULL;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true; // no player attached

	if (status->current_equip_item_index < 0 || sd->state.autobonus&sd->status.inventory[status->current_equip_item_index].equip)
		return true;

	rate = script_getnum(st,3);
	dur = script_getnum(st,4);
	bonus_script = script_getstr(st,2);
	if( !rate || !dur || !bonus_script )
		return true;

	if( script_hasdata(st,5) )
		atk_type = script_getnum(st,5);
	if( script_hasdata(st,6) )
		other_script = script_getstr(st,6);

	if( pc->addautobonus(sd->autobonus,ARRAYLENGTH(sd->autobonus),bonus_script,rate,dur,atk_type,other_script,
	                     sd->status.inventory[status->current_equip_item_index].equip,false)
	) {
		script->add_autobonus(bonus_script);
		if( other_script )
			script->add_autobonus(other_script);
	}

	return true;
}

static BUILDIN(autobonus2)
{
	unsigned int dur;
	short rate;
	short atk_type = 0;
	const char *bonus_script, *other_script = NULL;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true; // no player attached

	if (status->current_equip_item_index < 0 || sd->state.autobonus&sd->status.inventory[status->current_equip_item_index].equip)
		return true;

	rate = script_getnum(st,3);
	dur = script_getnum(st,4);
	bonus_script = script_getstr(st,2);
	if( !rate || !dur || !bonus_script )
		return true;

	if( script_hasdata(st,5) )
		atk_type = script_getnum(st,5);
	if( script_hasdata(st,6) )
		other_script = script_getstr(st,6);

	if( pc->addautobonus(sd->autobonus2,ARRAYLENGTH(sd->autobonus2),bonus_script,rate,dur,atk_type,other_script,
	                     sd->status.inventory[status->current_equip_item_index].equip,false)
	) {
		script->add_autobonus(bonus_script);
		if( other_script )
			script->add_autobonus(other_script);
	}

	return true;
}

static BUILDIN(autobonus3)
{
	unsigned int dur;
	short rate,atk_type;
	const char *bonus_script, *other_script = NULL;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true; // no player attached

	if (status->current_equip_item_index < 0 || sd->state.autobonus&sd->status.inventory[status->current_equip_item_index].equip)
		return true;

	rate = script_getnum(st,3);
	dur = script_getnum(st,4);
	atk_type = ( script_isstringtype(st,5) ? skill->name2id(script_getstr(st,5)) : script_getnum(st,5) );
	bonus_script = script_getstr(st,2);
	if( !rate || !dur || !atk_type || !bonus_script )
		return true;

	if( script_hasdata(st,6) )
		other_script = script_getstr(st,6);

	if( pc->addautobonus(sd->autobonus3,ARRAYLENGTH(sd->autobonus3),bonus_script,rate,dur,atk_type,other_script,
	                     sd->status.inventory[status->current_equip_item_index].equip,true)
	) {
		script->add_autobonus(bonus_script);
		if( other_script )
			script->add_autobonus(other_script);
	}

	return true;
}

/// Changes the level of a player skill.
/// <flag> defaults to 1
/// <flag>=0 : set the level of the skill
/// <flag>=1 : set the temporary level of the skill
/// <flag>=2 : add to the level of the skill
///
/// skill <skill id>,<level>,<flag>
/// skill <skill id>,<level>
/// skill "<skill name>",<level>,<flag>
/// skill "<skill name>",<level>
static BUILDIN(skill)
{
	int id;
	int level;
	int flag = SKILL_GRANT_TEMPORARY;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;// no player attached, report source

	id = ( script_isstringtype(st,2) ? skill->name2id(script_getstr(st,2)) : script_getnum(st,2) );
	level = script_getnum(st,3);
	if( script_hasdata(st,4) )
		flag = script_getnum(st,4);
	pc->skill(sd, id, level, flag);

	return true;
}

/// Changes the level of a player skill.
/// like skill, but <flag> defaults to 2
///
/// addtoskill <skill id>,<amount>,<flag>
/// addtoskill <skill id>,<amount>
/// addtoskill "<skill name>",<amount>,<flag>
/// addtoskill "<skill name>",<amount>
///
/// @see skill
static BUILDIN(addtoskill)
{
	int id;
	int level;
	int flag = SKILL_GRANT_TEMPSTACK;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;// no player attached, report source

	id = ( script_isstringtype(st,2) ? skill->name2id(script_getstr(st,2)) : script_getnum(st,2) );
	level = script_getnum(st,3);
	if( script_hasdata(st,4) )
		flag = script_getnum(st,4);
	pc->skill(sd, id, level, flag);

	return true;
}

/// Increases the level of a guild skill.
///
/// guildskill <skill id>,<amount>;
/// guildskill "<skill name>",<amount>;
static BUILDIN(guildskill)
{
	int skill_id, id, max_points;
	int level;

	struct guild *gd;
	struct guild_skill gd_skill;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true; // no player attached, report source

	if( (gd = sd->guild) == NULL )
		return true;

	skill_id = ( script_isstringtype(st,2) ? skill->name2id(script_getstr(st,2)) : script_getnum(st,2) );
	level = script_getnum(st,3);

	if (skill_id < GD_SKILLBASE || skill_id >= GD_MAX)
		return true;  // not guild skill

	id = skill_id - GD_SKILLBASE;
	max_points = guild->skill_get_max(skill_id);

	if( (gd->skill[id].lv + level) > max_points )
		level = max_points - gd->skill[id].lv;

	if( level <= 0 )
		return true;

	memcpy(&gd_skill, &(gd->skill[id]), sizeof(gd->skill[id]));
	gd_skill.lv += level;

	intif->guild_change_basicinfo( gd->guild_id, GBI_SKILLLV, &(gd_skill), sizeof(gd_skill) );
	return true;
}

/// Returns the level of the player skill.
///
/// getskilllv(<skill id>) -> <level>
/// getskilllv("<skill name>") -> <level>
static BUILDIN(getskilllv)
{
	int id;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;// no player attached, report source

	id = ( script_isstringtype(st,2) ? skill->name2id(script_getstr(st,2)) : script_getnum(st,2) );
	script_pushint(st, pc->checkskill(sd,id));

	return true;
}

/// Returns the level of the guild skill.
///
/// getgdskilllv(<guild id>,<skill id>) -> <level>
/// getgdskilllv(<guild id>,"<skill name>") -> <level>
static BUILDIN(getgdskilllv)
{
	int guild_id;
	uint16 skill_id;
	struct guild* g;

	guild_id = script_getnum(st,2);
	skill_id = ( script_isstringtype(st,3) ? skill->name2id(script_getstr(st,3)) : script_getnum(st,3) );
	g = guild->search(guild_id);
	if( g == NULL )
		script_pushint(st, -1);
	else
		script_pushint(st, guild->checkskill(g,skill_id));

	return true;
}

/// Returns the 'basic_skill_check' setting.
/// This config determines if the server checks the skill level of NV_BASIC
/// before allowing the basic actions.
///
/// basicskillcheck() -> <bool>
static BUILDIN(basicskillcheck)
{
	script_pushint(st, battle_config.basic_skill_check);
	return true;
}

/// Returns the GM level of the player.
///
/// getgmlevel() -> <level>
static BUILDIN(getgmlevel)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;// no player attached, report source

	script_pushint(st, pc_get_group_level(sd));

	return true;
}

/// set the group ID of the player.
/// setgroupid(<new group id>{,"<character name>"|<account id>})
/// return 1 on success, 0 if failed.
static BUILDIN(setgroupid)
{
	struct map_session_data* sd = NULL;
	int new_group = script_getnum(st, 2);

	if (script_hasdata(st, 3)) {
		if (script_isstringtype(st, 3))
			sd = script->nick2sd(st, script_getstr(st, 3));
		else
			sd = script->id2sd(st, script_getnum(st, 3));
	}
	else
		sd = script->rid2sd(st);

	if (sd == NULL)
		return true; // no player attached, report source

	script_pushint(st, !pc->set_group(sd, new_group));

	return true;
}

/// Returns the group ID of the player.
///
/// getgroupid({<account id>}) -> <int>
static BUILDIN(getgroupid)
{
	struct map_session_data *sd = NULL;

	if (script_hasdata(st, 2)) {
		sd = map->id2sd(script_getnum(st, 2));
	} else {
		sd = script->rid2sd(st);
	}

	if (sd == NULL) {
		script_pushint(st, -1);
		return true; // no player attached, report source
	}

	script_pushint(st, pc_get_group_id(sd));
	return true;
}

/// Terminates the execution of this script instance.
///
/// end
static BUILDIN(end)
{
	st->state = END;

	/* are we stopping inside a function? */
	if( st->stack->defsp >= 1 && st->stack->stack_data[st->stack->defsp-1].type == C_RETINFO ) {
		int i;
		for(i = 0; i < st->stack->sp; i++) {
			if( st->stack->stack_data[i].type == C_RETINFO ) {/* grab the first, aka the original */
				struct script_retinfo *ri = st->stack->stack_data[i].u.ri;
				st->script = ri->script;
				break;
			}
		}
	}
	return true;
}

/// Checks if the player has that effect state (option).
///
/// checkoption(<option>) -> <bool>
static BUILDIN(checkoption)
{
	int option;
	struct map_session_data *sd;

	if (script_hasdata(st, 3))
		sd = map->id2sd(script_getnum(st, 3));
	else
		sd = script->rid2sd(st);

	if (sd == NULL)
		return true;// no player attached, report source

	option = script_getnum(st,2);
	if( sd->sc.option&option )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return true;
}

/// Checks if the player is in that body state (opt1).
///
/// checkoption1(<opt1>) -> <bool>
static BUILDIN(checkoption1)
{
	int opt1;
	struct map_session_data *sd;

	if (script_hasdata(st, 3))
		sd = map->id2sd(script_getnum(st, 3));
	else
		sd = script->rid2sd(st);

	if (sd == NULL)
		return true;// no player attached, report source

	opt1 = script_getnum(st,2);
	if( sd->sc.opt1 == opt1 )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return true;
}

/// Checks if the player has that health state (opt2).
///
/// checkoption2(<opt2>) -> <bool>
static BUILDIN(checkoption2)
{
	int opt2;
	struct map_session_data *sd;

	if (script_hasdata(st, 3))
		sd = map->id2sd(script_getnum(st, 3));
	else
		sd = script->rid2sd(st);

	if (sd == NULL)
		return true;// no player attached, report source

	opt2 = script_getnum(st,2);
	if( sd->sc.opt2&opt2 )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return true;
}

/// Changes the effect state (option) of the player.
/// <flag> defaults to 1
/// <flag>=0 : removes the option
/// <flag>=other : adds the option
///
/// setoption <option>,<flag>;
/// setoption <option>;
static BUILDIN(setoption)
{
	int option;
	int flag = 1;
	struct map_session_data *sd;

	if (script_hasdata(st, 4))
		sd = map->id2sd(script_getnum(st, 4));
	else
		sd = script->rid2sd(st);

	if (sd == NULL)
		return true;// no player attached, report source

	option = script_getnum(st,2);
	if( script_hasdata(st,3) )
		flag = script_getnum(st,3);
	else if( !option ) {// Request to remove everything.
		flag = 0;
		option = OPTION_FALCON|OPTION_RIDING;
#ifndef NEW_CARTS
		option |= OPTION_CART;
#endif
	}
	if( flag ) {// Add option
		if( option&OPTION_WEDDING && !battle_config.wedding_modifydisplay )
			option &= ~OPTION_WEDDING;// Do not show the wedding sprites
		pc->setoption(sd, sd->sc.option|option);
	} else// Remove option
		pc->setoption(sd, sd->sc.option&~option);

	return true;
}

/// Returns if the player has a cart.
///
/// checkcart() -> <bool>
///
/// @author Valaris
static BUILDIN(checkcart)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;// no player attached, report source

	if( pc_iscarton(sd) )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return true;
}

/// Sets the cart of the player.
/// <type> defaults to 1
/// <type>=0 : removes the cart
/// <type>=1 : Normal cart
/// <type>=2 : Wooden cart
/// <type>=3 : Covered cart with flowers and ferns
/// <type>=4 : Wooden cart with a Panda doll on the back
/// <type>=5 : Normal cart with bigger wheels, a roof and a banner on the back
///
/// setcart <type>;
/// setcart;
static BUILDIN(setcart)
{
	int type = 1;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;// no player attached, report source

	if( script_hasdata(st,2) )
		type = script_getnum(st,2);
	pc->setcart(sd, type);

	return true;
}

/// Returns if the player has a falcon.
///
/// checkfalcon() -> <bool>
///
/// @author Valaris
static BUILDIN(checkfalcon)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;// no player attached, report source

	if (pc_isfalcon(sd))
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return true;
}

/// Sets if the player has a falcon or not.
/// <flag> defaults to 1
///
/// setfalcon <flag>;
/// setfalcon;
static BUILDIN(setfalcon)
{
	bool flag = true;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;// no player attached, report source

	if (script_hasdata(st,2))
		flag = script_getnum(st,2) ? true : false;

	pc->setfalcon(sd, flag);

	return true;
}

enum setmount_type {
	SETMOUNT_TYPE_AUTODETECT   = -1,
	SETMOUNT_TYPE_NONE         = 0,
	SETMOUNT_TYPE_PECO         = 1,
	SETMOUNT_TYPE_WUG          = 2,
	SETMOUNT_TYPE_MADO         = 3,
	SETMOUNT_TYPE_DRAGON_GREEN = 4,
	SETMOUNT_TYPE_DRAGON_BROWN = 5,
	SETMOUNT_TYPE_DRAGON_GRAY  = 6,
	SETMOUNT_TYPE_DRAGON_BLUE  = 7,
	SETMOUNT_TYPE_DRAGON_RED   = 8,
	SETMOUNT_TYPE_MAX,
	SETMOUNT_TYPE_DRAGON       = SETMOUNT_TYPE_DRAGON_GREEN,
};

/**
 * Checks if the player is riding a combat mount.
 *
 * Returns 0 if the player isn't riding, and non-zero if it is.
 * The exact returned values are the same used as flag in setmount, except for
 * dragons, where SETMOUNT_TYPE_DRAGON is returned, regardless of color.
 */
static BUILDIN(checkmount)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true; // no player attached, report source

	if (!pc_hasmount(sd)) {
		script_pushint(st, SETMOUNT_TYPE_NONE);
	} else if (pc_isridingpeco(sd)) {
		script_pushint(st, SETMOUNT_TYPE_PECO);
	} else if (pc_isridingwug(sd)) {
		script_pushint(st, SETMOUNT_TYPE_WUG);
	} else if (pc_ismadogear(sd)) {
		script_pushint(st, SETMOUNT_TYPE_MADO);
	} else { // if (pc_isridingdragon(sd))
		script_pushint(st, SETMOUNT_TYPE_DRAGON);
	}

	return true;
}

/**
 * Mounts or dismounts a combat mount.
 *
 * setmount <flag>, <mtype>;
 * setmount <flag>;
 * setmount;
 *
 * Accepted values for flag:
 * MOUNT_NONE         - dismount
 * MOUNT_PECO         - Peco Peco / Grand Peco / Gryphon (depending on the class)
 * MOUNT_WUG          - Wug (Rider)
 * MOUNT_MADO         - Mado Gear
 * MOUNT_DRAGON       - Dragon (default color)
 * MOUNT_DRAGON_GREEN - Green Dragon
 * MOUNT_DRAGON_BROWN - Brown Dragon
 * MOUNT_DRAGON_GRAY  - Gray Dragon
 * MOUNT_DRAGON_BLUE  - Blue Dragon
 * MOUNT_DRAGON_RED   - Red Dragon
 *
 * If an invalid value or no flag is specified, the appropriate mount is
 * auto-detected. As a result of this, there is no need to specify a flag at
 * all, unless it is a dragon color other than green.
 *
 * In newer clients you can specify the mado gear type though the mtype argument.
 */
static BUILDIN(setmount)
{
	int flag = SETMOUNT_TYPE_AUTODETECT;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;// no player attached, report source

	if (script_hasdata(st,2))
		flag = script_getnum(st,2);

	enum mado_type mtype = script_hasdata(st, 3) ? script_getnum(st, 3) : MADO_ROBOT;
	if (mtype < MADO_ROBOT || mtype >= MADO_MAX) {
		ShowError("script_setmount: Invalid mado type has been passed (%d).\n", flag);
		return false;
	}

	// Color variants for Rune Knight dragon mounts.
	if (flag != SETMOUNT_TYPE_NONE) {
		if (flag < SETMOUNT_TYPE_AUTODETECT || flag >= SETMOUNT_TYPE_MAX) {
			ShowWarning("script_setmount: Unknown flag %d specified. Using auto-detected value.\n", flag);
			flag = SETMOUNT_TYPE_AUTODETECT;
		}
		// Sanity checks and auto-detection
		if ((sd->job & MAPID_THIRDMASK) == MAPID_RUNE_KNIGHT) {
			if (pc->checkskill(sd, RK_DRAGONTRAINING)) {
				// Rune Knight (Dragon)
				unsigned int option;
				option = ( flag == SETMOUNT_TYPE_DRAGON_GREEN ? OPTION_DRAGON1 :
				           flag == SETMOUNT_TYPE_DRAGON_BROWN ? OPTION_DRAGON2 :
				           flag == SETMOUNT_TYPE_DRAGON_GRAY ? OPTION_DRAGON3 :
				           flag == SETMOUNT_TYPE_DRAGON_BLUE ? OPTION_DRAGON4 :
				           flag == SETMOUNT_TYPE_DRAGON_RED ? OPTION_DRAGON5 :
				           OPTION_DRAGON1); // default value
				pc->setridingdragon(sd, option);
			}
		} else if ((sd->job & MAPID_THIRDMASK) == MAPID_RANGER) {
			// Ranger (Warg)
			if (pc->checkskill(sd, RA_WUGRIDER))
				pc->setridingwug(sd, true);
		} else if ((sd->job & MAPID_THIRDMASK) == MAPID_MECHANIC) {
			// Mechanic (Mado Gear)
			if (pc->checkskill(sd, NC_MADOLICENCE))
				pc->setmadogear(sd, true, mtype);
		} else {
			// Knight / Crusader (Peco Peco)
			if (pc->checkskill(sd, KN_RIDING))
				pc->setridingpeco(sd, true);
		}
	} else if (pc_hasmount(sd)) {
		if (pc_isridingdragon(sd)) {
			pc->setridingdragon(sd, 0);
		}
		if (pc_isridingwug(sd)) {
			pc->setridingwug(sd, false);
		}
		if (pc_ismadogear(sd)) {
			pc->setmadogear(sd, false, mtype);
		}
		if (pc_isridingpeco(sd)) {
			pc->setridingpeco(sd, false);
		}
	}

	return true;
}

/// Returns if the player has a warg.
///
/// checkwug() -> <bool>
///
static BUILDIN(checkwug)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;// no player attached, report source

	if( pc_iswug(sd) || pc_isridingwug(sd) )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return true;
}

/// Sets the save point of the player.
///
/// save "<map name>",<x>,<y>
/// savepoint "<map name>",<x>,<y>
static BUILDIN(savepoint)
{
	int x;
	int y;
	short mapid;
	const char* str;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true; // no player attached, report source

	str   = script_getstr(st,2);
	x     = script_getnum(st,3);
	y     = script_getnum(st,4);
	mapid = script->mapindexname2id(st,str);
	if( mapid )
		pc->setsavepoint(sd, mapid, x, y);

	return true;
}

/*==========================================
 * GetTimeTick(0: System Tick, 1: Time Second Tick)
 *------------------------------------------*/
/* Asgard Version */
static BUILDIN(gettimetick)
{
	int type;
	time_t clock;
	struct tm *t;

	type=script_getnum(st,2);

	switch(type) {
		case 2:
			//type 2:(Get the number of seconds elapsed since 00:00 hours, Jan 1, 1970 UTC
			//        from the system clock.)
			script_pushint(st,(int)time(NULL));
			break;
		case 1:
			//type 1:(Second Ticks: 0-86399, 00:00:00-23:59:59)
			time(&clock);
			t=localtime(&clock);
			script_pushint(st,((t->tm_hour)*3600+(t->tm_min)*60+t->tm_sec));
			break;
		case 0:
		default:
			//type 0:(System Ticks)
			script_pushint(st,(int)timer->gettick()); // TODO: change this to int64 when we'll support 64 bit script values
			break;
	}
	return true;
}
/*==========================================
 * GetTime(Type);
 * 1: Sec     2: Min     3: Hour
 * 4: WeekDay     5: MonthDay     6: Month
 * 7: Year
 *------------------------------------------*/
/* Asgard Version */
static BUILDIN(gettime)
{
	int type;
	time_t clock;
	struct tm *t;

	type=script_getnum(st,2);

	time(&clock);
	t=localtime(&clock);

	switch(type) {
		case 1://Sec(0~59)
			script_pushint(st,t->tm_sec);
			break;
		case 2://Min(0~59)
			script_pushint(st,t->tm_min);
			break;
		case 3://Hour(0~23)
			script_pushint(st,t->tm_hour);
			break;
		case 4://WeekDay(0~6)
			script_pushint(st,t->tm_wday);
			break;
		case 5://MonthDay(01~31)
			script_pushint(st,t->tm_mday);
			break;
		case 6://Month(01~12)
			script_pushint(st,t->tm_mon+1);
			break;
		case 7://Year(20xx)
			script_pushint(st,t->tm_year+1900);
			break;
		case 8://Year Day(01~366)
			script_pushint(st,t->tm_yday+1);
			break;
		default://(format error)
			script_pushint(st,-1);
			break;
	}
	return true;
}

/*
 * GetTimeStr("TimeFMT", Length);
 */
static BUILDIN(gettimestr)
{
	char *tmpstr;
	const char *fmtstr;
	int maxlen;
	time_t now;

	fmtstr = script_getstr(st, 2);
	maxlen = script_getnum(st, 3);

	if (script_hasdata(st, 4)) {
		int timestamp = script_getnum(st, 4);
		if (timestamp < 0) {
			ShowWarning("buildin_gettimestr: UNIX timestamp must be in positive value.\n");
			return false;
		}

		now = (time_t)timestamp;
	} else {
		now = time(NULL);
	}

	tmpstr = (char *)aMalloc((maxlen +1)*sizeof(char));
	strftime(tmpstr, maxlen, fmtstr, localtime(&now));
	tmpstr[maxlen] = '\0';

	script_pushstr(st, tmpstr);
	return true;
}

/*==========================================
 * Open player storage
 *------------------------------------------*/
static BUILDIN(openstorage)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return false;

	if (sd->storage.received == false) {
		script_pushint(st, 0);
		ShowWarning("buildin_openstorage: Storage data for AID %d has not been loaded.\n", sd->bl.id);
		return false;
	}

	// Mapflag preventing from openstorage here
	if (!pc_has_permission(sd, PC_PERM_BYPASS_NOSTORAGE) && (map->list[sd->bl.m].flag.nostorage & 2)) {
		script_pushint(st, 0);
		return true;
	}

	storage->open(sd);

	script_pushint(st, 1); // success flag.
	return true;
}

static BUILDIN(guildopenstorage)
{
	int ret;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	// Mapflag preventing from openstorage here
	if (!pc_has_permission(sd, PC_PERM_BYPASS_NOSTORAGE) && (map->list[sd->bl.m].flag.nogstorage & 2)) {
		script_pushint(st, 1);
		return true;
	}

	ret = gstorage->open(sd);
	script_pushint(st,ret);
	return true;
}

/**
 * Makes the attached character use a skill by using an item.
 *
 * @code{.herc}
 *	itemskill(<skill id>, <skill level>{, <flag>});
 *	itemskill("<skill name>", <skill level>{, <flag>});
 * @endcode
 *
 */
static BUILDIN(itemskill)
{
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL || sd->ud.skilltimer != INVALID_TIMER)
		return true;

	pc->autocast_clear(sd);
	sd->autocast.type = AUTOCAST_ITEM;
	sd->autocast.skill_id = script_isstringtype(st, 2) ? skill->name2id(script_getstr(st, 2)) : script_getnum(st, 2);
	sd->autocast.skill_lv = script_getnum(st, 3);

	int flag = script_hasdata(st, 4) ? script_getnum(st, 4) : ISF_NONE;

	sd->autocast.itemskill_check_conditions = ((flag & ISF_CHECKCONDITIONS) == ISF_CHECKCONDITIONS);

	if (sd->autocast.itemskill_check_conditions) {
		if (skill->check_condition_castbegin(sd, sd->autocast.skill_id, sd->autocast.skill_lv) == 0
		    || skill->check_condition_castend(sd, sd->autocast.skill_id, sd->autocast.skill_lv) == 0) {
			pc->autocast_clear(sd);
			return true;
		}

		sd->autocast.itemskill_conditions_checked = true;
	}

	sd->autocast.itemskill_instant_cast = ((flag & ISF_INSTANTCAST) == ISF_INSTANTCAST);
	sd->autocast.itemskill_cast_on_self = ((flag & ISF_CASTONSELF) == ISF_CASTONSELF);

	clif->item_skill(sd, sd->autocast.skill_id, sd->autocast.skill_lv);

	return true;
}

/*==========================================
 * Attempt to create an item
 *------------------------------------------*/
static BUILDIN(produce)
{
	int trigger;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	trigger=script_getnum(st,2);
	clif->skill_produce_mix_list(sd, -1, trigger);
	return true;
}
/*==========================================
 *
 *------------------------------------------*/
static BUILDIN(cooking)
{
	int trigger;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	trigger=script_getnum(st,2);
	clif->cooking_list(sd, trigger, AM_PHARMACY, 1, 1);
	return true;
}
/*==========================================
 * Create a pet
 *------------------------------------------*/
static BUILDIN(makepet)
{
	struct map_session_data *sd;
	int id,pet_id;

	id=script_getnum(st,2);
	sd = script->rid2sd(st);
	if( sd == NULL )
		return true;

	pet_id = pet->search_petDB_index(id, PET_CLASS);

	if (pet_id < 0)
		pet_id = pet->search_petDB_index(id, PET_EGG);
	if (pet_id >= 0 && sd) {
		sd->catch_target_class = pet->db[pet_id].class_;
		intif->create_pet(sd->status.account_id, sd->status.char_id,
		                  pet->db[pet_id].class_, mob->db(pet->db[pet_id].class_)->lv,
		                  pet->db[pet_id].EggID, 0, (short)pet->db[pet_id].intimate,
		                  100, 0, 1, pet->db[pet_id].jname);
	}

	return true;
}
/*==========================================
 * Give player exp base,job * quest_exp_rate/100
 *------------------------------------------*/
static BUILDIN(getexp)
{
	int base=0,job=0;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	base = script_getnum(st,2);
	job = script_getnum(st,3);
	if (base < 0 || job < 0)
		return true;

	// bonus for npc-given exp
	base = cap_value(apply_percentrate(base, battle_config.quest_exp_rate, 100), 0, INT_MAX);
	job = cap_value(apply_percentrate(job, battle_config.quest_exp_rate, 100), 0, INT_MAX);

	pc->gainexp(sd, &sd->bl, base, job, true);

	return true;
}

/*==========================================
 * Gain guild exp [Celest]
 *------------------------------------------*/
static BUILDIN(guildgetexp)
{
	int exp;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	exp = script_getnum(st,2);
	if(exp < 0)
		return true;
	if(sd && sd->status.guild_id > 0)
		guild->getexp (sd, exp);

	return true;
}

/*==========================================
 * Changes the guild master of a guild [Skotlex]
 *------------------------------------------*/
static BUILDIN(guildchangegm)
{
	struct map_session_data *sd;
	int guild_id;
	const char *name;

	guild_id = script_getnum(st,2);
	name = script_getstr(st,3);
	sd = script->nick2sd(st, name);

	if (sd == NULL)
		script_pushint(st,0);
	else
		script_pushint(st, guild->gm_change(guild_id, sd->status.char_id));

	return true;
}

/*==========================================
 * Spawn a monster :
 * @mapn,x,y : location
 * @str : monster name
 * @class_ : mob_id
 * @amount : nb to spawn
 * @event : event to attach to mob
 *------------------------------------------*/
static BUILDIN(monster)
{
	const char *mapn  = script_getstr(st,2);
	int x             = script_getnum(st,3);
	int y             = script_getnum(st,4);
	const char *str   = script_getstr(st,5);
	int class_        = script_getnum(st,6);
	int amount        = script_getnum(st,7);
	const char *event = "";
	unsigned int size = SZ_SMALL;
	unsigned int ai   = AI_NONE;
	int mob_id;

	struct map_session_data* sd;
	int16 m;

	if (script_hasdata(st, 8))
	{
		event = script_getstr(st, 8);
		script->check_event(st, event);
	}

	if (script_hasdata(st, 9))
	{
		size = script_getnum(st, 9);
		if (size > 3)
		{
			ShowWarning("buildin_monster: Attempted to spawn non-existing size %u for monster class %d\n", size, class_);
			return false;
		}
	}

	if (script_hasdata(st, 10))
	{
		ai = script_getnum(st, 10);
		if (ai > AI_FLORA) {
			ShowWarning("buildin_monster: Attempted to spawn non-existing ai %u for monster class %d\n", ai, class_);
			return false;
		}
	}

	if (class_ >= 0 && !mob->db_checkid(class_))
	{
		ShowWarning("buildin_monster: Attempted to spawn non-existing monster class %d\n", class_);
		return false;
	}

	sd = map->id2sd(st->rid);

	if (sd && strcmp(mapn, "this") == 0)
		m = sd->bl.m;
	else {
		if ( ( m = map->mapname2mapid(mapn) ) == -1 ) {
			ShowWarning("buildin_monster: Attempted to spawn monster class %d on non-existing map '%s'\n",class_, mapn);
			return false;
		}

		if (map->list[m].flag.src4instance && st->instance_id >= 0) { // Try to redirect to the instance map, not the src map
			if ((m = instance->mapid2imapid(m, st->instance_id)) < 0) {
				ShowError("buildin_monster: Trying to spawn monster (%d) on instance map (%s) without instance attached.\n", class_, mapn);
				return false;
			}
		}
	}

	mob_id = mob->once_spawn(sd, m, x, y, str, class_, amount, event, size, ai);
	script_pushint(st, mob_id);
	return true;
}
/*==========================================
 * Request List of Monster Drops
 *------------------------------------------*/
static BUILDIN(getmobdrops)
{
	int class_ = script_getnum(st,2);
	int i, j = 0;
	struct mob_db *monster;

	if( !mob->db_checkid(class_) )
	{
		script_pushint(st, 0);
		return true;
	}

	monster = mob->db(class_);

	for( i = 0; i < MAX_MOB_DROP; i++ )
	{
		if( monster->dropitem[i].nameid < 1 )
			continue;
		if( itemdb->exists(monster->dropitem[i].nameid) == NULL )
			continue;

		mapreg->setreg(reference_uid(script->add_variable("$@MobDrop_item"), j), monster->dropitem[i].nameid);
		mapreg->setreg(reference_uid(script->add_variable("$@MobDrop_rate"), j), monster->dropitem[i].p);

		j++;
	}

	mapreg->setreg(script->add_variable("$@MobDrop_count"), j);
	script_pushint(st, 1);

	return true;
}
/*==========================================
 * Same as monster but randomize location in x0,x1,y0,y1 area
 *------------------------------------------*/
static BUILDIN(areamonster)
{
	const char *mapn  = script_getstr(st,2);
	int x0            = script_getnum(st,3);
	int y0            = script_getnum(st,4);
	int x1            = script_getnum(st,5);
	int y1            = script_getnum(st,6);
	const char *str   = script_getstr(st,7);
	int class_        = script_getnum(st,8);
	int amount        = script_getnum(st,9);
	const char *event = "";
	unsigned int size = SZ_SMALL;
	unsigned int ai   = AI_NONE;
	int mob_id;

	struct map_session_data* sd;
	int16 m;

	if (script_hasdata(st,10)) {
		event = script_getstr(st, 10);
		script->check_event(st, event);
	}

	if (script_hasdata(st, 11)) {
		size = script_getnum(st, 11);
		if (size > 3) {
			ShowWarning("buildin_monster: Attempted to spawn non-existing size %u for monster class %d\n", size, class_);
			return false;
		}
	}

	if (script_hasdata(st, 12)) {
		ai = script_getnum(st, 12);
		if (ai > AI_FLORA) {
			ShowWarning("buildin_monster: Attempted to spawn non-existing ai %u for monster class %d\n", ai, class_);
			return false;
		}
	}

	sd = map->id2sd(st->rid);

	if (sd && strcmp(mapn, "this") == 0)
		m = sd->bl.m;
	else {
		if ( ( m = map->mapname2mapid(mapn) ) == -1 ) {
			ShowWarning("buildin_areamonster: Attempted to spawn monster class %d on non-existing map '%s'\n",class_, mapn);
			return false;
		}
		if (map->list[m].flag.src4instance && st->instance_id >= 0) { // Try to redirect to the instance map, not the src map
			if ((m = instance->mapid2imapid(m, st->instance_id)) < 0) {
				ShowError("buildin_areamonster: Trying to spawn monster (%d) on instance map (%s) without instance attached.\n", class_, mapn);
				return false;
			}
		}
	}

	mob_id = mob->once_spawn_area(sd, m, x0, y0, x1, y1, str, class_, amount, event, size, ai);
	script_pushint(st, mob_id);

	return true;
}
/*==========================================
 * KillMonster subcheck, verify if mob to kill ain't got an even to handle, could be force kill by allflag
 *------------------------------------------*/
static int buildin_killmonster_sub_strip(struct block_list *bl, va_list ap)
{
	//same fix but with killmonster instead - stripping events from mobs.
	struct mob_data *md = NULL;
	char *event = va_arg(ap,char *);
	int allflag = va_arg(ap,int);

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_MOB);
	md = BL_UCAST(BL_MOB, bl);

	md->state.npc_killmonster = 1;

	if(!allflag) {
		if(strcmp(event,md->npc_event)==0)
			status_kill(bl);
	} else {
		if(!md->spawn)
			status_kill(bl);
	}
	md->state.npc_killmonster = 0;
	return 0;
}
static int buildin_killmonster_sub(struct block_list *bl, va_list ap)
{
	struct mob_data *md = NULL;
	char *event = va_arg(ap,char *);
	int allflag = va_arg(ap,int);

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_MOB);
	md = BL_UCAST(BL_MOB, bl);

	if(!allflag) {
		if(strcmp(event,md->npc_event)==0)
			status_kill(bl);
	} else {
		if(!md->spawn)
			status_kill(bl);
	}
	return 0;
}
static BUILDIN(killmonster)
{
	const char *mapname,*event;
	int16 m,allflag=0;
	mapname=script_getstr(st,2);
	event=script_getstr(st,3);

	if (strcmpi(event, "all") == 0) {
		if (strcmp(event, "all") != 0) {
			ShowWarning("buildin_killmonster: \"%s\" deprecated! Please use \"all\" instead.\n", event);
			script->reportsrc(st);
		}
		allflag = 1;
	} else {
		script->check_event(st, event);
	}

	if( (m=map->mapname2mapid(mapname))<0 )
		return true;

	if( map->list[m].flag.src4instance && st->instance_id >= 0 && (m = instance->mapid2imapid(m, st->instance_id)) < 0 )
		return true;

	if( script_hasdata(st,4) ) {
		if ( script_getnum(st,4) == 1 ) {
			map->foreachinmap(script->buildin_killmonster_sub, m, BL_MOB, event ,allflag);
			return true;
		}
	}

	map->freeblock_lock();
	map->foreachinmap(script->buildin_killmonster_sub_strip, m, BL_MOB, event ,allflag);
	map->freeblock_unlock();
	return true;
}

static int buildin_killmonsterall_sub_strip(struct block_list *bl, va_list ap)
{ //Strips the event from the mob if it's killed the old method.
	struct mob_data *md;

	md = BL_CAST(BL_MOB, bl);
	nullpo_ret(md);
	if (md->npc_event[0])
		md->npc_event[0] = 0;

	status_kill(bl);
	return 0;
}
static int buildin_killmonsterall_sub(struct block_list *bl, va_list ap)
{
	status_kill(bl);
	return 0;
}
static BUILDIN(killmonsterall)
{
	const char *mapname;
	int16 m;
	mapname=script_getstr(st,2);

	if( (m = map->mapname2mapid(mapname))<0 )
		return true;

	if( map->list[m].flag.src4instance && st->instance_id >= 0 && (m = instance->mapid2imapid(m, st->instance_id)) < 0 )
		return true;

	if( script_hasdata(st,3) ) {
		if ( script_getnum(st,3) == 1 ) {
			map->foreachinmap(script->buildin_killmonsterall_sub,m,BL_MOB);
			return true;
		}
	}

	map->foreachinmap(script->buildin_killmonsterall_sub_strip,m,BL_MOB);
	return true;
}

static BUILDIN(killmonstergid)
{
	int mobgid = script_getnum(st, 2);
	struct mob_data *md = map->id2md(mobgid);

	if (md == NULL) {
		ShowWarning("buildin_killmonstergid: Error in finding monster GID '%d' or the target is not a monster.\n", mobgid);
		return false;
	}

	md->state.npc_killmonster = 1;
	status_kill(&md->bl);
	return true;
}

/*==========================================
 * Creates a clone of a player.
 * clone map, x, y, event, char_id, master_id, mode, flag, duration
 *------------------------------------------*/
static BUILDIN(clone)
{
	struct map_session_data *sd, *msd = NULL;
	int char_id, master_id = 0, x, y, flag = 0, m;
	uint32 mode = 0;
	unsigned int duration = 0;
	const char *mapname, *event;

	mapname=script_getstr(st,2);
	x=script_getnum(st,3);
	y=script_getnum(st,4);
	event=script_getstr(st,5);
	char_id=script_getnum(st,6);

	if( script_hasdata(st,7) )
		master_id=script_getnum(st,7);

	if (script_hasdata(st,8))
		mode = script_getnum(st,8);

	if( script_hasdata(st,9) )
		flag=script_getnum(st,9);

	if( script_hasdata(st,10) )
		duration=script_getnum(st,10);

	script->check_event(st, event);

	m = map->mapname2mapid(mapname);
	if (m < 0) return true;

	sd = script->charid2sd(st, char_id);

	if (master_id) {
		msd = map->charid2sd(master_id);
		if (msd != NULL)
			master_id = msd->bl.id;
		else
			master_id = 0;
	}
	if (sd != NULL) //Return ID of newly crafted clone.
		script_pushint(st,mob->clone_spawn(sd, m, x, y, event, master_id, mode, flag, 1000*duration));
	else //Failed to create clone.
		script_pushint(st,0);

	return true;
}
/*==========================================
 *------------------------------------------*/
static BUILDIN(doevent)
{
	const char* event = script_getstr(st,2);
	struct map_session_data* sd;

	if( ( sd = script->rid2sd(st) ) == NULL )
	{
		return true;
	}

	script->check_event(st, event);
	npc->event(sd, event, 0);
	return true;
}
/*==========================================
 *------------------------------------------*/
static BUILDIN(donpcevent)
{
	const char* event = script_getstr(st,2);
	script->check_event(st, event);
	if( !npc->event_do(event) ) {
		struct npc_data * nd = map->id2nd(st->oid);
		ShowDebug("NPCEvent '%s' not found! (source: %s)\n",event,nd?nd->name:"Unknown");
		script_pushint(st, 0);
	} else
		script_pushint(st, 1);
	return true;
}

/*==========================================
 *------------------------------------------*/
static BUILDIN(addtimer)
{
	int tick = script_getnum(st, 2);
	const char* event = script_getstr(st, 3);
	struct map_session_data *sd;

	script->check_event(st, event);

	if (script_hasdata(st, 4))
		sd = map->id2sd(script_getnum(st, 4));
	else
		sd = script->rid2sd(st);

	if (sd == NULL) {
		script_pushint(st, 0);
		return false;
	}

	if (!pc->addeventtimer(sd, tick, event)) {
		ShowWarning("script:addtimer: Event timer is full, can't add new event timer. (cid:%d timer:%s)\n", sd->status.char_id, event);
		script_pushint(st, 0);
		return false;
	}

	script_pushint(st, 1);
	return true;
}
/*==========================================
 *------------------------------------------*/
static BUILDIN(deltimer)
{
	const char *event;
	struct map_session_data *sd;

	event=script_getstr(st, 2);

	if (script_hasdata(st, 3))
		sd = map->id2sd(script_getnum(st, 3));
	else
		sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	script->check_event(st, event);
	pc->deleventtimer(sd, event);
	return true;
}
/*==========================================
 *------------------------------------------*/
static BUILDIN(addtimercount)
{
	const char *event;
	int tick;
	struct map_session_data *sd;

	event = script_getstr(st, 2);
	tick = script_getnum(st, 3);

	if (script_hasdata(st, 4))
		sd = map->id2sd(script_getnum(st, 4));
	else
		sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	script->check_event(st, event);
	pc->addeventtimercount(sd, event, tick);
	return true;
}

enum gettimer_mode {
	GETTIMER_COUNT = 0,
	GETTIMER_TICK_NEXT = 1,
	GETTIMER_TICK_LAST = 2,
};

static BUILDIN(gettimer)
{
	struct map_session_data *sd;
	const struct TimerData *td;
	int i;
	int tick;
	const char *event = NULL;
	int val = 0;
	bool first = true;
	short mode = script_getnum(st, 2);

	if (script_hasdata(st, 3))
		sd = map->id2sd(script_getnum(st, 3));
	else
		sd = script->rid2sd(st);

	if (script_hasdata(st, 4)) {
		event = script_getstr(st, 4);
		script->check_event(st, event);
	}

	if (sd == NULL) {
		script_pushint(st, -1);
		return true;
	}

	switch (mode) {
	case GETTIMER_COUNT:
		// get number of timers
		for (i = 0; i < MAX_EVENTTIMER; i++) {
			if (sd->eventtimer[i] != INVALID_TIMER) {
				if (event != NULL) {
					td = timer->get(sd->eventtimer[i]);
					Assert_retr(false, td != NULL);

					if (strcmp((char *)(td->data), event) == 0) {
						val++;
					}
				} else {
					val++;
				}
			}
		}
		break;
	case GETTIMER_TICK_NEXT:
		// get the number of tick before the next timer runs
		for (i = 0; i < MAX_EVENTTIMER; i++) {
			if (sd->eventtimer[i] != INVALID_TIMER) {
				td = timer->get(sd->eventtimer[i]);
				Assert_retr(false, td != NULL);
				tick = max(0, DIFF_TICK32(td->tick, timer->gettick()));

				if (event != NULL) {
					if ((first == true || tick < val) && strcmp((char *)(td->data), event) == 0) {
						val = tick;
						first = false;
					}
				} else if (first == true || tick < val) {
					val = tick;
					first = false;
				}
			}
		}
		break;
	case GETTIMER_TICK_LAST:
		// get the number of ticks before the last timer runs
		for (i = MAX_EVENTTIMER - 1; i >= 0; i--) {
			if (sd->eventtimer[i] != INVALID_TIMER) {
				td = timer->get(sd->eventtimer[i]);
				Assert_retr(false, td != NULL);
				tick = max(0, DIFF_TICK32(td->tick, timer->gettick()));

				if (event != NULL) {
					if (strcmp((char *)(td->data), event) == 0) {
						val = max(val, tick);
					}
				} else {
					val = max(val, tick);
				}
			}
		}
		break;
	}

	script_pushint(st, val);
	return true;
}

static int buildin_getunits_sub(struct block_list *bl, va_list ap)
{
	struct script_state *st = va_arg(ap, struct script_state *);
	struct map_session_data *sd = va_arg(ap, struct map_session_data *);
	int32 id = va_arg(ap, int32);
	uint32 start = va_arg(ap, uint32);
	uint32 *count = va_arg(ap, uint32 *);
	uint32 limit = va_arg(ap, uint32);
	const char *name = va_arg(ap, const char *);
	struct reg_db *ref = va_arg(ap, struct reg_db *);
	enum bl_type type = va_arg(ap, enum bl_type);
	uint32 index = start + *count;

	if ((bl->type & type) == 0) {
		return 0; // type mismatch => skip
	}

	if (index >= SCRIPT_MAX_ARRAYSIZE || *count >= limit) {
		return -1;
	}

	script->set_reg(st, sd, reference_uid(id, index), name,
		(const void *)h64BPTRSIZE(bl->id), ref);

	(*count)++;
	return 1;
}

static int buildin_getunits_sub_pc(struct map_session_data *sd, va_list ap)
{
	return buildin_getunits_sub(&sd->bl, ap);
}

static int buildin_getunits_sub_mob(struct mob_data *md, va_list ap)
{
	return buildin_getunits_sub(&md->bl, ap);
}

static int buildin_getunits_sub_npc(struct npc_data *nd, va_list ap)
{
	return buildin_getunits_sub(&nd->bl, ap);
}

static BUILDIN(getunits)
{
	const char *name;
	int32 id;
	uint32 start;
	struct reg_db *ref;
	enum bl_type type = script_getnum(st, 2);
	struct script_data *data = script_getdata(st, 3);
	uint32 count = 0;
	uint32 limit = script_getnum(st, 4);
	struct map_session_data *sd = NULL;

	if (!data_isreference(data) || reference_toconstant(data)) {
		ShowError("script:getunits: second argument must be a variable\n");
		script->reportdata(data);
		st->state = END;
		return false;
	}

	id = reference_getid(data);
	start = reference_getindex(data);
	name = reference_getname(data);
	ref = reference_getref(data);

	if (not_server_variable(*name)) {
		sd = script->rid2sd(st);

		if (sd == NULL) {
			script_pushint(st, 0);
			return true; // player variable but no player attached
		}
	}

	if (is_string_variable(name)) {
		ShowError("script:getunits: second argument must be an integer variable\n");
		script->reportdata(data);
		st->state = END;
		return false;
	}

	if (limit < 1 || limit > SCRIPT_MAX_ARRAYSIZE) {
		limit = SCRIPT_MAX_ARRAYSIZE;
	}

	if (script_hasdata(st, 5)) {
		const char *mapname = script_getstr(st, 5);
		int16 m = map->mapname2mapid(mapname);

		if (m == -1) {
			ShowError("script:getunits: Invalid map(%s) provided.\n", mapname);
			script->reportdata(data);
			st->state = END;
			return false;
		}

		if (script_hasdata(st, 9)) {
			int16 x1 = script_getnum(st, 6);
			int16 y1 = script_getnum(st, 7);
			int16 x2 = script_getnum(st, 8);
			int16 y2 = script_getnum(st, 9);

			map->forcountinarea(buildin_getunits_sub, m, x1, y1, x2, y2, limit, type,
				st, sd, id, start, &count, limit, name, ref, type);
		} else {
			map->forcountinmap(buildin_getunits_sub, m, limit, type,
				st, sd, id, start, &count, limit, name, ref, type);
		}
	} else {
		// for faster lookup we try to reduce the scope of the search if possible
		switch (type) {
		case BL_PC:
			map->foreachpc(buildin_getunits_sub_pc,
				st, sd, id, start, &count, limit, name, ref, type);
			break;
		case BL_MOB:
			map->foreachmob(buildin_getunits_sub_mob,
				st, sd, id, start, &count, limit, name, ref, type);
			break;
		case BL_NPC:
			map->foreachnpc(buildin_getunits_sub_npc,
				st, sd, id, start, &count, limit, name, ref, type);
			break;
		default:
			// fallback to global lookup (slowest option)
			map->foreachiddb(buildin_getunits_sub,
				st, sd, id, start, &count, limit, name, ref, type);
		}
	}

	script_pushint(st, count);
	return true;
}

/*==========================================
 *------------------------------------------*/
static BUILDIN(initnpctimer)
{
	struct npc_data *nd;
	int flag = 0;

	if( script_hasdata(st,3) ) {
		//Two arguments: NPC name and attach flag.
		nd = npc->name2id(script_getstr(st, 2));
		flag = script_getnum(st,3);
	} else if( script_hasdata(st,2) ) {
		//Check if argument is numeric (flag) or string (npc name)
		struct script_data *data;
		data = script_getdata(st,2);
		script->get_val(st,data); // dereference if it's a variable
		if (data_isstring(data)) {
			//NPC name
			nd = npc->name2id(script->conv_str(st, data));
		} else if (data_isint(data)) {
			//Flag
			nd = map->id2nd(st->oid);
			flag = script->conv_num(st,data);
		} else {
			ShowError("initnpctimer: invalid argument type #1 (needs be int or string)).\n");
			return false;
		}
	} else {
		nd = map->id2nd(st->oid);
	}

	if( !nd )
		return true;
	if (flag) { //Attach
		struct map_session_data *sd = script->rid2sd(st);
		if (sd == NULL)
			return true;
		nd->u.scr.rid = sd->bl.id;
	}

	nd->u.scr.timertick = 0;
	npc->settimerevent_tick(nd,0);
	npc->timerevent_start(nd, st->rid);
	return true;
}
/*==========================================
 *------------------------------------------*/
static BUILDIN(startnpctimer)
{
	struct npc_data *nd;
	int flag = 0;

	if( script_hasdata(st,3) ) {
		//Two arguments: NPC name and attach flag.
		nd = npc->name2id(script_getstr(st, 2));
		flag = script_getnum(st,3);
	} else if( script_hasdata(st,2) ) {
		//Check if argument is numeric (flag) or string (npc name)
		struct script_data *data;
		data = script_getdata(st,2);
		script->get_val(st,data); // dereference if it's a variable
		if (data_isstring(data)) {
			//NPC name
			nd = npc->name2id(script->conv_str(st, data));
		} else if (data_isint(data)) {
			//Flag
			nd = map->id2nd(st->oid);
			flag = script->conv_num(st,data);
		} else {
			ShowError("initnpctimer: invalid argument type #1 (needs be int or string)).\n");
			return false;
		}
	} else {
		nd = map->id2nd(st->oid);
	}

	if( !nd )
		return true;
	if (flag) { //Attach
		struct map_session_data *sd = script->rid2sd(st);
		if (sd == NULL)
			return true;
		nd->u.scr.rid = sd->bl.id;
	}

	npc->timerevent_start(nd, st->rid);
	return true;
}
/*==========================================
 *------------------------------------------*/
static BUILDIN(stopnpctimer)
{
	struct npc_data *nd;
	int flag = 0;

	if( script_hasdata(st,3) ) {
		//Two arguments: NPC name and attach flag.
		nd = npc->name2id(script_getstr(st, 2));
		flag = script_getnum(st,3);
	} else if( script_hasdata(st,2) ) {
		//Check if argument is numeric (flag) or string (npc name)
		struct script_data *data;
		data = script_getdata(st,2);
		script->get_val(st,data); // Dereference if it's a variable
		if (data_isstring(data)) {
			//NPC name
			nd = npc->name2id(script->conv_str(st, data));
		} else if (data_isint(data)) {
			//Flag
			nd = map->id2nd(st->oid);
			flag = script->conv_num(st,data);
		} else {
			ShowError("initnpctimer: invalid argument type #1 (needs be int or string)).\n");
			return false;
		}
	} else {
		nd = map->id2nd(st->oid);
	}

	if( !nd )
		return true;
	if( flag ) //Detach
		nd->u.scr.rid = 0;

	npc->timerevent_stop(nd);
	return true;
}
/*==========================================
 *------------------------------------------*/
static BUILDIN(getnpctimer)
{
	struct npc_data *nd;
	struct map_session_data *sd;
	int type = script_getnum(st,2);
	int val = 0;

	if( script_hasdata(st,3) )
		nd = npc->name2id(script_getstr(st,3));
	else
		nd = map->id2nd(st->oid);

	if (nd == NULL) {
		script_pushint(st,0);
		ShowError("getnpctimer: Invalid NPC.\n");
		return false;
	}

	switch( type ) {
		case 0: val = (int)npc->gettimerevent_tick(nd); break; // FIXME: change this to int64 when we'll support 64 bit script values
		case 1:
			if (nd->u.scr.rid) {
				sd = script->id2sd(st, nd->u.scr.rid);
				if (sd == NULL)
					break;
				val = (sd->npc_timer_id != INVALID_TIMER);
			} else {
				val = (nd->u.scr.timerid != INVALID_TIMER);
			}
			break;
		case 2: val = nd->u.scr.timeramount; break;
	}

	script_pushint(st,val);
	return true;
}
/*==========================================
 *------------------------------------------*/
static BUILDIN(setnpctimer)
{
	int tick;
	struct npc_data *nd;

	tick = script_getnum(st,2);
	if( script_hasdata(st,3) )
		nd = npc->name2id(script_getstr(st,3));
	else
		nd = map->id2nd(st->oid);

	if (nd == NULL) {
		script_pushint(st,1);
		ShowError("setnpctimer: Invalid NPC.\n");
		return false;
	}

	npc->settimerevent_tick(nd,tick);
	script_pushint(st,0);
	return true;
}

/*==========================================
 * attaches the player rid to the timer [Celest]
 *------------------------------------------*/
static BUILDIN(attachnpctimer)
{
	struct map_session_data *sd;
	struct npc_data *nd = map->id2nd(st->oid);

	if (nd == NULL) {
		script_pushint(st,1);
		ShowError("setnpctimer: Invalid NPC.\n");
		return false;
	}

	if (script_hasdata(st,2))
		sd = script->nick2sd(st, script_getstr(st,2));
	else
		sd = script->rid2sd(st);

	if (sd == NULL) {
		script_pushint(st,1);
		return true;
	}

	nd->u.scr.rid = sd->bl.id;
	script_pushint(st,0);
	return true;
}

/*==========================================
 * detaches a player rid from the timer [Celest]
 *------------------------------------------*/
static BUILDIN(detachnpctimer)
{
	struct npc_data *nd;

	if( script_hasdata(st,2) )
		nd = npc->name2id(script_getstr(st,2));
	else
		nd = map->id2nd(st->oid);

	if (nd == NULL) {
		script_pushint(st,1);
		ShowError("detachnpctimer: Invalid NPC.\n");
		return false;
	}

	nd->u.scr.rid = 0;
	script_pushint(st,0);
	return true;
}

/*==========================================
 * To avoid "player not attached" script errors, this function is provided,
 * it checks if there is a player attached to the current script. [Skotlex]
 * If no, returns 0, if yes, returns the account_id of the attached player.
 *------------------------------------------*/
static BUILDIN(playerattached)
{
	if(st->rid == 0 || map->id2sd(st->rid) == NULL)
		script_pushint(st,0);
	else
		script_pushint(st,st->rid);
	return true;
}

/*==========================================
 * Used by OnTouchNPC: label to return monster GID
 *------------------------------------------*/
static BUILDIN(mobattached)
{
	if (st->rid == 0 || map->id2md(st->rid) == NULL)
		script_pushint(st, 0);
	else
		script_pushint(st, st->rid);
	return true;
}

/*==========================================
 *------------------------------------------*/
static BUILDIN(announce)
{
	const char *mes       = script_getstr(st,2);
	int         flag      = script_getnum(st,3);
	const char *fontColor = script_hasdata(st,4) ? script_getstr(st,4) : NULL;
	int         fontType  = script_hasdata(st,5) ? script_getnum(st,5) : 0x190; // default fontType (FW_NORMAL)
	int         fontSize  = script_hasdata(st,6) ? script_getnum(st,6) : 12;    // default fontSize
	int         fontAlign = script_hasdata(st,7) ? script_getnum(st,7) : 0;     // default fontAlign
	int         fontY     = script_hasdata(st,8) ? script_getnum(st,8) : 0;     // default fontY
	size_t len = strlen(mes);
	send_target target = ALL_CLIENT;
	struct block_list *bl = NULL;
	Assert_retr(false, len < INT_MAX);

	if( flag&(BC_TARGET_MASK|BC_SOURCE_MASK) ) {
		// Broadcast source or broadcast region defined
		if (flag&BC_NPC) {
			// If bc_npc flag is set, use NPC as broadcast source
			bl = map->id2bl(st->oid);
		} else {
			struct map_session_data *sd = script->rid2sd(st);
			if (sd != NULL)
				bl = &sd->bl;
		}
		if (bl == NULL)
			return true;

		switch( flag&BC_TARGET_MASK ) {
			case BC_MAP:  target = ALL_SAMEMAP; break;
			case BC_AREA: target = AREA;        break;
			case BC_SELF: target = SELF;        break;
			default:      target = ALL_CLIENT;  break; // BC_ALL
		}
	}

	if (fontColor)
		clif->broadcast2(bl, mes, (int)len+1, (unsigned int)strtoul(fontColor, (char **)NULL, 0), fontType, fontSize, fontAlign, fontY, target);
	else
		clif->broadcast(bl, mes, (int)len+1, flag&BC_COLOR_MASK, target);

	return true;
}
/*==========================================
 *------------------------------------------*/
static int buildin_announce_sub(struct block_list *bl, va_list ap)
{
	const char *mes = va_arg(ap, const char *);
	int   len       = va_arg(ap, int);
	int   type      = va_arg(ap, int);
	const char *fontColor = va_arg(ap, const char *);
	short fontType  = (short)va_arg(ap, int);
	short fontSize  = (short)va_arg(ap, int);
	short fontAlign = (short)va_arg(ap, int);
	short fontY     = (short)va_arg(ap, int);
	if (fontColor)
		clif->broadcast2(bl, mes, len, (unsigned int)strtoul(fontColor, (char **)NULL, 0), fontType, fontSize, fontAlign, fontY, SELF);
	else
		clif->broadcast(bl, mes, len, type, SELF);
	return 0;
}
/* Runs item effect on attached character.
 * itemeffect <item id>;
 * itemeffect "<item name>"; */
static BUILDIN(itemeffect)
{
	struct npc_data *nd;
	struct item_data *item_data;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	nd = map->id2nd(sd->npc_id);
	if (nd == NULL)
		return false;

	if( script_isstringtype(st, 2) ) {
		const char *name = script_getstr(st, 2);

		if( ( item_data = itemdb->search_name( name ) ) == NULL ) {
			ShowError( "buildin_itemeffect: Nonexistant item %s requested.\n", name );
			return false;
		}
	} else {
		int nameid = script_getnum(st, 2);

		if( ( item_data = itemdb->exists( nameid ) ) == NULL ) {
			ShowError("buildin_itemeffect: Nonexistant item %d requested.\n", nameid );
			return false;
		}
	}

	script->run_use_script(sd, item_data, nd->bl.id);

	return true;
}

static BUILDIN(mapannounce)
{
	const char *mapname   = script_getstr(st,2);
	const char *mes       = script_getstr(st,3);
	int         flag      = script_getnum(st,4);
	const char *fontColor = script_hasdata(st,5) ? script_getstr(st,5) : NULL;
	int         fontType  = script_hasdata(st,6) ? script_getnum(st,6) : 0x190; // default fontType (FW_NORMAL)
	int         fontSize  = script_hasdata(st,7) ? script_getnum(st,7) : 12;    // default fontSize
	int         fontAlign = script_hasdata(st,8) ? script_getnum(st,8) : 0;     // default fontAlign
	int         fontY     = script_hasdata(st,9) ? script_getnum(st,9) : 0;     // default fontY
	int16 m;
	size_t len = strlen(mes);
	Assert_retr(false, len < INT_MAX);

	if ((m = map->mapname2mapid(mapname)) < 0)
		return true;

	map->foreachinmap(script->buildin_announce_sub, m, BL_PC,
	                  mes, (int)len+1, flag&BC_COLOR_MASK, fontColor, fontType, fontSize, fontAlign, fontY);
	return true;
}
/*==========================================
 *------------------------------------------*/
static BUILDIN(areaannounce)
{
	const char *mapname   = script_getstr(st,2);
	int         x0        = script_getnum(st,3);
	int         y0        = script_getnum(st,4);
	int         x1        = script_getnum(st,5);
	int         y1        = script_getnum(st,6);
	const char *mes       = script_getstr(st,7);
	int         flag      = script_getnum(st,8);
	const char *fontColor = script_hasdata(st,9) ? script_getstr(st,9) : NULL;
	int         fontType  = script_hasdata(st,10) ? script_getnum(st,10) : 0x190; // default fontType (FW_NORMAL)
	int         fontSize  = script_hasdata(st,11) ? script_getnum(st,11) : 12;    // default fontSize
	int         fontAlign = script_hasdata(st,12) ? script_getnum(st,12) : 0;     // default fontAlign
	int         fontY     = script_hasdata(st,13) ? script_getnum(st,13) : 0;     // default fontY
	int16 m;
	size_t len = strlen(mes);
	Assert_retr(false, len < INT_MAX);

	if ((m = map->mapname2mapid(mapname)) < 0)
		return true;

	map->foreachinarea(script->buildin_announce_sub, m, x0, y0, x1, y1, BL_PC,
	                   mes, (int)len+1, flag&BC_COLOR_MASK, fontColor, fontType, fontSize, fontAlign, fontY);
	return true;
}

/*==========================================
 *------------------------------------------*/
static BUILDIN(getusers)
{
	int flag, val = 0;
	struct map_session_data* sd;
	struct block_list* bl = NULL;

	flag = script_getnum(st,2);

	switch(flag&0x07) {
		case 0:
			if(flag&0x8) {
				// npc
				bl = map->id2bl(st->oid);
			} else if((sd = script->rid2sd(st))!=NULL) {
				// pc
				bl = &sd->bl;
			}

			if(bl) {
				val = map->list[bl->m].users;
			}
			break;
		case 1:
			val = map->getusers();
			break;
		default:
			ShowWarning("buildin_getusers: Unknown type %d.\n", flag);
			script_pushint(st,0);
			return false;
	}

	script_pushint(st,val);
	return true;
}
/*==========================================
 * Works like @WHO - displays all online users names in window
 *------------------------------------------*/
static BUILDIN(getusersname)
{
	struct map_session_data *sd;
	const struct map_session_data *pl_sd;
	int /*disp_num=1,*/ group_level = 0;
	struct s_mapiterator* iter;

	sd = script->rid2sd(st);
	if (!sd) return true;

	group_level = pc_get_group_level(sd);
	iter = mapit_getallusers();
	for (pl_sd = BL_UCCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); pl_sd = BL_UCCAST(BL_PC, mapit->next(iter))) {
		if (pc_has_permission(pl_sd, PC_PERM_HIDE_SESSION) && pc_get_group_level(pl_sd) > group_level)
			continue; // skip hidden sessions

		/* Temporary fix for bugreport:1023.
		 * Do not uncomment unless you want thousands of 'next' buttons.
		 if((disp_num++)%10==0)
		 clif->scriptnext(sd,st->oid);*/
		clif->scriptmes(sd,st->oid,pl_sd->status.name);
	}
	mapit->free(iter);

	return true;
}
/*==========================================
 * getmapguildusers("mapname",guild ID) Returns the number guild members present on a map [Reddozen]
 *------------------------------------------*/
static BUILDIN(getmapguildusers)
{
	const char *str;
	int16 m;
	int gid;
	int c=0;
	struct guild *g = NULL;
	str=script_getstr(st,2);
	gid=script_getnum(st,3);
	if ((m = map->mapname2mapid(str)) < 0) { // map id on this server (m == -1 if not in actual map-server)
		script_pushint(st,-1);
		return true;
	}
	g = guild->search(gid);

	if (g) {
		int i;
		for (i = 0; i < g->max_member; i++) {
			if (g->member[i].sd && g->member[i].sd->bl.m == m)
				c++;
		}
	}

	script_pushint(st,c);
	return true;
}
/*==========================================
 *------------------------------------------*/
static BUILDIN(getmapusers)
{
	const char *str;
	int16 m;
	str=script_getstr(st,2);
	if( (m=map->mapname2mapid(str))< 0) {
		script_pushint(st,-1);
		return true;
	}
	script_pushint(st,map->list[m].users);
	return true;
}
/*==========================================
 *------------------------------------------*/
static int buildin_getareausers_sub(struct block_list *bl, va_list ap)
{
	int *users=va_arg(ap,int *);
	nullpo_ret(users);
	(*users)++;
	return 0;
}

static BUILDIN(getareausers)
{
	int16 m = -1, x0, y0, x1, y1;
	int users = 0;
	int idx = 2;

	if (script_hasdata(st, 2) && script_isstringtype(st, 2)) {
		const char *str = script_getstr(st, 2);
		if ((m = map->mapname2mapid(str)) < 0) {
			script_pushint(st, -1);
			return true;
		}
		idx = 3;
	} else {
		struct map_session_data *sd = script->rid2sd(st);
		if (sd == NULL) {
			script_pushint(st, -1);
			return true;
		}
		m = sd->bl.m;
	}

	if (script_hasdata(st, idx + 3)) {
		x0 = script_getnum(st, idx + 0);
		y0 = script_getnum(st, idx + 1);
		x1 = script_getnum(st, idx + 2);
		y1 = script_getnum(st, idx + 3);
	} else if (st->oid) {
		struct npc_data *nd = map->id2nd(st->oid);
		if (!nd) {
			script_pushint(st, -1);
			return true;
		}
		if (script_hasdata(st, idx)) {
			int range = script_getnum(st, idx);
			x0 = nd->bl.x - range;
			y0 = nd->bl.y - range;
			x1 = nd->bl.x + range;
			y1 = nd->bl.y + range;
		} else if (nd->u.scr.xs != -1 && nd->u.scr.ys != -1) {
			x0 = nd->bl.x - nd->u.scr.xs;
			y0 = nd->bl.y - nd->u.scr.ys;
			x1 = nd->bl.x + nd->u.scr.xs;
			y1 = nd->bl.y + nd->u.scr.ys;
		} else {
			script_pushint(st, -1);
			return true;
		}
	} else {
		script_pushint(st, -1);
		return false;
	}
	map->foreachinarea(script->buildin_getareausers_sub,
	                   m, x0, y0, x1, y1, BL_PC, &users);
	script_pushint(st, users);
	return true;
}

/*==========================================
 *------------------------------------------*/
static int buildin_getareadropitem_sub(struct block_list *bl, va_list ap)
{
	int item = va_arg(ap, int);
	int *amount = va_arg(ap, int *);
	const struct flooritem_data *drop = NULL;

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_ITEM);
	drop = BL_UCCAST(BL_ITEM, bl);

	if (drop->item_data.nameid == item)
		(*amount) += drop->item_data.amount;

	return 0;
}

static BUILDIN(getareadropitem)
{
	const char *str;
	int16 m,x0,y0,x1,y1;
	int item,amount=0;

	str=script_getstr(st,2);
	x0=script_getnum(st,3);
	y0=script_getnum(st,4);
	x1=script_getnum(st,5);
	y1=script_getnum(st,6);

	if( script_isstringtype(st, 7) ) {
		const char *name = script_getstr(st, 7);
		struct item_data *item_data = itemdb->search_name(name);
		item=UNKNOWN_ITEM_ID;
		if( item_data )
			item=item_data->nameid;
	} else {
		item=script_getnum(st, 7);
	}

	if( (m=map->mapname2mapid(str))< 0) {
		script_pushint(st,-1);
		return true;
	}
	map->foreachinarea(script->buildin_getareadropitem_sub,
	                   m,x0,y0,x1,y1,BL_ITEM,item,&amount);
	script_pushint(st,amount);
	return true;
}
/*==========================================
 *------------------------------------------*/
static BUILDIN(enablenpc)
{
	const char *str;
	str=script_getstr(st,2);
	npc->enable(str,1);
	return true;
}
/*==========================================
 *------------------------------------------*/
static BUILDIN(disablenpc)
{
	const char *str;
	str=script_getstr(st,2);
	npc->enable(str,0);
	return true;
}

/*==========================================
 *------------------------------------------*/
static BUILDIN(hideoffnpc)
{
	const char *str;
	str=script_getstr(st,2);
	npc->enable(str,2);
	return true;
}
/*==========================================
 *------------------------------------------*/
static BUILDIN(hideonnpc)
{
	const char *str;
	str=script_getstr(st,2);
	npc->enable(str,4);
	return true;
}
/*==========================================
 *------------------------------------------*/
static BUILDIN(cloakonnpc)
{
	struct npc_data *nd = npc->name2id(script_getstr(st, 2));
	if (nd == NULL) {
		ShowError("buildin_cloakonnpc: invalid npc name '%s'.\n", script_getstr(st, 2));
		return false;
	}

	if (script_hasdata(st, 3)) {
		struct map_session_data *sd = map->id2sd(script_getnum(st, 3));
		if (sd == NULL)
			return false;

		uint32 val = nd->option;
		nd->option |= OPTION_CLOAK;
		clif->changeoption_target(&nd->bl, &sd->bl, SELF);
		nd->option = val;
	} else {
		nd->option |= OPTION_CLOAK;
		clif->changeoption(&nd->bl);
	}
	return true;
}
/*==========================================
 *------------------------------------------*/
static BUILDIN(cloakoffnpc)
{
	struct npc_data *nd = npc->name2id(script_getstr(st, 2));
	if (nd == NULL) {
		ShowError("buildin_cloakoffnpc: invalid npc name '%s'.\n", script_getstr(st, 2));
		return false;
	}

	if (script_hasdata(st, 3)) {
		struct map_session_data *sd = map->id2sd(script_getnum(st, 3));
		if (sd == NULL)
			return false;

		uint32 val = nd->option;
		nd->option &= ~OPTION_CLOAK;
		clif->changeoption_target(&nd->bl, &sd->bl, SELF);
		nd->option = val;
	} else {
		nd->option &= ~OPTION_CLOAK;
		clif->changeoption(&nd->bl);
	}
	return true;
}

/* Starts a status effect on the target unit or on the attached player.
 *
 * sc_start  <effect_id>,<duration>,<val1>{,<rate>,<flag>,{<unit_id>}};
 * sc_start2 <effect_id>,<duration>,<val1>,<val2>{,<rate,<flag>,{<unit_id>}};
 * sc_start4 <effect_id>,<duration>,<val1>,<val2>,<val3>,<val4>{,<rate,<flag>,{<unit_id>}};
 * <flag>: @see enum scstart_flag
 */
static BUILDIN(sc_start)
{
	struct npc_data *nd = map->id2nd(st->oid);
	struct block_list* bl;
	enum sc_type type;
	int tick, val1, val2, val3, val4=0, rate, flag;
	char start_type;
	const char* command = script->getfuncname(st);

	if(strstr(command, "4"))
		start_type = 4;
	else if(strstr(command, "2"))
		start_type = 2;
	else
		start_type = 1;

	type = (sc_type)script_getnum(st,2);
	tick = script_getnum(st,3);
	val1 = script_getnum(st,4);

	//If from NPC we make default flag SCFLAG_NOAVOID to be unavoidable
	if(nd && nd->bl.id == npc->fake_nd->bl.id)
		flag = script_hasdata(st,5+start_type) ? script_getnum(st,5+start_type) : SCFLAG_FIXEDTICK;
	else
		flag = script_hasdata(st,5+start_type) ? script_getnum(st,5+start_type) : SCFLAG_NOAVOID;

	rate = script_hasdata(st,4+start_type)?min(script_getnum(st,4+start_type),10000):10000;

	if(script_hasdata(st,(6+start_type)))
		bl = map->id2bl(script_getnum(st,(6+start_type)));
	else
		bl = map->id2bl(st->rid);

	if(tick == 0 && val1 > 0 && type > SC_NONE && type < SC_MAX && status->sc2skill(type) != 0)
	{// When there isn't a duration specified, try to get it from the skill_db
		tick = skill->get_time(status->sc2skill(type), val1);
	}

	if(script->potion_flag == 1 && script->potion_target) { //skill.c set the flags before running the script, this is a potion-pitched effect.
		bl = map->id2bl(script->potion_target);
		tick /= 2;// Thrown potions only last half.
		val4 = 1;// Mark that this was a thrown sc_effect
	}

	if(!bl)
		return true;

	switch(start_type) {
		case 1:
			status->change_start(bl, bl, type, rate, val1, 0, 0, val4, tick, flag);
			break;
		case 2:
			val2 = script_getnum(st,5);
			status->change_start(bl, bl, type, rate, val1, val2, 0, val4, tick, flag);
			break;
		case 4:
			val2 = script_getnum(st,5);
			val3 = script_getnum(st,6);
			val4 = script_getnum(st,7);
			status->change_start(bl, bl, type, rate, val1, val2, val3, val4, tick, flag);
			break;
	}
	return true;
}

/// Ends one or all status effects on the target unit or on the attached player.
///
/// sc_end <effect_id>{,<unit_id>};
static BUILDIN(sc_end)
{
	struct block_list* bl;
	int type;

	type = script_getnum(st, 2);
	if (script_hasdata(st, 3))
		bl = map->id2bl(script_getnum(st, 3));
	else
		bl = map->id2bl(st->rid);

	if (script->potion_flag == 1 && script->potion_target) //##TODO how does this work [FlavioJS]
		bl = map->id2bl(script->potion_target);

	if (!bl)
		return true;

	if (type >= 0 && type < SC_MAX) {
		struct status_change *sc = status->get_sc(bl);
		struct status_change_entry *sce = sc ? sc->data[type] : NULL;

		if (!sce)
			return true;

		/* status that can't be individually removed (TODO sc_config option?) */
		switch (type) {
			case SC_WEIGHTOVER50:
			case SC_WEIGHTOVER90:
			case SC_NOCHAT:
			case SC_PUSH_CART:
				return true;
			default:
				break;
		}

		//This should help status_change_end force disabling the SC in case it has no limit.
		if (type != SC_BERSERK)
			sce->val1 = 0; // SC_BERSERK requires skill_lv that's stored in sce->val1 when being removed [KirieZ]
		sce->val2 = sce->val3 = sce->val4 = 0;
		status_change_end(bl, (sc_type)type, INVALID_TIMER);
	}
	else
		status->change_clear(bl, 3); // remove all effects

	return true;
}

/*==========================================
 * @FIXME atm will return reduced tick, 0 immune, 1 no tick
 *------------------------------------------*/
static BUILDIN(getscrate)
{
	struct block_list *bl;
	int type,rate;

	type=script_getnum(st,2);
	rate=script_getnum(st,3);
	if( script_hasdata(st,4) ) //get for the bl assigned
		bl = map->id2bl(script_getnum(st,4));
	else
		bl = map->id2bl(st->rid);

	if (bl)
		rate = status->get_sc_def(bl, bl, (sc_type)type, 10000, 10000, SCFLAG_NONE);

	script_pushint(st,rate);
	return true;
}

/*==========================================
 * getstatus <type>{, <info>};
 *------------------------------------------*/
static BUILDIN(getstatus)
{
	int id, type;
	struct map_session_data* sd = script->rid2sd(st);

	if( sd == NULL )
	{// no player attached
		return true;
	}

	id = script_getnum(st, 2);
	type = script_hasdata(st, 3) ? script_getnum(st, 3) : 0;

	if( id <= SC_NONE || id >= SC_MAX )
	{// invalid status type given
		ShowWarning("script.c:getstatus: Invalid status type given (%d).\n", id);
		return true;
	}

	if( sd->sc.count == 0 || !sd->sc.data[id] )
	{// no status is active
		script_pushint(st, 0);
		return true;
	}

	switch( type ) {
		case 1: script_pushint(st, sd->sc.data[id]->val1); break;
		case 2: script_pushint(st, sd->sc.data[id]->val2); break;
		case 3: script_pushint(st, sd->sc.data[id]->val3); break;
		case 4: script_pushint(st, sd->sc.data[id]->val4); break;
		case 5:
			if (sd->sc.data[id]->infinite_duration) {
				script_pushint(st, INFINITE_DURATION);
			} else {
				const struct TimerData *td = timer->get(sd->sc.data[id]->timer);

				if (td != NULL) {
					// return the amount of time remaining
					script_pushint(st, (int)(td->tick - timer->gettick())); // TODO: change this to int64 when we'll support 64 bit script values
				}
			}
			break;
		default: script_pushint(st, 1); break;
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
static BUILDIN(debugmes)
{
	struct StringBuf buf;
	StrBuf->Init(&buf);

	if (!script->sprintf_helper(st, 2, &buf)) {
		StrBuf->Destroy(&buf);
		script_pushint(st, 0);
		return false;
	}

	ShowDebug("script debug : %d %d : %s\n", st->rid, st->oid, StrBuf->Value(&buf));
	StrBuf->Destroy(&buf);
	script_pushint(st, 1);
	return true;
}

/*==========================================
 *------------------------------------------*/
static BUILDIN(catchpet)
{
	int pet_id;
	struct map_session_data *sd;

	pet_id= script_getnum(st,2);
	sd=script->rid2sd(st);
	if( sd == NULL )
		return true;

	pet->catch_process1(sd,pet_id);
	return true;
}

/*==========================================
 * [orn]
 *------------------------------------------*/
static BUILDIN(homunculus_evolution)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	if(homun_alive(sd->hd)) {
		if (sd->hd->homunculus.intimacy > 91000)
			homun->evolve(sd->hd);
		else
			clif->emotion(&sd->hd->bl, E_SWT);
	}
	return true;
}

/*==========================================
 * [Xantara]
 * Checks for vaporized morph state
 * and deletes ITEMID_STRANGE_EMBRYO.
 *------------------------------------------*/
static BUILDIN(homunculus_mutate)
{
	bool success = false;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL || sd->hd == NULL)
		return true;

	if (sd->hd->homunculus.vaporize == HOM_ST_MORPH) {
		enum homun_type m_class, m_id;
		int homun_id;
		int i = pc->search_inventory(sd, ITEMID_STRANGE_EMBRYO);
		if (script_hasdata(st,2))
			homun_id = script_getnum(st,2);
		else
			homun_id = HOMID_EIRA + (rnd() % 4);

		m_class = homun->class2type(sd->hd->homunculus.class_);
		m_id    = homun->class2type(homun_id);

		if (m_class == HT_EVO && m_id == HT_S &&
			sd->hd->homunculus.level >= 99 && i != INDEX_NOT_FOUND &&
			!pc->delitem(sd, i, 1, 0, DELITEM_NORMAL, LOG_TYPE_SCRIPT) ) {
			sd->hd->homunculus.vaporize = HOM_ST_REST; // Remove morph state.
			homun->call(sd); // Respawn homunculus.
			homun->mutate(sd->hd, homun_id);
			success = true;
		} else {
			clif->emotion(&sd->hd->bl, E_SWT);
		}
	} else {
		clif->emotion(&sd->hd->bl, E_SWT);
	}

	script_pushint(st,success?1:0);
	return true;
}

/*==========================================
 * Puts homunculus into morph state
 * and gives ITEMID_STRANGE_EMBRYO.
 *------------------------------------------*/
static BUILDIN(homunculus_morphembryo)
{
	bool success = false;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL || sd->hd == NULL)
		return true;

	if (homun_alive(sd->hd)) {
		enum homun_type m_class = homun->class2type(sd->hd->homunculus.class_);

		if (m_class == HT_EVO && sd->hd->homunculus.level >= 99) {
			struct item item_tmp;
			int i = 0;

			memset(&item_tmp, 0, sizeof(item_tmp));
			item_tmp.nameid = ITEMID_STRANGE_EMBRYO;
			item_tmp.identify = 1;

			if ((i = pc->additem(sd, &item_tmp, 1, LOG_TYPE_SCRIPT))) {
				clif->additem(sd, 0, 0, i);
				clif->emotion(&sd->hd->bl, E_SWT);
			} else {
				homun->vaporize(sd, HOM_ST_MORPH, true);
				success = true;
			}
		} else {
			clif->emotion(&sd->hd->bl, E_SWT);
		}
	} else {
		clif->emotion(&sd->hd->bl, E_SWT);
	}

	script_pushint(st, success?1:0);
	return true;
}

/*==========================================
 * Check for homunculus state.
 * Return: -1 = No homunculus
 *          0 = Homunculus is active
 *          1 = Homunculus is vaporized (rest)
 *          2 = Homunculus is in morph state
 *------------------------------------------*/
static BUILDIN(homunculus_checkcall)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL || sd->hd == NULL)
		script_pushint(st, -1);
	else
		script_pushint(st, sd->hd->homunculus.vaporize);

	return true;
}

// [Zephyrus]
static BUILDIN(homunculus_shuffle)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	if(homun_alive(sd->hd))
		homun->shuffle(sd->hd);

	return true;
}

//These two functions bring the eA MAPID_* class functionality to scripts.
static BUILDIN(eaclass)
{
	int class;
	if (script_hasdata(st,2)) {
		class = script_getnum(st,2);
	} else {
		struct map_session_data *sd = script->rid2sd(st);
		if (sd == NULL)
			return true;
		class = sd->status.class;
	}
	script_pushint(st,pc->jobid2mapid(class));
	return true;
}

static BUILDIN(roclass)
{
	int job = script_getnum(st,2);
	int sex;
	if (script_hasdata(st,3)) {
		sex = script_getnum(st,3);
	} else {
		struct map_session_data *sd;
		if (st->rid && (sd=script->rid2sd(st)) != NULL)
			sex = sd->status.sex;
		else
			sex = 1; //Just use male when not found.
	}
	script_pushint(st,pc->mapid2jobid(job, sex));
	return true;
}

/*==========================================
 * Tells client to open a hatching window, used for pet incubator
 *------------------------------------------*/
static BUILDIN(birthpet)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	if( sd->status.pet_id )
	{// do not send egg list, when you already have a pet
		return true;
	}

	clif->sendegg(sd);
	return true;
}

/*==========================================
 * Added - AppleGirl For Advanced Classes, (Updated for Cleaner Script Purposes)
 * @type
 * 1 : make like after rebirth
 * 2 : blvl,jlvl=1, skillpoint=0
 * 3 : don't reset skill, blvl=1
 * 4 : jlvl=0
 *------------------------------------------*/
static BUILDIN(resetlvl)
{
	int type=script_getnum(st,2);
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	pc->resetlvl(sd,type);
	return true;
}
/*==========================================
 * Reset a player status point
 *------------------------------------------*/
static BUILDIN(resetstatus)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;
	pc->resetstate(sd);
	return true;
}

/*==========================================
 * script command resetskill
 *------------------------------------------*/
static BUILDIN(resetskill)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;
	pc->resetskill(sd, PCRESETSKILL_RESYNC);
	return true;
}

/*==========================================
 * Counts total amount of skill points.
 *------------------------------------------*/
static BUILDIN(skillpointcount)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;
	script_pushint(st,sd->status.skill_point + pc->resetskill(sd, PCRESETSKILL_RECOUNT));
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
static BUILDIN(changebase)
{
	struct map_session_data *sd = NULL;
	int vclass;

	if (script_hasdata(st,3))
		sd = script->id2sd(st, script_getnum(st,3));
	else
		sd=script->rid2sd(st);

	if (sd == NULL)
		return true;

	vclass = script_getnum(st,2);
	if(vclass == JOB_WEDDING)
	{
		if (!battle_config.wedding_modifydisplay || //Do not show the wedding sprites
			sd->job & JOBL_BABY //Baby classes screw up when showing wedding sprites. [Skotlex] They don't seem to anymore.
			)
			return true;
	}

	if (sd->disguise == -1 && vclass != sd->vd.class)
		pc->changelook(sd,LOOK_BASE,vclass); //Updated client view. Base, Weapon and Cloth Colors.

	return true;
}

static struct map_session_data *prepareChangeSex(struct script_state *st)
{
	int i;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return NULL;

	pc->resetskill(sd, PCRESETSKILL_CHSEX);
	// to avoid any problem with equipment and invalid sex, equipment is unequiped.
	for (i=0; i<EQI_MAX; i++)
		if (sd->equip_index[i] >= 0) pc->unequipitem(sd, sd->equip_index[i], PCUNEQUIPITEM_RECALC|PCUNEQUIPITEM_FORCE);
	return sd;
}

/*==========================================
 * Unequip all item and request for a changesex to char-serv
 *------------------------------------------*/
static BUILDIN(changesex)
{
	struct map_session_data *sd = prepareChangeSex(st);
	if (sd == NULL)
		return true;
	chrif->changesex(sd, true);
	return true;
}

/*==========================================
 * Unequip all items and change character sex [4144]
 *------------------------------------------*/
static BUILDIN(changecharsex)
{
	struct map_session_data *sd = prepareChangeSex(st);
	if (sd == NULL)
		return true;
	chrif->changesex(sd, false);
	return true;
}

/*==========================================
 * Works like 'announce' but outputs in the common chat window
 *------------------------------------------*/
static BUILDIN(globalmes)
{
	const char *name=NULL,*mes;

	mes=script_getstr(st,2);
	if(mes==NULL) return true;

	if(script_hasdata(st,3)) {
		//  npc name to display
		name=script_getstr(st,3);
	} else {
		const struct npc_data *nd = map->id2nd(st->oid);
		nullpo_retr(false, nd);
		name = nd->name; //use current npc name
	}

	npc->globalmessage(name,mes); // broadcast to all players connected

	return true;
}

/////////////////////////////////////////////////////////////////////
// NPC waiting room (chat room)
//

/// Creates a waiting room (chat room) for this npc.
///
/// waitingroom "<title>",<limit>{,"<event>"{,<trigger>{,<zeny>{,<minlvl>{,<maxlvl>}}}}};
static BUILDIN(waitingroom)
{
	struct npc_data* nd;
	const char* title = script_getstr(st, 2);
	int limit = script_getnum(st, 3);
	const char* ev = script_hasdata(st,4) ? script_getstr(st,4) : "";
	int trigger =  script_hasdata(st,5) ? script_getnum(st,5) : limit;
	int zeny =  script_hasdata(st,6) ? script_getnum(st,6) : 0;
	int minLvl =  script_hasdata(st,7) ? script_getnum(st,7) : 1;
	int maxLvl =  script_hasdata(st,8) ? script_getnum(st,8) : MAX_LEVEL;

	nd = map->id2nd(st->oid);
	if (nd != NULL) {
		int pub = 1;
		chat->create_npc_chat(nd, title, limit, pub, trigger, ev, zeny, minLvl, maxLvl);
	}

	return true;
}

/// Removes the waiting room of the current or target npc.
///
/// delwaitingroom "<npc_name>";
/// delwaitingroom;
static BUILDIN(delwaitingroom)
{
	struct npc_data* nd;
	if( script_hasdata(st,2) )
		nd = npc->name2id(script_getstr(st, 2));
	else
		nd = map->id2nd(st->oid);
	if (nd != NULL)
		chat->delete_npc_chat(nd);
	return true;
}

/// Kicks all the players from the waiting room of the current or target npc.
///
/// kickwaitingroomall "<npc_name>";
/// kickwaitingroomall;
static BUILDIN(waitingroomkickall)
{
	struct npc_data* nd;
	struct chat_data* cd;

	if( script_hasdata(st,2) )
		nd = npc->name2id(script_getstr(st,2));
	else
		nd = map->id2nd(st->oid);

	if (nd != NULL && (cd=map->id2cd(nd->chat_id)) != NULL)
		chat->npc_kick_all(cd);
	return true;
}

/// Enables the waiting room event of the current or target npc.
///
/// enablewaitingroomevent "<npc_name>";
/// enablewaitingroomevent;
static BUILDIN(enablewaitingroomevent)
{
	struct npc_data* nd;
	struct chat_data* cd;

	if( script_hasdata(st,2) )
		nd = npc->name2id(script_getstr(st, 2));
	else
		nd = map->id2nd(st->oid);

	if (nd != NULL && (cd=map->id2cd(nd->chat_id)) != NULL)
		chat->enable_event(cd);
	return true;
}

/// Disables the waiting room event of the current or target npc.
///
/// disablewaitingroomevent "<npc_name>";
/// disablewaitingroomevent;
static BUILDIN(disablewaitingroomevent)
{
	struct npc_data *nd;
	struct chat_data *cd;

	if( script_hasdata(st,2) )
		nd = npc->name2id(script_getstr(st, 2));
	else
		nd = map->id2nd(st->oid);

	if (nd != NULL && (cd=map->id2cd(nd->chat_id)) != NULL)
		chat->disable_event(cd);
	return true;
}

/// Returns info on the waiting room of the current or target npc.
/// Returns -1 if the type unknown
/// <type>=0 : current number of users
/// <type>=1 : maximum number of users allowed
/// <type>=2 : the number of users that trigger the event
/// <type>=3 : if the trigger is disabled
/// <type>=4 : the title of the waiting room
/// <type>=5 : the password of the waiting room
/// <type>=16 : the name of the waiting room event
/// <type>=32 : if the waiting room is full
/// <type>=33 : if there are enough users to trigger the event
/// -- Custom Added
/// <type>=34 : minimum player of waiting room
/// <type>=35 : maximum player of waiting room
/// <type>=36 : minimum zeny required
///
/// getwaitingroomstate(<type>,"<npc_name>") -> <info>
/// getwaitingroomstate(<type>) -> <info>
static BUILDIN(getwaitingroomstate)
{
	const struct npc_data *nd;
	const struct chat_data *cd;
	int type;
	int i;

	type = script_getnum(st,2);
	if( script_hasdata(st,3) )
		nd = npc->name2id(script_getstr(st, 3));
	else
		nd = map->id2nd(st->oid);

	if (nd == NULL || (cd=map->id2cd(nd->chat_id)) == NULL) {
		script_pushint(st, -1);
		return true;
	}

	switch(type) {
		case 0:
			for (i = 0; i < cd->users; i++) {
				struct map_session_data *sd = cd->usersd[i];
				nullpo_retr(false, sd);
				mapreg->setreg(reference_uid(script->add_variable("$@chatmembers"), i), sd->bl.id);
			}
			script_pushint(st, cd->users);
			break;
		case 1:  script_pushint(st, cd->limit); break;
		case 2:  script_pushint(st, cd->trigger&0x7f); break;
		case 3:  script_pushint(st, ((cd->trigger&0x80)!=0)); break;
		case 4:  script_pushstrcopy(st, cd->title); break;
		case 5:  script_pushstrcopy(st, cd->pass); break;
		case 16: script_pushstrcopy(st, cd->npc_event);break;
		case 32: script_pushint(st, (cd->users >= cd->limit)); break;
		case 33: script_pushint(st, (cd->users >= cd->trigger)); break;

		case 34: script_pushint(st, cd->min_level); break;
		case 35: script_pushint(st, cd->max_level); break;
		case 36: script_pushint(st, cd->zeny); break;
		default: script_pushint(st, -1); break;
	}
	return true;
}

/// Warps the trigger or target amount of players to the target map and position.
/// Players are automatically removed from the waiting room.
/// Those waiting the longest will get warped first.
/// The target map can be "Random" for a random position in the current map,
/// and "SavePoint" for the savepoint map+position.
/// The map flag noteleport of the current map is only considered when teleporting to the savepoint.
///
/// The id's of the teleported players are put into the array $@warpwaitingpc[]
/// The total number of teleported players is put into $@warpwaitingpcnum
///
/// warpwaitingpc "<map name>",<x>,<y>,<number of players>;
/// warpwaitingpc "<map name>",<x>,<y>;
static BUILDIN(warpwaitingpc)
{
	int x, y, i, n;
	const char* map_name;
	struct npc_data* nd;
	struct chat_data* cd;

	nd = map->id2nd(st->oid);
	if (nd == NULL || (cd=map->id2cd(nd->chat_id)) == NULL)
		return true;

	map_name = script_getstr(st,2);
	x = script_getnum(st,3);
	y = script_getnum(st,4);
	n = cd->trigger&0x7f;

	if( script_hasdata(st,5) )
		n = script_getnum(st,5);

	for (i = 0; i < n && cd->users > 0; i++) {
		struct map_session_data *sd = cd->usersd[0];

		nullpo_retr(false, sd);
		if (strcmp(map_name,"SavePoint") == 0 && map->list[sd->bl.m].flag.noteleport) {
			// can't teleport on this map
			break;
		}

		if (cd->zeny) {
			// fee set
			if( (uint32)sd->status.zeny < cd->zeny ) {
				// no zeny to cover set fee
				break;
			}
			pc->payzeny(sd, cd->zeny, LOG_TYPE_NPC, NULL);
		}

		mapreg->setreg(reference_uid(script->add_variable("$@warpwaitingpc"), i), sd->bl.id);

		if( strcmp(map_name,"Random") == 0 )
			pc->randomwarp(sd,CLR_TELEPORT);
		else if( strcmp(map_name,"SavePoint") == 0 )
			pc->setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, CLR_TELEPORT);
		else
			pc->setpos(sd, script->mapindexname2id(st,map_name), x, y, CLR_OUTSIGHT);
	}
	mapreg->setreg(script->add_variable("$@warpwaitingpcnum"), i);
	return true;
}

/////////////////////////////////////////////////////////////////////
// ...
//

/// Detaches a character from a script.
///
/// @param st Script state to detach the character from.
static void script_detach_rid(struct script_state *st)
{
	if(st->rid) {
		script->detach_state(st, false);
		st->rid = 0;
	}
}

/*==========================================
 * Attach sd char id to script and detach current one if any
 *------------------------------------------*/
static BUILDIN(attachrid)
{
	int rid = script_getnum(st,2);

	if (map->id2sd(rid) != NULL) {
		script->detach_rid(st);

		st->rid = rid;
		script->attach_state(st);
		script_pushint(st,1);
	} else
		script_pushint(st,0);
	return true;
}
/*==========================================
 * Detach script to rid
 *------------------------------------------*/
static BUILDIN(detachrid)
{
	script->detach_rid(st);
	return true;
}
/*==========================================
 * Chk if account connected, (and charid from account if specified)
 *------------------------------------------*/
static BUILDIN(isloggedin)
{
	struct map_session_data *sd = map->id2sd(script_getnum(st,2));
	if (script_hasdata(st,3) && sd != NULL
	 && sd->status.char_id != script_getnum(st,3))
		sd = NULL;
	script->push_val(st->stack,C_INT,sd!=NULL,NULL);
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
static BUILDIN(setmapflagnosave)
{
	int16 m,x,y;
	unsigned short map_index;
	const char *str,*str2;

	str=script_getstr(st,2);
	str2=script_getstr(st,3);
	x=script_getnum(st,4);
	y=script_getnum(st,5);
	m = map->mapname2mapid(str);
	map_index = script->mapindexname2id(st,str2);

	if(m >= 0 && map_index) {
		map->list[m].flag.nosave=1;
		map->list[m].save.map=map_index;
		map->list[m].save.x=x;
		map->list[m].save.y=y;
	}

	return true;
}

enum mapinfo_info {
	MAPINFO_NAME,
	MAPINFO_ID,
	MAPINFO_SIZE_X,
	MAPINFO_SIZE_Y,
	MAPINFO_ZONE,
	MAPINFO_NPC_COUNT
};

static BUILDIN(getmapinfo)
{
	enum mapinfo_info mode = script_getnum(st, 2);
	int16 m = -1;

	if (script_hasdata(st, 3)) {
		if (script_isstringtype(st, 3)) {
			const char *str = script_getstr(st, 3);
			m = map->mapindex2mapid(strdb_iget(mapindex->db, str));
		} else {
			m = script_getnum(st, 3);
		}
	} else {
		struct block_list *bl = NULL;

		if (st->oid) {
			bl = map->id2bl(st->oid);
		} else if (st->rid) {
			bl = map->id2bl(st->rid);
		}

		if (bl == NULL) {
			ShowError("buildin_getmapinfo: map not supplied and NPC/PC not attached!\n");
			script_pushint(st, -3);
			return false;
		}

		m = bl->m;
	}

	if (m < 0) {
		// here we don't throw an error, so the command can be used
		// to detect whether or not a map exists
		script_pushint(st, -1);
		return true;
	}

	switch (mode) {
	case MAPINFO_NAME:
		script_pushconststr(st, map->list[m].name);
		break;
	case MAPINFO_ID:
		script_pushint(st, m);
		break;
	case MAPINFO_SIZE_X:
		script_pushint(st, map->list[m].xs);
		break;
	case MAPINFO_SIZE_Y:
		script_pushint(st, map->list[m].ys);
		break;
	case MAPINFO_ZONE:
		script_pushstrcopy(st, map->list[m].zone->name);
		break;
	case MAPINFO_NPC_COUNT:
		script_pushint(st, map->list[m].npc_num);
		break;
	default:
		ShowError("buildin_getmapinfo: unknown option in second argument (%u).\n", mode);
		script_pushint(st, -2);
		return false;
	}

	return true;
}

static BUILDIN(getmapflag)
{
	int16 m,i;
	const char *str;

	str=script_getstr(st,2);
	i=script_getnum(st,3);

	m = map->mapname2mapid(str);
	if(m >= 0) {
		switch(i) {
			case MF_NOMEMO:             script_pushint(st,map->list[m].flag.nomemo); break;
			case MF_NOTELEPORT:         script_pushint(st,map->list[m].flag.noteleport); break;
			case MF_NOSAVE:             script_pushint(st,map->list[m].flag.nosave); break;
			case MF_NOBRANCH:           script_pushint(st,map->list[m].flag.nobranch); break;
			case MF_NOPENALTY:          script_pushint(st,map->list[m].flag.noexppenalty); break;
			case MF_NOZENYPENALTY:      script_pushint(st,map->list[m].flag.nozenypenalty); break;
			case MF_PVP:                script_pushint(st,map->list[m].flag.pvp); break;
			case MF_PVP_NOPARTY:        script_pushint(st,map->list[m].flag.pvp_noparty); break;
			case MF_PVP_NOGUILD:        script_pushint(st,map->list[m].flag.pvp_noguild); break;
			case MF_GVG:                script_pushint(st,map->list[m].flag.gvg); break;
			case MF_GVG_NOPARTY:        script_pushint(st,map->list[m].flag.gvg_noparty); break;
			case MF_NOTRADE:            script_pushint(st,map->list[m].flag.notrade); break;
			case MF_NOSKILL:            script_pushint(st,map->list[m].flag.noskill); break;
			case MF_NOWARP:             script_pushint(st,map->list[m].flag.nowarp); break;
			case MF_PARTYLOCK:          script_pushint(st,map->list[m].flag.partylock); break;
			case MF_NOICEWALL:          script_pushint(st,map->list[m].flag.noicewall); break;
			case MF_SNOW:               script_pushint(st,map->list[m].flag.snow); break;
			case MF_FOG:                script_pushint(st,map->list[m].flag.fog); break;
			case MF_SAKURA:             script_pushint(st,map->list[m].flag.sakura); break;
			case MF_LEAVES:             script_pushint(st,map->list[m].flag.leaves); break;
			case MF_CLOUDS:             script_pushint(st,map->list[m].flag.clouds); break;
			case MF_CLOUDS2:            script_pushint(st,map->list[m].flag.clouds2); break;
			case MF_FIREWORKS:          script_pushint(st,map->list[m].flag.fireworks); break;
			case MF_GVG_CASTLE:         script_pushint(st,map->list[m].flag.gvg_castle); break;
			case MF_GVG_DUNGEON:        script_pushint(st,map->list[m].flag.gvg_dungeon); break;
			case MF_NIGHTENABLED:       script_pushint(st,map->list[m].flag.nightenabled); break;
			case MF_NOBASEEXP:          script_pushint(st,map->list[m].flag.nobaseexp); break;
			case MF_NOJOBEXP:           script_pushint(st,map->list[m].flag.nojobexp); break;
			case MF_NOMOBLOOT:          script_pushint(st,map->list[m].flag.nomobloot); break;
			case MF_NOMVPLOOT:          script_pushint(st,map->list[m].flag.nomvploot); break;
			case MF_NORETURN:           script_pushint(st,map->list[m].flag.noreturn); break;
			case MF_NOWARPTO:           script_pushint(st,map->list[m].flag.nowarpto); break;
			case MF_NIGHTMAREDROP:      script_pushint(st,map->list[m].flag.pvp_nightmaredrop); break;
			case MF_NOCOMMAND:          script_pushint(st,map->list[m].nocommand); break;
			case MF_NODROP:             script_pushint(st,map->list[m].flag.nodrop); break;
			case MF_JEXP:               script_pushint(st,map->list[m].jexp); break;
			case MF_BEXP:               script_pushint(st,map->list[m].bexp); break;
			case MF_NOVENDING:          script_pushint(st,map->list[m].flag.novending); break;
			case MF_LOADEVENT:          script_pushint(st,map->list[m].flag.loadevent); break;
			case MF_NOCHAT:             script_pushint(st,map->list[m].flag.nochat); break;
			case MF_NOEXPPENALTY:       script_pushint(st,map->list[m].flag.noexppenalty ); break;
			case MF_GUILDLOCK:          script_pushint(st,map->list[m].flag.guildlock); break;
			case MF_TOWN:               script_pushint(st,map->list[m].flag.town); break;
			case MF_AUTOTRADE:          script_pushint(st,map->list[m].flag.autotrade); break;
			case MF_ALLOWKS:            script_pushint(st,map->list[m].flag.allowks); break;
			case MF_MONSTER_NOTELEPORT: script_pushint(st,map->list[m].flag.monster_noteleport); break;
			case MF_PVP_NOCALCRANK:     script_pushint(st,map->list[m].flag.pvp_nocalcrank); break;
			case MF_BATTLEGROUND:       script_pushint(st,map->list[m].flag.battleground); break;
			case MF_RESET:              script_pushint(st,map->list[m].flag.reset); break;
			case MF_NOTOMB:             script_pushint(st,map->list[m].flag.notomb); break;
			case MF_NOCASHSHOP:         script_pushint(st,map->list[m].flag.nocashshop); break;
			case MF_NOAUTOLOOT:         script_pushint(st, map->list[m].flag.noautoloot); break;
			case MF_NOVIEWID:           script_pushint(st, map->list[m].flag.noviewid); break;
			case MF_PAIRSHIP_STARTABLE: script_pushint(st, map->list[m].flag.pairship_startable); break;
			case MF_PAIRSHIP_ENDABLE:   script_pushint(st, map->list[m].flag.pairship_endable); break;
			case MF_NOSTORAGE:          script_pushint(st, map->list[m].flag.nostorage); break;
			case MF_NOGSTORAGE:         script_pushint(st, map->list[m].flag.nogstorage); break;
		}
	}

	return true;
}
/* pvp timer handling */
static int script_mapflag_pvp_sub(struct block_list *bl, va_list ap)
{
	struct map_session_data *sd = NULL;

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_PC);
	sd = BL_UCAST(BL_PC, bl);

	if (sd->pvp_timer == INVALID_TIMER) {
		if (!map->list[sd->bl.m].flag.pvp_nocalcrank)
			sd->pvp_timer = timer->add(timer->gettick() + 200, pc->calc_pvprank_timer, sd->bl.id, 0);
		sd->pvp_rank = 0;
		sd->pvp_lastusers = 0;
		sd->pvp_point = 5;
		sd->pvp_won = 0;
		sd->pvp_lost = 0;
	}
	clif->map_property(sd, MAPPROPERTY_FREEPVPZONE);
	clif->maptypeproperty2(&sd->bl,SELF);
	return 0;
}

static BUILDIN(setmapflag)
{
	int16 m,i;
	const char *str, *val2 = NULL;
	int val=0;

	str=script_getstr(st,2);

	i = script_getnum(st, 3);

	if (script_hasdata(st,4)) {
		if (script_isstringtype(st, 4)) {
			val2 = script_getstr(st, 4);
		} else if (script_isinttype(st, 4)) {
			val = script_getnum(st, 4);
		} else {
			ShowError("buildin_setmapflag: invalid data type for argument 3.\n");
			return false;
		}
	}
	m = map->mapname2mapid(str);

	if(m >= 0) {
		switch(i) {
			case MF_NOMEMO:             map->list[m].flag.nomemo = 1; break;
			case MF_NOTELEPORT:         map->list[m].flag.noteleport = 1; break;
			case MF_NOSAVE:             map->list[m].flag.nosave = 1; break;
			case MF_NOBRANCH:           map->list[m].flag.nobranch = 1; break;
			case MF_NOPENALTY:          map->list[m].flag.noexppenalty = 1; map->list[m].flag.nozenypenalty = 1; break;
			case MF_NOZENYPENALTY:      map->list[m].flag.nozenypenalty = 1; break;
			case MF_PVP:
				map->list[m].flag.pvp = 1;
				if( !battle_config.pk_mode ) {
					map->foreachinmap(script->mapflag_pvp_sub,m,BL_PC);
				}
				break;
			case MF_PVP_NOPARTY:        map->list[m].flag.pvp_noparty = 1; break;
			case MF_PVP_NOGUILD:        map->list[m].flag.pvp_noguild = 1; break;
			case MF_GVG: {
				struct block_list bl;
				memset(&bl, 0, sizeof(bl));
				map->list[m].flag.gvg = 1;
				clif->map_property_mapall(m, MAPPROPERTY_AGITZONE);
				bl.type = BL_NUL;
				bl.m = m;
				clif->maptypeproperty2(&bl,ALL_SAMEMAP);
			}
				break;
			case MF_GVG_NOPARTY:        map->list[m].flag.gvg_noparty = 1; break;
			case MF_NOTRADE:            map->list[m].flag.notrade = 1; break;
			case MF_NOSKILL:            map->list[m].flag.noskill = 1; break;
			case MF_NOWARP:             map->list[m].flag.nowarp = 1; break;
			case MF_PARTYLOCK:          map->list[m].flag.partylock = 1; break;
			case MF_NOICEWALL:          map->list[m].flag.noicewall = 1; break;
			case MF_SNOW:               map->list[m].flag.snow = 1; break;
			case MF_FOG:                map->list[m].flag.fog = 1; break;
			case MF_SAKURA:             map->list[m].flag.sakura = 1; break;
			case MF_LEAVES:             map->list[m].flag.leaves = 1; break;
			case MF_CLOUDS:             map->list[m].flag.clouds = 1; break;
			case MF_CLOUDS2:            map->list[m].flag.clouds2 = 1; break;
			case MF_FIREWORKS:          map->list[m].flag.fireworks = 1; break;
			case MF_GVG_CASTLE:         map->list[m].flag.gvg_castle = 1; break;
			case MF_GVG_DUNGEON:        map->list[m].flag.gvg_dungeon = 1; break;
			case MF_NIGHTENABLED:       map->list[m].flag.nightenabled = 1; break;
			case MF_NOBASEEXP:          map->list[m].flag.nobaseexp = 1; break;
			case MF_NOJOBEXP:           map->list[m].flag.nojobexp = 1; break;
			case MF_NOMOBLOOT:          map->list[m].flag.nomobloot = 1; break;
			case MF_NOMVPLOOT:          map->list[m].flag.nomvploot = 1; break;
			case MF_NORETURN:           map->list[m].flag.noreturn = 1; break;
			case MF_NOWARPTO:           map->list[m].flag.nowarpto = 1; break;
			case MF_NIGHTMAREDROP:      map->list[m].flag.pvp_nightmaredrop = 1; break;
			case MF_ZONE:
				if (val2 != NULL) {
					const char *zone = "zone";
					const char *empty = "";
					char params[MAP_ZONE_MAPFLAG_LENGTH];
					memcpy(params, val2, MAP_ZONE_MAPFLAG_LENGTH);
					npc->parse_mapflag(map->list[m].name, empty, zone, params, empty, empty, empty, NULL);
				}
				break;
			case MF_NOCOMMAND:          map->list[m].nocommand = (val <= 0) ? 100 : val; break;
			case MF_NODROP:             map->list[m].flag.nodrop = 1; break;
			case MF_JEXP:               map->list[m].jexp = (val <= 0) ? 100 : val; break;
			case MF_BEXP:               map->list[m].bexp = (val <= 0) ? 100 : val; break;
			case MF_NOVENDING:          map->list[m].flag.novending = 1; break;
			case MF_LOADEVENT:          map->list[m].flag.loadevent = 1; break;
			case MF_NOCHAT:             map->list[m].flag.nochat = 1; break;
			case MF_NOEXPPENALTY:       map->list[m].flag.noexppenalty  = 1; break;
			case MF_GUILDLOCK:          map->list[m].flag.guildlock = 1; break;
			case MF_TOWN:               map->list[m].flag.town = 1; break;
			case MF_AUTOTRADE:          map->list[m].flag.autotrade = 1; break;
			case MF_ALLOWKS:            map->list[m].flag.allowks = 1; break;
			case MF_MONSTER_NOTELEPORT: map->list[m].flag.monster_noteleport = 1; break;
			case MF_PVP_NOCALCRANK:     map->list[m].flag.pvp_nocalcrank = 1; break;
			case MF_BATTLEGROUND:       map->list[m].flag.battleground = (val <= 0 || val > 2) ? 1 : val; break;
			case MF_RESET:              map->list[m].flag.reset = 1; break;
			case MF_NOTOMB:             map->list[m].flag.notomb = 1; break;
			case MF_NOCASHSHOP:         map->list[m].flag.nocashshop = 1; break;
			case MF_NOAUTOLOOT:         map->list[m].flag.noautoloot = 1; break;
			case MF_NOVIEWID:           map->list[m].flag.noviewid = (val <= 0) ? EQP_NONE : val; break;
			case MF_PAIRSHIP_STARTABLE: map->list[m].flag.pairship_startable = 1; break;
			case MF_PAIRSHIP_ENDABLE:   map->list[m].flag.pairship_endable = 1; break;
			case MF_NOSTORAGE:          map->list[m].flag.nostorage = cap_value(val, 0, 3); break;
			case MF_NOGSTORAGE:         map->list[m].flag.nogstorage = cap_value(val, 0, 3); break;
		}
	}

	return true;
}

static BUILDIN(removemapflag)
{
	int16 m,i;
	const char *str;

	str=script_getstr(st,2);
	i=script_getnum(st,3);

	m = map->mapname2mapid(str);
	if(m >= 0) {
		switch(i) {
			case MF_NOMEMO:             map->list[m].flag.nomemo = 0; break;
			case MF_NOTELEPORT:         map->list[m].flag.noteleport = 0; break;
			case MF_NOSAVE:             map->list[m].flag.nosave = 0; break;
			case MF_NOBRANCH:           map->list[m].flag.nobranch = 0; break;
			case MF_NOPENALTY:          map->list[m].flag.noexppenalty = 0; map->list[m].flag.nozenypenalty = 0; break;
			case MF_NOZENYPENALTY:      map->list[m].flag.nozenypenalty = 0; break;
			case MF_PVP: {
				struct block_list bl;
				memset(&bl, 0, sizeof(bl));
				bl.type = BL_NUL;
				bl.m = m;
				map->list[m].flag.pvp = 0;
				clif->map_property_mapall(m, MAPPROPERTY_NOTHING);
				clif->maptypeproperty2(&bl,ALL_SAMEMAP);
			}
				break;
			case MF_PVP_NOPARTY:        map->list[m].flag.pvp_noparty = 0; break;
			case MF_PVP_NOGUILD:        map->list[m].flag.pvp_noguild = 0; break;
			case MF_GVG: {
				struct block_list bl;
				memset(&bl, 0, sizeof(bl));
				bl.type = BL_NUL;
				bl.m = m;
				map->list[m].flag.gvg = 0;
				clif->map_property_mapall(m, MAPPROPERTY_NOTHING);
				clif->maptypeproperty2(&bl,ALL_SAMEMAP);
			}
				break;
			case MF_GVG_NOPARTY:        map->list[m].flag.gvg_noparty = 0; break;
			case MF_NOTRADE:            map->list[m].flag.notrade = 0; break;
			case MF_NOSKILL:            map->list[m].flag.noskill = 0; break;
			case MF_NOWARP:             map->list[m].flag.nowarp = 0; break;
			case MF_PARTYLOCK:          map->list[m].flag.partylock = 0; break;
			case MF_NOICEWALL:          map->list[m].flag.noicewall = 0; break;
			case MF_SNOW:               map->list[m].flag.snow = 0; break;
			case MF_FOG:                map->list[m].flag.fog = 0; break;
			case MF_SAKURA:             map->list[m].flag.sakura = 0; break;
			case MF_LEAVES:             map->list[m].flag.leaves = 0; break;
			case MF_CLOUDS:             map->list[m].flag.clouds = 0; break;
			case MF_CLOUDS2:            map->list[m].flag.clouds2 = 0; break;
			case MF_FIREWORKS:          map->list[m].flag.fireworks = 0; break;
			case MF_GVG_CASTLE:         map->list[m].flag.gvg_castle = 0; break;
			case MF_GVG_DUNGEON:        map->list[m].flag.gvg_dungeon = 0; break;
			case MF_NIGHTENABLED:       map->list[m].flag.nightenabled = 0; break;
			case MF_NOBASEEXP:          map->list[m].flag.nobaseexp = 0; break;
			case MF_NOJOBEXP:           map->list[m].flag.nojobexp = 0; break;
			case MF_NOMOBLOOT:          map->list[m].flag.nomobloot = 0; break;
			case MF_NOMVPLOOT:          map->list[m].flag.nomvploot = 0; break;
			case MF_NORETURN:           map->list[m].flag.noreturn = 0; break;
			case MF_NOWARPTO:           map->list[m].flag.nowarpto = 0; break;
			case MF_NIGHTMAREDROP:      map->list[m].flag.pvp_nightmaredrop = 0; break;
			case MF_ZONE:
				map->zone_change2(m, map->list[m].prev_zone);
				break;
			case MF_NOCOMMAND:          map->list[m].nocommand = 0; break;
			case MF_NODROP:             map->list[m].flag.nodrop = 0; break;
			case MF_JEXP:               map->list[m].jexp = 0; break;
			case MF_BEXP:               map->list[m].bexp = 0; break;
			case MF_NOVENDING:          map->list[m].flag.novending = 0; break;
			case MF_LOADEVENT:          map->list[m].flag.loadevent = 0; break;
			case MF_NOCHAT:             map->list[m].flag.nochat = 0; break;
			case MF_NOEXPPENALTY:       map->list[m].flag.noexppenalty  = 0; break;
			case MF_GUILDLOCK:          map->list[m].flag.guildlock = 0; break;
			case MF_TOWN:               map->list[m].flag.town = 0; break;
			case MF_AUTOTRADE:          map->list[m].flag.autotrade = 0; break;
			case MF_ALLOWKS:            map->list[m].flag.allowks = 0; break;
			case MF_MONSTER_NOTELEPORT: map->list[m].flag.monster_noteleport = 0; break;
			case MF_PVP_NOCALCRANK:     map->list[m].flag.pvp_nocalcrank = 0; break;
			case MF_BATTLEGROUND:       map->list[m].flag.battleground = 0; break;
			case MF_RESET:              map->list[m].flag.reset = 0; break;
			case MF_NOTOMB:             map->list[m].flag.notomb = 0; break;
			case MF_NOCASHSHOP:         map->list[m].flag.nocashshop = 0; break;
			case MF_NOAUTOLOOT:         map->list[m].flag.noautoloot = 0; break;
			case MF_NOVIEWID:           map->list[m].flag.noviewid = EQP_NONE; break;
			case MF_NOSTORAGE:          map->list[m].flag.nostorage = 0; break;
			case MF_NOGSTORAGE:         map->list[m].flag.nogstorage = 0; break;
		}
	}

	return true;
}

static BUILDIN(pvpon)
{
	int16 m;
	const char *str;
	struct map_session_data *sd = NULL;
	struct s_mapiterator* iter;
	struct block_list bl;

	memset(&bl, 0, sizeof(bl));
	str = script_getstr(st,2);
	m = map->mapname2mapid(str);
	if( m < 0 || map->list[m].flag.pvp )
		return true; // nothing to do

	if( !strdb_exists(map->zone_db,MAP_ZONE_PVP_NAME) ) {
		ShowError("buildin_pvpon: zone_db missing '%s'\n",MAP_ZONE_PVP_NAME);
		return true;
	}

	map->zone_change2(m, strdb_get(map->zone_db, MAP_ZONE_PVP_NAME));
	map->list[m].flag.pvp = 1;
	clif->map_property_mapall(m, MAPPROPERTY_FREEPVPZONE);
	bl.type = BL_NUL;
	bl.m = m;
	clif->maptypeproperty2(&bl,ALL_SAMEMAP);

	if(battle_config.pk_mode) // disable ranking functions if pk_mode is on [Valaris]
		return true;

	iter = mapit_getallusers();
	for (sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); sd = BL_UCAST(BL_PC, mapit->next(iter))) {
		if( sd->bl.m != m || sd->pvp_timer != INVALID_TIMER )
			continue; // not applicable

		if (!map->list[m].flag.pvp_nocalcrank)
			sd->pvp_timer = timer->add(timer->gettick()+200,pc->calc_pvprank_timer,sd->bl.id,0);
		sd->pvp_rank = 0;
		sd->pvp_lastusers = 0;
		sd->pvp_point = 5;
		sd->pvp_won = 0;
		sd->pvp_lost = 0;
	}
	mapit->free(iter);

	return true;
}

static int buildin_pvpoff_sub(struct block_list *bl, va_list ap)
{
	struct map_session_data *sd = NULL;

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_PC);
	sd = BL_UCAST(BL_PC, bl);

	clif->pvpset(sd, 0, 0, 2);
	if (sd->pvp_timer != INVALID_TIMER) {
		timer->delete(sd->pvp_timer, pc->calc_pvprank_timer);
		sd->pvp_timer = INVALID_TIMER;
	}
	return 0;
}

static BUILDIN(pvpoff)
{
	int16 m;
	const char *str;
	struct block_list bl;

	memset(&bl, 0, sizeof(bl));
	str=script_getstr(st,2);
	m = map->mapname2mapid(str);
	if(m < 0 || !map->list[m].flag.pvp)
		return true; //fixed Lupus

	map->zone_change2(m, map->list[m].prev_zone);
	map->list[m].flag.pvp = 0;
	clif->map_property_mapall(m, MAPPROPERTY_NOTHING);
	bl.type = BL_NUL;
	bl.m = m;
	clif->maptypeproperty2(&bl,ALL_SAMEMAP);

	if(battle_config.pk_mode) // disable ranking options if pk_mode is on [Valaris]
		return true;

	map->foreachinmap(script->buildin_pvpoff_sub, m, BL_PC);
	return true;
}

static BUILDIN(gvgon)
{
	int16 m;
	const char *str;

	str=script_getstr(st,2);
	m = map->mapname2mapid(str);
	if(m >= 0 && !map->list[m].flag.gvg) {
		struct block_list bl;

		memset(&bl, 0, sizeof(bl));
		if( !strdb_exists(map->zone_db,MAP_ZONE_GVG_NAME) ) {
			ShowError("buildin_gvgon: zone_db missing '%s'\n",MAP_ZONE_GVG_NAME);
			return true;
		}

		map->zone_change2(m, strdb_get(map->zone_db, MAP_ZONE_GVG_NAME));
		map->list[m].flag.gvg = 1;
		clif->map_property_mapall(m, MAPPROPERTY_AGITZONE);
		bl.type = BL_NUL;
		bl.m = m;
		clif->maptypeproperty2(&bl,ALL_SAMEMAP);
	}

	return true;
}
static BUILDIN(gvgoff)
{
	int16 m;
	const char *str;

	str=script_getstr(st,2);
	m = map->mapname2mapid(str);
	if(m >= 0 && map->list[m].flag.gvg) {
		struct block_list bl;
		memset(&bl, 0, sizeof(bl));
		map->zone_change2(m, map->list[m].prev_zone);
		map->list[m].flag.gvg = 0;
		clif->map_property_mapall(m, MAPPROPERTY_NOTHING);
		bl.type = BL_NUL;
		bl.m = m;
		clif->maptypeproperty2(&bl,ALL_SAMEMAP);
	}

	return true;
}
/*==========================================
 * Shows an emoticon on top of the player/npc
 * emotion emotion#, <target: 0 - NPC, 1 - PC>, <NPC/PC name>
 *------------------------------------------*/
//Optional second parameter added by [Skotlex]
static BUILDIN(emotion)
{
	int type;
	int player=0;

	type=script_getnum(st,2);
	if(type < 0 || type > 100)
		return true;

	if( script_hasdata(st,3) )
		player=script_getnum(st,3);

	if (player != 0) {
		struct map_session_data *sd = NULL;
		if (script_hasdata(st,4))
			sd = script->nick2sd(st, script_getstr(st,4));
		else
			sd = script->rid2sd(st);
		if (sd != NULL)
			clif->emotion(&sd->bl,type);
	} else if( script_hasdata(st,4) ) {
		struct npc_data *nd = npc->name2id(script_getstr(st,4));
		if (nd != NULL)
			clif->emotion(&nd->bl,type);
	} else {
		clif->emotion(map->id2bl(st->oid),type);
	}
	return true;
}

static int buildin_maprespawnguildid_sub_pc(struct map_session_data *sd, va_list ap)
{
	int16 m=va_arg(ap,int);
	int g_id=va_arg(ap,int);
	int flag=va_arg(ap,int);

	if(!sd || sd->bl.m != m)
		return 0;
	if(
	    (sd->status.guild_id == g_id && flag&1) //Warp out owners
	 || (sd->status.guild_id != g_id && flag&2) //Warp out outsiders
	 || (sd->status.guild_id == 0)              // Warp out players not in guild [Valaris]
	  )
		pc->setpos(sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,CLR_TELEPORT);
	return 1;
}

static int buildin_maprespawnguildid_sub_mob(struct block_list *bl, va_list ap)
{
	struct mob_data *md = NULL;

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_MOB);
	md = BL_UCAST(BL_MOB, bl);

	if (md->guardian_data == NULL && md->class_ != MOBID_EMPELIUM)
		status_kill(bl);

	return 0;
}

static BUILDIN(maprespawnguildid)
{
	const char *mapname=script_getstr(st,2);
	int g_id=script_getnum(st,3);
	int flag=script_getnum(st,4);

	int16 m=map->mapname2mapid(mapname);

	if(m == -1)
		return true;

	//Catch ALL players (in case some are 'between maps' on execution time)
	map->foreachpc(script->buildin_maprespawnguildid_sub_pc,m,g_id,flag);
	if (flag&4) //Remove script mobs.
		map->foreachinmap(script->buildin_maprespawnguildid_sub_mob,m,BL_MOB);
	return true;
}

static BUILDIN(agitstart)
{
	if(map->agit_flag==1) return true;      // Agit already Start.
	map->agit_flag=1;
	guild->agit_start();
	return true;
}

static BUILDIN(agitend)
{
	if(map->agit_flag==0) return true;      // Agit already End.
	map->agit_flag=0;
	guild->agit_end();
	return true;
}

static BUILDIN(agitstart2)
{
	if(map->agit2_flag==1) return true;      // Agit2 already Start.
	map->agit2_flag=1;
	guild->agit2_start();
	return true;
}

static BUILDIN(agitend2)
{
	if(map->agit2_flag==0) return true;      // Agit2 already End.
	map->agit2_flag=0;
	guild->agit2_end();
	return true;
}

/*==========================================
 * Returns whether woe is on or off.
 *------------------------------------------*/
static BUILDIN(agitcheck)
{
	script_pushint(st,map->agit_flag);
	return true;
}

/*==========================================
 * Returns whether woese is on or off.
 *------------------------------------------*/
static BUILDIN(agitcheck2)
{
	script_pushint(st,map->agit2_flag);
	return true;
}

/// Sets the guild_id of this npc.
///
/// flagemblem <guild_id>;
static BUILDIN(flagemblem)
{
	struct npc_data *nd;
	int g_id = script_getnum(st,2);

	if(g_id < 0) return true;

	nd = map->id2nd(st->oid);
	if( nd == NULL ) {
		ShowError("script:flagemblem: npc %d not found\n", st->oid);
	} else if( nd->subtype != SCRIPT ) {
		ShowError("script:flagemblem: unexpected subtype %u for npc %d '%s'\n", nd->subtype, st->oid, nd->exname);
	} else {
		bool changed = ( nd->u.scr.guild_id != g_id )?true:false;
		nd->u.scr.guild_id = g_id;
		clif->guild_emblem_area(&nd->bl);
		/* guild flag caching */
		if( g_id ) /* adding a id */
			guild->flag_add(nd);
		else if( changed ) /* removing a flag */
			guild->flag_remove(nd);
	}
	return true;
}

static BUILDIN(getcastlename)
{
	const char* mapname = mapindex->getmapname(script_getstr(st,2),NULL);
	struct guild_castle* gc = guild->mapname2gc(mapname);
	const char* name = (gc) ? gc->castle_name : "";
	script_pushstrcopy(st,name);
	return true;
}

static BUILDIN(getcastledata)
{
	const char *mapname = mapindex->getmapname(script_getstr(st,2),NULL);
	int index = script_getnum(st,3);
	struct guild_castle *gc = guild->mapname2gc(mapname);

	if (gc == NULL) {
		script_pushint(st,0);
		ShowWarning("buildin_setcastledata: guild castle for map '%s' not found\n", mapname);
		return false;
	}

	switch (index) {
		case 1:
			script_pushint(st,gc->guild_id); break;
		case 2:
			script_pushint(st,gc->economy); break;
		case 3:
			script_pushint(st,gc->defense); break;
		case 4:
			script_pushint(st,gc->triggerE); break;
		case 5:
			script_pushint(st,gc->triggerD); break;
		case 6:
			script_pushint(st,gc->nextTime); break;
		case 7:
			script_pushint(st,gc->payTime); break;
		case 8:
			script_pushint(st,gc->createTime); break;
		case 9:
			script_pushint(st,gc->visibleC); break;
		default:
			if (index > 9 && index <= 9+MAX_GUARDIANS) {
				script_pushint(st,gc->guardian[index-10].visible);
				break;
			}
			script_pushint(st,0);
			ShowWarning("buildin_setcastledata: index = '%d' is out of allowed range\n", index);
			return false;
	}
	return true;
}

static BUILDIN(setcastledata)
{
	const char *mapname = mapindex->getmapname(script_getstr(st,2),NULL);
	int index = script_getnum(st,3);
	int value = script_getnum(st,4);
	struct guild_castle *gc = guild->mapname2gc(mapname);

	if (gc == NULL) {
		ShowWarning("buildin_setcastledata: guild castle for map '%s' not found\n", mapname);
		return false;
	}

	if (index <= 0 || index > 9+MAX_GUARDIANS) {
		ShowWarning("buildin_setcastledata: index = '%d' is out of allowed range\n", index);
		return false;
	}

	guild->castledatasave(gc->castle_id, index, value);
	return true;
}

/* =====================================================================
 * ---------------------------------------------------------------------*/
static BUILDIN(requestguildinfo)
{
	int guild_id=script_getnum(st,2);
	const char *event=NULL;

	if( script_hasdata(st,3) ) {
		event=script_getstr(st,3);
		script->check_event(st, event);
	}

	if(guild_id>0)
		guild->npc_request_info(guild_id,event);
	return true;
}

/// Returns the number of cards that have been compounded onto the specified equipped item.
/// getequipcardcnt(<equipment slot>);
static BUILDIN(getequipcardcnt)
{
	int i=-1,j,num;
	struct map_session_data *sd;
	int count;

	num=script_getnum(st,2);
	sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	if (num > 0 && num <= ARRAYLENGTH(script->equip))
		i=pc->checkequip(sd,script->equip[num-1]);

	if (i < 0 || !sd->inventory_data[i]) {
		script_pushint(st,0);
		return true;
	}

	if(itemdb_isspecial(sd->status.inventory[i].card[0]))
	{
		script_pushint(st,0);
		return true;
	}

	count = 0;
	for( j = 0; j < sd->inventory_data[i]->slot; j++ )
		if( sd->status.inventory[i].card[j] && itemdb_type(sd->status.inventory[i].card[j]) == IT_CARD )
			count++;

	script_pushint(st,count);
	return true;
}

/// Removes all cards from the item found in the specified equipment slot of the invoking character,
/// and give them to the character. If any cards were removed in this manner, it will also show a success effect.
/// successremovecards(<slot>);
static BUILDIN(successremovecards)
{
	int i = -1, c, cardflag = 0;

	struct map_session_data *sd = script->rid2sd(st);
	int num = script_getnum(st, 2);

	if (sd == NULL)
		return true;

	if (num > 0 && num <= ARRAYLENGTH(script->equip))
		i = pc->checkequip(sd,script->equip[num - 1]);

	if (i < 0 || sd->inventory_data[i] == NULL)
		return true;

	if (itemdb_isspecial(sd->status.inventory[i].card[0]))
		return true;

	for (c = sd->inventory_data[i]->slot - 1; c >= 0; --c) {
		if (sd->status.inventory[i].card[c] > 0 && itemdb_type(sd->status.inventory[i].card[c]) == IT_CARD) {
			int flag;
			struct item item_tmp;

			memset(&item_tmp, 0, sizeof(item_tmp));

			cardflag = 1;
			item_tmp.nameid = sd->status.inventory[i].card[c];
			item_tmp.identify = 1;
			sd->status.inventory[i].card[c] = 0;

			if ((flag = pc->additem(sd, &item_tmp, 1, LOG_TYPE_SCRIPT))) {
				clif->additem(sd, 0, 0, flag);
				map->addflooritem(&sd->bl, &item_tmp, 1, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0, false);
			}
		}
	}

	if (cardflag == 1) {
		pc->unequipitem(sd, i, PCUNEQUIPITEM_FORCE);
		clif->delitem(sd, i, 1, DELITEM_MATERIALCHANGE);
		clif->additem(sd, i, 1, 0);
		pc->equipitem(sd, i, sd->status.inventory[i].equip);
		clif->misceffect(&sd->bl,3);
	}
	return true;
}

/// Removes all cards from the item found in the specified equipment slot of the invoking character.
/// failedremovecards(<slot>, <type>);
/// <type>=0 : will destroy both the item and the cards.
/// <type>=1 : will keep the item, but destroy the cards.
/// <type>=2 : will keep the cards, but destroy the item.
/// <type>=3 : will just display the failure effect.
static BUILDIN(failedremovecards)
{
	int i = -1, c, cardflag = 0;
	int num = script_getnum(st, 2);
	int typefail = script_getnum(st, 3);

	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	if (num > 0 && num <= ARRAYLENGTH(script->equip))
		i = pc->checkequip(sd, script->equip[num - 1]);

	if (i < 0 || sd->inventory_data[i] == NULL)
		return true;

	if (itemdb_isspecial(sd->status.inventory[i].card[0]))
		return true;

	for (c = sd->inventory_data[i]->slot - 1; c >= 0; --c) {
		if (sd->status.inventory[i].card[c] > 0 && itemdb_type(sd->status.inventory[i].card[c]) == IT_CARD) {
			cardflag = 1;

			if (typefail == 1)
				sd->status.inventory[i].card[c] = 0;

			if (typefail == 2) { // add cards to inventory, clear
				int flag;
				struct item item_tmp;

				memset(&item_tmp, 0, sizeof(item_tmp));

				item_tmp.nameid = sd->status.inventory[i].card[c];
				item_tmp.identify = 1;

				if ((flag = pc->additem(sd, &item_tmp, 1, LOG_TYPE_SCRIPT))) {
					clif->additem(sd, 0, 0, flag);
					map->addflooritem(&sd->bl, &item_tmp, 1, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0, false);
				}
			}
		}
	}

	if (cardflag == 1) {
		if (typefail == 0 || typefail == 2) { // destroy the item
			pc->delitem(sd, i, 1, 0, DELITEM_FAILREFINE, LOG_TYPE_SCRIPT);
		} else if (typefail == 1) {
			pc->unequipitem(sd, i, PCUNEQUIPITEM_FORCE);
			clif->delitem(sd, i, 1, DELITEM_MATERIALCHANGE);
			clif->additem(sd, i, 1, 0);
			pc->equipitem(sd, i, sd->status.inventory[i].equip);
		}
	}
	clif->misceffect(&sd->bl, 2);

	return true;
}

/* ================================================================
 * mapwarp "<from map>","<to map>",<x>,<y>,<type>,<ID for Type>;
 * type: 0=everyone, 1=guild, 2=party; [Reddozen]
 * improved by [Lance]
 * ================================================================*/
// Added by RoVeRT
static BUILDIN(mapwarp)
{
	int x,y,m,check_val=0,check_ID=0,i=0;
	struct guild *g = NULL;
	struct party_data *p = NULL;
	const char *str;
	const char *mapname;
	unsigned int index;
	mapname=script_getstr(st,2);
	str=script_getstr(st,3);
	x=script_getnum(st,4);
	y=script_getnum(st,5);
	if(script_hasdata(st,7)) {
		check_val=script_getnum(st,6);
		check_ID=script_getnum(st,7);
	}

	if((m=map->mapname2mapid(mapname))< 0)
		return true;

	if(!(index=script->mapindexname2id(st,str)))
		return true;

	switch(check_val) {
		case 1:
			g = guild->search(check_ID);
			if (g) {
				for( i=0; i < g->max_member; i++)
				{
					if(g->member[i].sd && g->member[i].sd->bl.m==m) {
						pc->setpos(g->member[i].sd,index,x,y,CLR_TELEPORT);
					}
				}
			}
			break;
		case 2:
			p = party->search(check_ID);
			if(p) {
				for(i=0;i<MAX_PARTY; i++) {
					if(p->data[i].sd && p->data[i].sd->bl.m == m) {
						pc->setpos(p->data[i].sd,index,x,y,CLR_TELEPORT);
					}
				}
			}
			break;
		default:
			map->foreachinmap(script->buildin_areawarp_sub,m,BL_PC,index,x,y,0,0);
			break;
	}

	return true;
}

// Added by RoVeRT
static int buildin_mobcount_sub(struct block_list *bl, va_list ap)
{
	char *event = va_arg(ap,char *);
	const struct mob_data *md = NULL;

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_MOB);
	md = BL_UCCAST(BL_MOB, bl);

	if( md->status.hp > 0 && (!event || strcmp(event,md->npc_event) == 0) )
		return 1;
	return 0;
}

// Added by RoVeRT
static BUILDIN(mobcount)
{
	const char *mapname,*event;
	int16 m;
	mapname=script_getstr(st,2);
	event=script_getstr(st,3);

	if( strcmp(event, "all") == 0 )
		event = NULL;
	else
		script->check_event(st, event);

	if( strcmp(mapname, "this") == 0 ) {
		struct map_session_data *sd = script->rid2sd(st);

		if (sd == NULL)
			return true;

		m = sd->bl.m;
	} else if( (m = map->mapname2mapid(mapname)) < 0 ) {
		script_pushint(st,-1);
		return true;
	}

	if( map->list[m].flag.src4instance && map->list[m].instance_id == -1 && st->instance_id >= 0 && (m = instance->mapid2imapid(m, st->instance_id)) < 0 ) {
		script_pushint(st,-1);
		return true;
	}

	script_pushint(st,map->foreachinmap(script->buildin_mobcount_sub, m, BL_MOB, event));

	return true;
}

static BUILDIN(marriage)
{
	const char *partner=script_getstr(st,2);
	struct map_session_data *sd = script->rid2sd(st);
	struct map_session_data *p_sd = script->nick2sd(st, partner);

	if (sd == NULL || p_sd == NULL || pc->marriage(sd,p_sd) < 0) {
		script_pushint(st,0);
		return true;
	}
	script_pushint(st,1);
	return true;
}
static BUILDIN(wedding_effect)
{
	struct map_session_data *sd = script->rid2sd(st);
	struct block_list *bl;

	if (sd == NULL)
		return true; //bl=map->id2bl(st->oid);

	bl=&sd->bl;
	clif->wedding_effect(bl);
	return true;
}
static BUILDIN(divorce)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL || pc->divorce(sd) < 0) {
		script_pushint(st,0);
		return true;
	}
	script_pushint(st,1);
	return true;
}

static BUILDIN(ispartneron)
{
	struct map_session_data *sd = script->rid2sd(st);

	if (sd==NULL || !pc->ismarried(sd)
	 || map->charid2sd(sd->status.partner_id) == NULL) {
		script_pushint(st,0);
		return true;
	}

	script_pushint(st,1);
	return true;
}

static BUILDIN(getpartnerid)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	script_pushint(st,sd->status.partner_id);
	return true;
}

static BUILDIN(getchildid)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	script_pushint(st,sd->status.child);
	return true;
}

static BUILDIN(getmotherid)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	script_pushint(st,sd->status.mother);
	return true;
}

static BUILDIN(getfatherid)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	script_pushint(st,sd->status.father);
	return true;
}

static BUILDIN(warppartner)
{
	int x,y;
	unsigned short map_index;
	const char *str;
	struct map_session_data *sd = script->rid2sd(st);
	struct map_session_data *p_sd = NULL;

	if (sd == NULL || !pc->ismarried(sd)
	 || (p_sd = script->charid2sd(st, sd->status.partner_id)) == NULL) {
		script_pushint(st,0);
		return true;
	}

	str=script_getstr(st,2);
	x=script_getnum(st,3);
	y=script_getnum(st,4);

	map_index = script->mapindexname2id(st,str);
	if (map_index) {
		pc->setpos(p_sd,map_index,x,y,CLR_OUTSIGHT);
		script_pushint(st,1);
	} else
		script_pushint(st,0);
	return true;
}

/*================================================
 * Script for Displaying MOB Information [Valaris]
 *------------------------------------------------*/
static BUILDIN(strmobinfo)
{

	int num=script_getnum(st,2);
	int class_=script_getnum(st,3);

	if(!mob->db_checkid(class_))
	{
		if (num < 3) //requested a string
			script_pushconststr(st,"");
		else
			script_pushint(st,0);
		return true;
	}

	switch (num) {
		case 1: script_pushstrcopy(st,mob->db(class_)->name); break;
		case 2: script_pushstrcopy(st,mob->db(class_)->jname); break;
		case 3: script_pushint(st,mob->db(class_)->lv); break;
		case 4: script_pushint(st,mob->db(class_)->status.max_hp); break;
		case 5: script_pushint(st,mob->db(class_)->status.max_sp); break;
		case 6: script_pushint(st,mob->db(class_)->base_exp); break;
		case 7: script_pushint(st,mob->db(class_)->job_exp); break;
		default:
			script_pushint(st,0);
			break;
	}
	return true;
}

/**
 * Summons a castle guardian mob.
 *
 * @code{.herc}
 *	guardian("<map name>", <x>, <y>, "<name to show>", <mob id>{, <guardian index>});
 *	guardian("<map name>", <x>, <y>, "<name to show>", <mob id>{, "<event label>"{, <guardian index>}});
 * @endcode
 *
 * @author Valaris
 *
 **/
static BUILDIN(guardian)
{
	bool has_index = false;
	int guardian = 0;
	const char *event = "";

	if (script_hasdata(st, 8)) { /// "<event label>", <guardian index>
		event = script_getstr(st, 7);
		script->check_event(st, event);
		guardian = script_getnum(st, 8);
		has_index = true;
	} else if (script_hasdata(st, 7)) {
		struct script_data *data = script_getdata(st, 7);

		script->get_val(st, data); /// Dereference if it's a variable.

		if (data_isstring(data)) { /// "<event label>"
			event = script_getstr(st, 7);
			script->check_event(st, event);
		} else if (data_isint(data)) { /// <guardian index>
			guardian = script_getnum(st, 7);
			has_index = true;
		} else {
			ShowError("script:guardian: Invalid data type for argument #6!\n");
			script->reportdata(data);
			return false;
		}
	}

	const char *mapname = script_getstr(st, 2);
	const char *name = script_getstr(st, 5);
	const int x = script_getnum(st, 3);
	const int y = script_getnum(st, 4);
	const int mob_id = script_getnum(st, 6);

	script_pushint(st, mob->spawn_guardian(mapname, x, y, name, mob_id, event, guardian, has_index, st->oid));
	return true;
}
/*==========================================
 * Invisible Walls [Zephyrus]
 *------------------------------------------*/
static BUILDIN(setwall)
{
	const char *mapname, *name;
	int x, y, m, size, dir;
	bool shootable;

	mapname   = script_getstr(st,2);
	x         = script_getnum(st,3);
	y         = script_getnum(st,4);
	size      = script_getnum(st,5);
	dir       = script_getnum(st,6);
	shootable = script_getnum(st,7);
	name      = script_getstr(st,8);

	if( (m = map->mapname2mapid(mapname)) < 0 )
		return true; // Invalid Map

	map->iwall_set(m, x, y, size, dir, shootable, name);
	return true;
}

static BUILDIN(delwall)
{
	const char *name = script_getstr(st,2);

	if (!map->iwall_remove(name)) {
		ShowWarning("buildin_delwall: Non-existent '%s' provided.\n", name);
		return false;
	}

	return true;
}

/// Retrieves various information about the specified guardian.
///
/// guardianinfo("<map_name>", <index>, <type>) -> <value>
/// type: 0 - whether it is deployed or not
///       1 - maximum hp
///       2 - current hp
///
static BUILDIN(guardianinfo)
{
	const char* mapname = mapindex->getmapname(script_getstr(st,2),NULL);
	int id = script_getnum(st,3);
	int type = script_getnum(st,4);

	struct guild_castle* gc = guild->mapname2gc(mapname);
	struct mob_data* gd;

	if( gc == NULL || id < 0 || id >= MAX_GUARDIANS ) {
		script_pushint(st,-1);
		return true;
	}

	if( type == 0 )
		script_pushint(st, gc->guardian[id].visible);
	else if( !gc->guardian[id].visible )
		script_pushint(st,-1);
	else if( (gd = map->id2md(gc->guardian[id].id)) == NULL )
		script_pushint(st,-1);
	else if( type == 1 )
		script_pushint(st,gd->status.max_hp);
	else if( type == 2 )
		script_pushint(st,gd->status.hp);
	else
		script_pushint(st,-1);

	return true;
}

/*==========================================
 * Get the item name by item_id or null
 *------------------------------------------*/
static BUILDIN(getitemname)
{
	int item_id=0;
	struct item_data *i_data;
	char *item_name;

	if( script_isstringtype(st, 2) ) {
		const char *name = script_getstr(st, 2);
		struct item_data *item_data = itemdb->search_name(name);
		if( item_data )
			item_id=item_data->nameid;
	} else {
		item_id = script_getnum(st, 2);
	}

	i_data = itemdb->exists(item_id);
	if (i_data == NULL) {
		script_pushconststr(st,"null");
		return true;
	}
	item_name=(char *)aMalloc(ITEM_NAME_LENGTH*sizeof(char));

	memcpy(item_name, i_data->jname, ITEM_NAME_LENGTH);
	script_pushstr(st,item_name);
	return true;
}
/*==========================================
 * Returns number of slots an item has. [Skotlex]
 *------------------------------------------*/
static BUILDIN(getitemslots)
{
	int item_id;
	struct item_data *i_data;

	item_id=script_getnum(st,2);

	i_data = itemdb->exists(item_id);

	if (i_data)
		script_pushint(st,i_data->slot);
	else
		script_pushint(st,-1);
	return true;
}

/**
 * Returns various information about an item.
 *
 * @code{.herc}
 *	getiteminfo(<item ID>, <type>);
 *	getiteminfo("<item name>", <type>);
 * @endcode
 *
 **/
static BUILDIN(getiteminfo)
{
	struct item_data *it;

	if (script_isstringtype(st, 2)) { /// Item name.
		const char *name = script_getstr(st, 2);
		it = itemdb->search_name(name);
	} else { /// Item ID.
		it = itemdb->exists(script_getnum(st, 2));
	}

	if (it == NULL) {
		script_pushint(st, -1);
		return true;
	}

	int type = script_getnum(st, 3);

	switch (type) {
	case ITEMINFO_BUYPRICE:
		script_pushint(st, it->value_buy);
		break;
	case ITEMINFO_SELLPRICE:
		script_pushint(st, it->value_sell);
		break;
	case ITEMINFO_TYPE:
		script_pushint(st, it->type);
		break;
	case ITEMINFO_MAXCHANCE:
		script_pushint(st, it->maxchance);
		break;
	case ITEMINFO_SEX:
		script_pushint(st, it->sex);
		break;
	case ITEMINFO_LOC:
		script_pushint(st, it->equip);
		break;
	case ITEMINFO_WEIGHT:
		script_pushint(st, it->weight);
		break;
	case ITEMINFO_ATK:
		script_pushint(st, it->atk);
		break;
	case ITEMINFO_DEF:
		script_pushint(st, it->def);
		break;
	case ITEMINFO_RANGE:
		script_pushint(st, it->range);
		break;
	case ITEMINFO_SLOTS:
		script_pushint(st, it->slot);
		break;
	case ITEMINFO_SUBTYPE:
		script_pushint(st, it->subtype);
		break;
	case ITEMINFO_ELV:
		script_pushint(st, it->elv);
		break;
	case ITEMINFO_WLV:
		script_pushint(st, it->wlv);
		break;
	case ITEMINFO_VIEWID:
		script_pushint(st, it->view_id);
		break;
	case ITEMINFO_MATK:
		script_pushint(st, it->matk);
		break;
	case ITEMINFO_VIEWSPRITE:
		script_pushint(st, it->view_sprite);
		break;
	case ITEMINFO_TRADE:
		script_pushint(st, it->flag.trade_restriction);
		break;
	case ITEMINFO_ELV_MAX:
		script_pushint(st, it->elvmax);
		break;
	case ITEMINFO_DROPEFFECT_MODE:
		script_pushint(st, it->dropeffectmode);
		break;
	case ITEMINFO_DELAY:
		script_pushint(st, it->delay);
		break;
	case ITEMINFO_CLASS_BASE_1:
		script_pushint(st, it->class_base[0]);
		break;
	case ITEMINFO_CLASS_BASE_2:
		script_pushint(st, it->class_base[1]);
		break;
	case ITEMINFO_CLASS_BASE_3:
		script_pushint(st, it->class_base[2]);
		break;
	case ITEMINFO_CLASS_UPPER:
		script_pushint(st, it->class_upper);
		break;
	case ITEMINFO_FLAG_NO_REFINE:
		script_pushint(st, it->flag.no_refine);
		break;
	case ITEMINFO_FLAG_DELAY_CONSUME:
		script_pushint(st, it->flag.delay_consume);
		break;
	case ITEMINFO_FLAG_AUTOEQUIP:
		script_pushint(st, it->flag.autoequip);
		break;
	case ITEMINFO_FLAG_AUTO_FAVORITE:
		script_pushint(st, it->flag.auto_favorite);
		break;
	case ITEMINFO_FLAG_BUYINGSTORE:
		script_pushint(st, it->flag.buyingstore);
		break;
	case ITEMINFO_FLAG_BINDONEQUIP:
		script_pushint(st, it->flag.bindonequip);
		break;
	case ITEMINFO_FLAG_KEEPAFTERUSE:
		script_pushint(st, it->flag.keepafteruse);
		break;
	case ITEMINFO_FLAG_FORCE_SERIAL:
		script_pushint(st, it->flag.force_serial);
		break;
	case ITEMINFO_FLAG_NO_OPTIONS:
		script_pushint(st, it->flag.no_options);
		break;
	case ITEMINFO_FLAG_DROP_ANNOUNCE:
		script_pushint(st, it->flag.drop_announce);
		break;
	case ITEMINFO_FLAG_SHOWDROPEFFECT:
		script_pushint(st, it->flag.showdropeffect);
		break;
	case ITEMINFO_STACK_AMOUNT:
		script_pushint(st, it->stack.amount);
		break;
	case ITEMINFO_STACK_FLAG: {
		int stack_flag = 0;

		if (it->stack.inventory != 0)
			stack_flag |= 1;

		if (it->stack.cart != 0)
			stack_flag |= 2;

		if (it->stack.storage != 0)
			stack_flag |= 4;

		if (it->stack.guildstorage != 0)
			stack_flag |= 8;

		script_pushint(st, stack_flag);
		break;
	}
	case ITEMINFO_ITEM_USAGE_FLAG:
		script_pushint(st, it->item_usage.flag);
		break;
	case ITEMINFO_ITEM_USAGE_OVERRIDE:
		script_pushint(st, it->item_usage.override);
		break;
	case ITEMINFO_GM_LV_TRADE_OVERRIDE:
		script_pushint(st, it->gm_lv_trade_override);
		break;
	case ITEMINFO_ID:
		script_pushint(st, it->nameid);
		break;
	case ITEMINFO_AEGISNAME:
		script_pushstrcopy(st, it->name);
		break;
	case ITEMINFO_NAME:
		script_pushstrcopy(st, it->jname);
		break;
	default:
		ShowError("buildin_getiteminfo: Invalid item info type %d.\n", type);
		script_pushint(st, -1);
		return false;
	}

	return true;
}

/**
 * Returns the value of the current equipment being parsed using static variables -
 * current_equip_item_index and current_equip_option_index.
 * !!Designed to be used with item_options.conf only!!
 * *getequippedoptioninfo(<info_type>);
 *
 * @param (int) Types -
 *        IT_OPT_INDEX     ID of the item option.
 *        IT_OPT_VALUE     Amount of the bonus to be added.
 * @return value of the type or -1.
 */
static BUILDIN(getequippedoptioninfo)
{
	int val = 0, type = script_getnum(st, 2);
	struct map_session_data *sd = NULL;

	if ((sd = script->rid2sd(st)) == NULL || status->current_equip_item_index == -1 || status->current_equip_option_index == -1
		|| !sd->status.inventory[status->current_equip_item_index].option[status->current_equip_option_index].index) {
		script_pushint(st, -1);
		return false;
	}

	switch (type) {
	case IT_OPT_INDEX:
		val = sd->status.inventory[status->current_equip_item_index].option[status->current_equip_option_index].index;
		break;
	case IT_OPT_VALUE:
		val = sd->status.inventory[status->current_equip_item_index].option[status->current_equip_option_index].value;
		break;
	default:
		ShowError("buildin_getequippedoptioninfo: Invalid option data type %d (Max %d).\n", type, IT_OPT_MAX-1);
		script_pushint(st, -1);
		return false;
	}

	script_pushint(st, val);

	return true;
}

/**
 * Gets the option information of an equipment.
 * *getequipoption(<equip_index>,<slot>,<type>);
 *
 * @param equip_index as the Index of the Equipment.
 * @param slot        as the slot# of the Item Option (1 to MAX_ITEM_OPTIONS)
 * @param type        IT_OPT_INDEX or IT_OPT_VALUE.
 * @return (int) value or -1 on failure.
 */
static BUILDIN(getequipoption)
{
	int val = 0, equip_index = script_getnum(st, 2);
	int slot = script_getnum(st, 3);
	int opt_type = script_getnum(st, 4);
	int i = -1;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL) {
		script_pushint(st, -1);
		ShowError("buildin_getequipoption: Player not attached!\n");
		return false;
	}

	if (slot <= 0 || slot > MAX_ITEM_OPTIONS) {
		script_pushint(st, -1);
		ShowError("buildin_getequipoption: Invalid option slot %d (Min: 1, Max: %d) provided.\n", slot, MAX_ITEM_OPTIONS);
		return false;
	}

	if (equip_index > 0 && equip_index <= ARRAYLENGTH(script->equip)) {
		if ((i = pc->checkequip(sd, script->equip[equip_index - 1])) == -1) {
			ShowError("buildin_getequipoption: No equipment is equipped in the given index %d.\n", equip_index);
			script_pushint(st, -1);
			return false;
		}
	} else {
		ShowError("buildin_getequipoption: Invalid equipment index %d provided.\n", equip_index);
		script_pushint(st, 0);
		return false;
	}

	if (sd->status.inventory[i].nameid != 0) {
		switch (opt_type) {
		case IT_OPT_INDEX:
			val = sd->status.inventory[i].option[slot-1].index;
			break;
		case IT_OPT_VALUE:
			val = sd->status.inventory[i].option[slot-1].value;
			break;
		default:
			ShowError("buildin_getequipoption: Invalid option data type %d provided.\n", opt_type);
			script_pushint(st, -1);
			break;
		}
	}

	script_pushint(st, val);

	return true;
}

/**
 * Set an equipment's option value.
 * *setequipoption(<equip_index>,<slot>,<opt_index>,<value>);
 *
 * @param equip_index     as the inventory index of the equipment.
 * @param slot            as the slot of the item option (1 to MAX_ITEM_OPTIONS)
 * @param opt_index       as the index of the option available as "Id" in db/item_options.conf.
 * @param value           as the value of the option type.
 *                        For IT_OPT_INDEX see "Name" in item_options.conf
 *                        For IT_OPT_VALUE, the value of the script bonus.
 * @return 0 on failure, 1 on success.
 */
static BUILDIN(setequipoption)
{
	int equip_index = script_getnum(st, 2);
	int slot = script_getnum(st, 3);
	int opt_index = script_getnum(st, 4);
	int value = script_getnum(st, 5);
	int i = -1;

	struct map_session_data *sd = script->rid2sd(st);
	struct itemdb_option *ito = NULL;

	if (sd == NULL) {
		script_pushint(st, 0);
		ShowError("buildin_setequipoption: Player not attached!\n");
		return false;
	}

	if (slot <= 0 || slot > MAX_ITEM_OPTIONS) {
		script_pushint(st, 0);
		ShowError("buildin_setequipoption: Invalid option index %d (Min: 1, Max: %d) provided.\n", slot, MAX_ITEM_OPTIONS);
		return false;
	}

	if (equip_index > 0 && equip_index <= ARRAYLENGTH(script->equip)) {
		if ((i = pc->checkequip(sd, script->equip[equip_index - 1])) == -1) {
			ShowError("buildin_setequipoption: No equipment is equipped in the given index %d.\n", equip_index);
			script_pushint(st, 0);
			return false;
		}
	} else {
		ShowError("buildin_setequipoption: Invalid equipment index %d provided.\n", equip_index);
		script_pushint(st, 0);
		return false;
	}

	if (sd->status.inventory[i].nameid != 0) {
		if (opt_index == 0) {
			// Remove the option
			sd->status.inventory[i].option[slot-1].index = 0;
			sd->status.inventory[i].option[slot-1].value = 0;
		} else {
			if ((ito = itemdb->option_exists(opt_index)) == NULL) {
				script_pushint(st, 0);
				ShowError("buildin_setequipotion: Option index %d does not exist!\n", opt_index);
				return false;
			} else if (value < -INT16_MAX || value > INT16_MAX) {
				script_pushint(st, 0);
				ShowError("buildin_setequipotion: Option value %d exceeds maximum limit (%d to %d) for type!\n", value, -INT16_MAX, INT16_MAX);
				return false;
			}
			/* Add Option Index */
			sd->status.inventory[i].option[slot-1].index = ito->index;
			/* Add Option Value */
			sd->status.inventory[i].option[slot-1].value = value;
		}

		/* Unequip and simulate deletion of the item. */
		pc->unequipitem(sd, i, PCUNEQUIPITEM_FORCE); // status calc will happen in pc->equipitem() below
		clif->refine(sd->fd, 0, i, sd->status.inventory[i].refine); // notify client of a refine.
		clif->delitem(sd, i, 1, DELITEM_MATERIALCHANGE); // notify client to simulate item deletion.
		/* Log deletion of the item. */
		logs->pick_pc(sd, LOG_TYPE_SCRIPT, -1, &sd->status.inventory[i],sd->inventory_data[i]);
		/* Equip and simulate addition of the item. */
		clif->additem(sd, i, 1, 0); // notify client to simulate item addition.
		/* Log addition of the item. */
		logs->pick_pc(sd, LOG_TYPE_SCRIPT, 1, &sd->status.inventory[i], sd->inventory_data[i]);
		pc->equipitem(sd, i, sd->status.inventory[i].equip); // force equip the item at the original position.
		clif->misceffect(&sd->bl, 2); // show effect
	}

	script_pushint(st, 1);

	return true;

}

/*==========================================
 * Set some values of an item [Lupus]
 * Price, Weight, etc...
 *------------------------------------------*/
static BUILDIN(setiteminfo)
{
	// TODO: Validate data in a similar way as during database load
	int item_id = script_getnum(st, 2);
	int n = script_getnum(st, 3);
	int value = script_getnum(st,4);
	struct item_data *it = itemdb->exists(item_id);

	if (it == NULL) {
		script_pushint(st, -1);
		return true;
	}

	switch (n) {
	case ITEMINFO_BUYPRICE:
		it->value_buy = value;
		break;
	case ITEMINFO_SELLPRICE:
		it->value_sell = value;
		break;
	case ITEMINFO_TYPE:
		it->type = value;
		break;
	case ITEMINFO_MAXCHANCE:
		it->maxchance = value;
		break;
	case ITEMINFO_SEX:
		it->sex = value;
		break;
	case ITEMINFO_LOC:
		it->equip = value;
		break;
	case ITEMINFO_WEIGHT:
		it->weight = value;
		break;
	case ITEMINFO_ATK:
		it->atk = value;
		break;
	case ITEMINFO_DEF:
		it->def = value;
		break;
	case ITEMINFO_RANGE:
		it->range = value;
		break;
	case ITEMINFO_SLOTS:
		it->slot = value;
		break;
	case ITEMINFO_SUBTYPE:
		it->subtype = value;
		break;
	case ITEMINFO_ELV:
		it->elv = value;
		break;
	case ITEMINFO_WLV:
		it->wlv = value;
		break;
	case ITEMINFO_VIEWID:
		it->view_id = value;
		break;
	case ITEMINFO_MATK:
		it->matk = value;
		break;
	case ITEMINFO_VIEWSPRITE:
		it->view_sprite = value;
		break;
	case ITEMINFO_TRADE:
		it->flag.trade_restriction = value;
		break;
	case ITEMINFO_ELV_MAX:
		it->elvmax = cap_value(value, 0, MAX_LEVEL);
		break;
	case ITEMINFO_DROPEFFECT_MODE:
		it->dropeffectmode = value;
		break;
	case ITEMINFO_DELAY:
		it->delay = value;
		break;
	case ITEMINFO_CLASS_BASE_1:
		it->class_base[0] = value;
		break;
	case ITEMINFO_CLASS_BASE_2:
		it->class_base[1] = value;
		break;
	case ITEMINFO_CLASS_BASE_3:
		it->class_base[2] = value;
		break;
	case ITEMINFO_CLASS_UPPER:
		it->class_upper = value;
		break;
	case ITEMINFO_FLAG_NO_REFINE:
		it->flag.no_refine = cap_value(value, 0, MAX_REFINE);
		break;
	case ITEMINFO_FLAG_DELAY_CONSUME:
		it->flag.delay_consume = value;
		break;
	case ITEMINFO_FLAG_AUTOEQUIP:
		it->flag.autoequip = cap_value(value, 0, 1);
		break;
	case ITEMINFO_FLAG_AUTO_FAVORITE:
		it->flag.auto_favorite = cap_value(value, 0, 1);
		break;
	case ITEMINFO_FLAG_BUYINGSTORE:
		it->flag.buyingstore = cap_value(value, 0, 1);
		break;
	case ITEMINFO_FLAG_BINDONEQUIP:
		it->flag.bindonequip = cap_value(value, 0, 1);
		break;
	case ITEMINFO_FLAG_KEEPAFTERUSE:
		it->flag.keepafteruse = cap_value(value, 0, 1);
		break;
	case ITEMINFO_FLAG_FORCE_SERIAL:
		it->flag.force_serial = cap_value(value, 0, 1);
		break;
	case ITEMINFO_FLAG_NO_OPTIONS:
		it->flag.no_options = cap_value(value, 0, 1);
		break;
	case ITEMINFO_FLAG_DROP_ANNOUNCE:
		it->flag.drop_announce = cap_value(value, 0, 1);
		break;
	case ITEMINFO_FLAG_SHOWDROPEFFECT:
		it->flag.showdropeffect = cap_value(value, 0, 1);
		break;
	case ITEMINFO_STACK_AMOUNT:
		it->stack.amount = cap_value(value, 0, USHRT_MAX);
		break;
	case ITEMINFO_STACK_FLAG:
		it->stack.inventory = ((value & 1) != 0);
		it->stack.cart = ((value & 2) != 0);
		it->stack.storage = ((value & 4) != 0);
		it->stack.guildstorage = ((value & 8) != 0);
		break;
	case ITEMINFO_ITEM_USAGE_FLAG:
		it->item_usage.flag = cap_value(value, 0, 1);
		break;
	case ITEMINFO_ITEM_USAGE_OVERRIDE:
		it->item_usage.override = value;
		break;
	case ITEMINFO_GM_LV_TRADE_OVERRIDE:
		it->gm_lv_trade_override = value;
		break;
	default:
		ShowError("buildin_setiteminfo: invalid type %d.\n", n);
		script_pushint(st,-1);
		return false;
	}
	script_pushint(st,value);
	return true;
}

/*==========================================
 * Returns value from equipped item slot n [Lupus]
 * getequpcardid(num,slot)
 * where
 * num = eqip position slot
 * slot = 0,1,2,3 (Card Slot N)
 *
 * This func returns CARD ID, 255,254,-255 (for card 0, if the item is produced)
 * it's useful when you want to check item cards or if it's signed
 * Useful for such quests as "Sign this refined item with players name" etc
 * Hat[0] +4 -> Player's Hat[0] +4
 *------------------------------------------*/
static BUILDIN(getequipcardid)
{
	int i=-1,num,slot;
	struct map_session_data *sd;

	num=script_getnum(st,2);
	slot=script_getnum(st,3);
	sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	if (num > 0 && num <= ARRAYLENGTH(script->equip))
		i=pc->checkequip(sd,script->equip[num-1]);
	if(i >= 0 && slot>=0 && slot<4)
		script_pushint(st,sd->status.inventory[i].card[slot]);
	else
		script_pushint(st,0);

	return true;
}

/*==========================================
 * petskillbonus [Valaris] //Rewritten by [Skotlex]
 *------------------------------------------*/
static BUILDIN(petskillbonus)
{
	struct pet_data *pd;

	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL || sd->pd == NULL)
		return true;

	pd=sd->pd;
	if (pd->bonus)
	{ //Clear previous bonus
		if (pd->bonus->timer != INVALID_TIMER)
			timer->delete(pd->bonus->timer, pet->skill_bonus_timer);
	} else //init
		pd->bonus = (struct pet_bonus *) aMalloc(sizeof(struct pet_bonus));

	pd->bonus->type=script_getnum(st,2);
	pd->bonus->val=script_getnum(st,3);
	pd->bonus->duration=script_getnum(st,4);
	pd->bonus->delay=script_getnum(st,5);

	if (pd->state.skillbonus == 1)
		pd->state.skillbonus=0; // waiting state

	// wait for timer to start
	if (battle_config.pet_equip_required && pd->pet.equip == 0)
		pd->bonus->timer = INVALID_TIMER;
	else
		pd->bonus->timer = timer->add(timer->gettick()+pd->bonus->delay*1000, pet->skill_bonus_timer, sd->bl.id, 0);

	return true;
}

/*==========================================
 * pet looting [Valaris] //Rewritten by [Skotlex]
 *------------------------------------------*/
static BUILDIN(petloot)
{
	int max;
	struct pet_data *pd;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL || sd->pd == NULL)
		return true;

	max=script_getnum(st,2);

	if(max < 1)
		max = 1; //Let'em loot at least 1 item.
	else if (max > MAX_PETLOOT_SIZE)
		max = MAX_PETLOOT_SIZE;

	pd = sd->pd;
	if (pd->loot != NULL) {
		//Release whatever was there already and reallocate memory
		pet->lootitem_drop(pd, pd->msd);
		aFree(pd->loot->item);
	}
	else
		pd->loot = (struct pet_loot *)aMalloc(sizeof(struct pet_loot));

	pd->loot->item = (struct item *)aCalloc(max,sizeof(struct item));

	pd->loot->max=max;
	pd->loot->count = 0;
	pd->loot->weight = 0;

	return true;
}
/*==========================================
 * Set arrays with info of all sd inventory :
 * @inventorylist_id, @inventorylist_amount, @inventorylist_equip,
 * @inventorylist_refine, @inventorylist_identify, @inventorylist_attribute,
 * @inventorylist_card(0..3),
 * @inventorylist_opt_id(0..MAX_ITEM_OPTIONS),
 * @inventorylist_opt_val(0..MAX_ITEM_OPTIONS),
 * @inventorylist_opt_param(0..MAX_ITEM_OPTIONS),
 * @inventorylist_expire, @inventorylist_bound, @inventorylist_favorite,
 * @inventorylist_idx
 * @inventorylist_count = scalar
 *------------------------------------------*/
static BUILDIN(getinventorylist)
{
	struct map_session_data *sd = script->rid2sd(st);
	char script_var[SCRIPT_VARNAME_LENGTH];
	int j = 0, k = 0;

	if (sd == NULL)
		return true;

	for (int i = 0; i < sd->status.inventorySize; i++) {
		if (sd->status.inventory[i].nameid > 0 && sd->status.inventory[i].amount > 0) {
			pc->setreg(sd, reference_uid(script->add_variable("@inventorylist_id"), j), sd->status.inventory[i].nameid);
			pc->setreg(sd, reference_uid(script->add_variable("@inventorylist_amount"), j), sd->status.inventory[i].amount);
			if (sd->status.inventory[i].equip != 0) {
				pc->setreg(sd, reference_uid(script->add_variable("@inventorylist_equip"), j), pc->equippoint(sd, i));
			} else {
				pc->setreg(sd, reference_uid(script->add_variable("@inventorylist_equip"), j), 0);
			}
			pc->setreg(sd, reference_uid(script->add_variable("@inventorylist_refine"), j), sd->status.inventory[i].refine);
			pc->setreg(sd, reference_uid(script->add_variable("@inventorylist_identify"), j), sd->status.inventory[i].identify);
			pc->setreg(sd, reference_uid(script->add_variable("@inventorylist_attribute"), j), sd->status.inventory[i].attribute);
			for (k = 0; k < MAX_SLOTS; k++) {
				sprintf(script_var, "@inventorylist_card%d", k + 1);
				pc->setreg(sd, reference_uid(script->add_variable(script_var), j), sd->status.inventory[i].card[k]);
			}
			for (k = 0; k < MAX_ITEM_OPTIONS; k++) {
				sprintf(script_var, "@inventorylist_opt_id%d", k + 1);
				pc->setreg(sd, reference_uid(script->add_variable(script_var), j), sd->status.inventory[i].option[k].index);
				sprintf(script_var, "@inventorylist_opt_val%d", k + 1);
				pc->setreg(sd, reference_uid(script->add_variable(script_var), j), sd->status.inventory[i].option[k].value);
				sprintf(script_var, "@inventorylist_opt_param%d", k + 1);
				pc->setreg(sd, reference_uid(script->add_variable(script_var), j), sd->status.inventory[i].option[k].param);
			}
			pc->setreg(sd, reference_uid(script->add_variable("@inventorylist_expire"), j), sd->status.inventory[i].expire_time);
			pc->setreg(sd, reference_uid(script->add_variable("@inventorylist_bound"), j), sd->status.inventory[i].bound);
			pc->setreg(sd, reference_uid(script->add_variable("@inventorylist_favorite"), j), sd->status.inventory[i].favorite);
			pc->setreg(sd, reference_uid(script->add_variable("@inventorylist_idx"), j), i);
			j++;
		}
	}
	pc->setreg(sd, script->add_variable("@inventorylist_count"), j);
	return true;
}

static BUILDIN(getcartinventorylist)
{
	struct map_session_data *sd = script->rid2sd(st);
	char card_var[SCRIPT_VARNAME_LENGTH];

	int i,j=0,k;
	if(!sd) return true;

	for(i=0;i<MAX_CART;i++) {
		if(sd->status.cart[i].nameid > 0 && sd->status.cart[i].amount > 0) {
			pc->setreg(sd,reference_uid(script->add_variable("@cartinventorylist_id"), j),sd->status.cart[i].nameid);
			pc->setreg(sd,reference_uid(script->add_variable("@cartinventorylist_amount"), j),sd->status.cart[i].amount);
			pc->setreg(sd,reference_uid(script->add_variable("@cartinventorylist_equip"), j),sd->status.cart[i].equip);
			pc->setreg(sd,reference_uid(script->add_variable("@cartinventorylist_refine"), j),sd->status.cart[i].refine);
			pc->setreg(sd,reference_uid(script->add_variable("@cartinventorylist_identify"), j),sd->status.cart[i].identify);
			pc->setreg(sd,reference_uid(script->add_variable("@cartinventorylist_attribute"), j),sd->status.cart[i].attribute);
			for (k = 0; k < MAX_SLOTS; k++) {
				sprintf(card_var, "@cartinventorylist_card%d",k+1);
				pc->setreg(sd,reference_uid(script->add_variable(card_var), j),sd->status.cart[i].card[k]);
			}
			for (k = 0; k < MAX_ITEM_OPTIONS; k++) {
				sprintf(card_var, "@cartinventorylist_opt_id%d", k + 1);
				pc->setreg(sd, reference_uid(script->add_variable(card_var), j), sd->status.cart[i].option[k].index);
				sprintf(card_var, "@cartinventorylist_opt_val%d", k + 1);
				pc->setreg(sd, reference_uid(script->add_variable(card_var), j), sd->status.cart[i].option[k].value);
				sprintf(card_var, "@cartinventorylist_opt_param%d", k + 1);
				pc->setreg(sd, reference_uid(script->add_variable(card_var), j), sd->status.cart[i].option[k].param);
			}
			pc->setreg(sd,reference_uid(script->add_variable("@cartinventorylist_expire"), j),sd->status.cart[i].expire_time);
			pc->setreg(sd,reference_uid(script->add_variable("@cartinventorylist_bound"), j),sd->status.cart[i].bound);
			j++;
		}
	}
	pc->setreg(sd,script->add_variable("@cartinventorylist_count"),j);
	return true;
}

static BUILDIN(getskilllist)
{
	struct map_session_data *sd = script->rid2sd(st);
	int i,j=0;
	if (sd == NULL)
		return true;
	for (i = 0; i < MAX_SKILL_DB; i++) {
		if(sd->status.skill[i].id > 0 && sd->status.skill[i].lv > 0) {
			pc->setreg(sd,reference_uid(script->add_variable("@skilllist_id"), j),sd->status.skill[i].id);
			pc->setreg(sd,reference_uid(script->add_variable("@skilllist_lv"), j),sd->status.skill[i].lv);
			pc->setreg(sd,reference_uid(script->add_variable("@skilllist_flag"), j),sd->status.skill[i].flag);
			j++;
		}
	}
	pc->setreg(sd,script->add_variable("@skilllist_count"),j);
	return true;
}

static BUILDIN(clearitem)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;
	for (int i = 0; i < sd->status.inventorySize; i++) {
		if (sd->status.inventory[i].amount) {
			pc->delitem(sd, i, sd->status.inventory[i].amount, 0, DELITEM_NORMAL, LOG_TYPE_SCRIPT);
		}
	}
	return true;
}

/*==========================================
 * Disguise Player (returns Mob/NPC ID if success, 0 on fail)
 *------------------------------------------*/
static BUILDIN(disguise)
{
	int id;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	id = script_getnum(st,2);

	if (mob->db_checkid(id) || npc->db_checkid(id)) {
		pc->disguise(sd, id);
		script_pushint(st,id);
	} else
		script_pushint(st,0);

	return true;
}

/*==========================================
 * Undisguise Player (returns 1 if success, 0 on fail)
 *------------------------------------------*/
static BUILDIN(undisguise)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	if (sd->disguise != -1) {
		pc->disguise(sd, -1);
		script_pushint(st,0);
	} else {
		script_pushint(st,1);
	}
	return true;
}

/*==========================================
 * Transform a bl to another class,
 * @type unused
 *------------------------------------------*/
static BUILDIN(classchange)
{
	int class, type, target;
	struct block_list *bl = map->id2bl(st->oid);

	if (bl == NULL)
		return true;

	class = script_getnum(st, 2);
	type = script_getnum(st, 3);
	target = script_hasdata(st, 4) ? script_getnum(st, 4) : 0;

	if (target > 0) {
		struct map_session_data *sd = script->charid2sd(st, target);
		if (sd != NULL) {
			clif->class_change(bl, class, type, sd);
		}
	} else {
		clif->class_change(bl, class, type, NULL);
	}
	return true;
}

/*==========================================
 * Display an effect
 *------------------------------------------*/
static BUILDIN(misceffect)
{
	int type;

	type=script_getnum(st,2);
	if(st->oid && st->oid != npc->fake_nd->bl.id) {
		struct block_list *bl = map->id2bl(st->oid);
		if (bl)
			clif->specialeffect(bl,type,AREA);
	} else {
		struct map_session_data *sd = script->rid2sd(st);
		if (sd != NULL)
			clif->specialeffect(&sd->bl,type,AREA);
	}
	return true;
}
/*==========================================
 * Play a BGM on a single client [Rikter/Yommy]
 *------------------------------------------*/
static BUILDIN(playbgm)
{
	struct map_session_data* sd = script->rid2sd(st);

	if (sd != NULL) {
		const char *name = script_getstr(st,2);

		clif->playBGM(sd, name);
	}

	return true;
}

static int playbgm_sub(struct block_list *bl, va_list ap)
{
	const char* name = va_arg(ap,const char*);

	clif->playBGM(BL_CAST(BL_PC, bl), name);

	return 0;
}

static int playbgm_foreachpc_sub(struct map_session_data *sd, va_list args)
{
	const char* name = va_arg(args, const char*);

	nullpo_ret(name);
	clif->playBGM(sd, name);
	return 0;
}

/*==========================================
 * Play a BGM on multiple client [Rikter/Yommy]
 *------------------------------------------*/
static BUILDIN(playbgmall)
{
	const char* name;

	name = script_getstr(st,2);

	if( script_hasdata(st,7) ) {
		// specified part of map
		const char *mapname = script_getstr(st,3);
		int x0 = script_getnum(st,4);
		int y0 = script_getnum(st,5);
		int x1 = script_getnum(st,6);
		int y1 = script_getnum(st,7);
		int m;

		if ( ( m = map->mapname2mapid(mapname) ) == -1 ) {
			ShowWarning("playbgmall: Attempted to play song '%s' on non-existent map '%s'\n",name, mapname);
			return true;
		}

		map->foreachinarea(script->playbgm_sub, m, x0, y0, x1, y1, BL_PC, name);
	} else if( script_hasdata(st,3) ) {
		// entire map
		const char* mapname = script_getstr(st,3);
		int m;

		if ( ( m = map->mapname2mapid(mapname) ) == -1 ) {
			ShowWarning("playbgmall: Attempted to play song '%s' on non-existent map '%s'\n",name, mapname);
			return true;
		}

		map->foreachinmap(script->playbgm_sub, m, BL_PC, name);
	} else {
		// entire server
		map->foreachpc(script->playbgm_foreachpc_sub, name);
	}

	return true;
}

/*==========================================
 * Play a .wav sound for sd
 *------------------------------------------*/
static BUILDIN(soundeffect)
{
	struct map_session_data *sd = script->rid2sd(st);
	const char* name = script_getstr(st,2);
	int type = script_getnum(st,3);

	if (sd != NULL) {
		clif->soundeffect(sd,&sd->bl,name,type);
	}
	return true;
}

static int soundeffect_sub(struct block_list *bl, va_list ap)
{
	struct map_session_data *sd = NULL;
	char *name = va_arg(ap, char *);
	int type = va_arg(ap, int);

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_PC);
	sd = BL_UCAST(BL_PC, bl);

	clif->soundeffect(sd, bl, name, type);

	return true;
}

/*==========================================
 * Play a sound effect (.wav) on multiple clients
 * soundeffectall "<filepath>",<type>{,"<map name>"}{,<x0>,<y0>,<x1>,<y1>};
 *------------------------------------------*/
static BUILDIN(soundeffectall)
{
	struct block_list* bl;
	const char* name;
	int type;

	bl = (st->rid) ? &(script->rid2sd(st)->bl) : map->id2bl(st->oid);
	if (!bl)
		return true;

	name = script_getstr(st,2);
	type = script_getnum(st,3);

	//FIXME: enumerating map squares (map->foreach) is slower than enumerating the list of online players (map->foreachpc?) [ultramage]

	if(!script_hasdata(st,4)) { // area around
		clif->soundeffectall(bl, name, type, AREA);
	} else {
		if(!script_hasdata(st,5)) { // entire map
			const char *mapname = script_getstr(st,4);
			int m;

			if ( ( m = map->mapname2mapid(mapname) ) == -1 ) {
				ShowWarning("soundeffectall: Attempted to play song '%s' (type %d) on non-existent map '%s'\n",name,type, mapname);
				return true;
			}

			map->foreachinmap(script->soundeffect_sub, m, BL_PC, name, type);
		} else if(script_hasdata(st,8)) { // specified part of map
			const char *mapname = script_getstr(st,4);
			int x0 = script_getnum(st,5);
			int y0 = script_getnum(st,6);
			int x1 = script_getnum(st,7);
			int y1 = script_getnum(st,8);
			int m;

			if ( ( m = map->mapname2mapid(mapname) ) == -1 ) {
				ShowWarning("soundeffectall: Attempted to play song '%s' (type %d) on non-existent map '%s'\n",name,type, mapname);
				return true;
			}

			map->foreachinarea(script->soundeffect_sub, m, x0, y0, x1, y1, BL_PC, name, type);
		} else {
			ShowError("buildin_soundeffectall: insufficient arguments for specific area broadcast.\n");
		}
	}

	return true;
}
/*==========================================
 * pet status recovery [Valaris] / Rewritten by [Skotlex]
 *------------------------------------------*/
static BUILDIN(petrecovery)
{
	struct pet_data *pd;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL || sd->pd == NULL)
		return true;

	pd=sd->pd;

	if (pd->recovery)
	{ //Halt previous bonus
		if (pd->recovery->timer != INVALID_TIMER)
			timer->delete(pd->recovery->timer, pet->recovery_timer);
	} else //Init
		pd->recovery = (struct pet_recovery *)aMalloc(sizeof(struct pet_recovery));

	pd->recovery->type = (sc_type)script_getnum(st,2);
	pd->recovery->delay = script_getnum(st,3);
	pd->recovery->timer = INVALID_TIMER;

	return true;
}

/*==========================================
 * pet attack skills [Valaris]
 *------------------------------------------*/
/// petskillattack <skill id>,<level>,<div>,<rate>,<bonusrate>
/// petskillattack "<skill name>",<level>,<div>,<rate>,<bonusrate>
static BUILDIN(petskillattack)
{
	struct pet_data *pd;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd==NULL || sd->pd==NULL)
		return true;

	pd=sd->pd;
	if (pd->a_skill == NULL)
		pd->a_skill = (struct pet_skill_attack *)aMalloc(sizeof(struct pet_skill_attack));

	pd->a_skill->id = script_isstringtype(st,2) ? skill->name2id(script_getstr(st,2)) : script_getnum(st,2);
	pd->a_skill->lv = script_getnum(st,3);
	pd->a_skill->div_ = script_getnum(st,4);
	pd->a_skill->rate = script_getnum(st,5);
	pd->a_skill->bonusrate = script_getnum(st,6);

	return true;
}

/*==========================================
 * pet support skills [Skotlex]
 *------------------------------------------*/
/// petskillsupport <skill id>,<level>,<delay>,<hp>,<sp>
/// petskillsupport "<skill name>",<level>,<delay>,<hp>,<sp>
static BUILDIN(petskillsupport)
{
	struct pet_data *pd;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL || sd->pd == NULL)
		return true;

	pd=sd->pd;
	if (pd->s_skill) {
		//Clear previous skill
		if (pd->s_skill->timer != INVALID_TIMER) {
			timer->delete(pd->s_skill->timer, pet->skill_support_timer);
		}
	} else {
		//init memory
		pd->s_skill = (struct pet_skill_support *) aMalloc(sizeof(struct pet_skill_support));
	}

	pd->s_skill->id=( script_isstringtype(st,2) ? skill->name2id(script_getstr(st,2)) : script_getnum(st,2) );
	pd->s_skill->lv=script_getnum(st,3);
	pd->s_skill->delay=script_getnum(st,4);
	pd->s_skill->hp=script_getnum(st,5);
	pd->s_skill->sp=script_getnum(st,6);

	//Use delay as initial offset to avoid skill/heal exploits
	if (battle_config.pet_equip_required && pd->pet.equip == 0)
		pd->s_skill->timer = INVALID_TIMER;
	else
		pd->s_skill->timer = timer->add(timer->gettick()+pd->s_skill->delay*1000,pet->skill_support_timer,sd->bl.id,0);

	return true;
}

/*==========================================
 * Scripted skill effects [Celest]
 *------------------------------------------*/
/// skilleffect <skill id>,<level>
/// skilleffect "<skill name>",<level>
static BUILDIN(skilleffect)
{
	struct map_session_data *sd;

	uint16 skill_id=( script_isstringtype(st,2) ? skill->name2id(script_getstr(st,2)) : script_getnum(st,2) );
	uint16 skill_lv=script_getnum(st,3);
	sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	/* ensure we're standing because the following packet causes the client to virtually set the char to stand,
	 * which leaves the server thinking it still is sitting. */
	if( pc_issit(sd) ) {
		pc->setstand(sd);
		skill->sit(sd,0);
		clif->standing(&sd->bl);
	}
	clif->skill_nodamage(&sd->bl,&sd->bl,skill_id,skill_lv,1);

	return true;
}

/*==========================================
 * NPC skill effects [Valaris]
 *------------------------------------------*/
/// npcskilleffect <skill id>,<level>,<x>,<y>
/// npcskilleffect "<skill name>",<level>,<x>,<y>
static BUILDIN(npcskilleffect)
{
	struct block_list *bl= map->id2bl(st->oid);

	uint16 skill_id=( script_isstringtype(st,2) ? skill->name2id(script_getstr(st,2)) : script_getnum(st,2) );
	uint16 skill_lv=script_getnum(st,3);
	int x=script_getnum(st,4);
	int y=script_getnum(st,5);

	if (bl)
		clif->skill_poseffect(bl,skill_id,skill_lv,x,y,timer->gettick());

	return true;
}

/*==========================================
 * Special effects [Valaris]
 *------------------------------------------*/
static BUILDIN(specialeffect)
{
	struct block_list *bl = NULL;
	int type = script_getnum(st, 2);
	enum send_target target = AREA;

	if (script_hasdata(st, 3)) {
		target = script_getnum(st, 3);
	}

	if (script_hasdata(st, 4)) {
		if (script_isstringtype(st, 4)) {
			struct npc_data *nd = npc->name2id(script_getstr(st, 4));
			if (nd != NULL) {
				bl = &nd->bl;
			}
		} else {
			bl = map->id2bl(script_getnum(st, 4));
		}
	} else {
		bl = map->id2bl(st->oid);
	}

	if (bl == NULL) {
		return true;
	}

	if (target == SELF) {
		struct map_session_data *sd;
		if (script_hasdata(st, 5)) {
			sd = map->id2sd(script_getnum(st, 5));
		} else {
			sd = script->rid2sd(st);
		}
		if (sd != NULL) {
			clif->specialeffect_single(bl, type, sd->fd);
		}
	} else {
		clif->specialeffect(bl, type, target);
	}

	return true;
}

/*==========================================
 * Special effects with num [4144]
 *------------------------------------------*/
static BUILDIN(specialeffectnum)
{
	struct block_list *bl = NULL;
	int type = script_getnum(st, 2);
	int num = script_getnum(st, 3);
	int num2 = script_getnum(st, 4);
	enum send_target target = AREA;

	if (script_hasdata(st, 5)) {
		target = script_getnum(st, 5);
	}

	if (script_hasdata(st, 6)) {
		if (script_isstringtype(st, 6)) {
			struct npc_data *nd = npc->name2id(script_getstr(st, 6));
			if (nd != NULL) {
				bl = &nd->bl;
			}
		} else {
			bl = map->id2bl(script_getnum(st, 6));
		}
	} else {
		bl = map->id2bl(st->oid);
	}

	if (bl == NULL) {
		return true;
	}

	uint64 bigNum = ((uint64)num2) * 0xffffffff + num;
	if (target == SELF) {
		struct map_session_data *sd;
		if (script_hasdata(st, 7)) {
			sd = map->id2sd(script_getnum(st, 7));
		} else {
			sd = script->rid2sd(st);
		}
		if (sd != NULL) {
			clif->specialeffect_value_single(bl, type, bigNum, sd->fd);
		}
	} else {
		clif->specialeffect_value(bl, type, bigNum, target);
	}

	return true;
}

static BUILDIN(specialeffect2)
{
	struct map_session_data *sd;
	int type = script_getnum(st,2);
	enum send_target target = script_hasdata(st,3) ? (send_target)script_getnum(st,3) : AREA;

	if (script_hasdata(st,4))
		sd = script->nick2sd(st, script_getstr(st,4));
	else
		sd = script->rid2sd(st);

	if (sd != NULL)
		clif->specialeffect(&sd->bl, type, target);

	return true;
}

static BUILDIN(removespecialeffect)
{
	struct block_list *bl = NULL;
	int type = script_getnum(st, 2);
	enum send_target target = AREA;

	if (script_hasdata(st, 3)) {
		target = script_getnum(st, 3);
	}

	if (script_hasdata(st, 4)) {
		if (script_isstringtype(st, 4)) {
			struct npc_data *nd = npc->name2id(script_getstr(st, 4));
			if (nd != NULL) {
				bl = &nd->bl;
			}
		} else {
			bl = map->id2bl(script_getnum(st, 4));
		}
	} else {
		bl = map->id2bl(st->oid);
	}

	if (bl == NULL) {
		return true;
	}

	if (target == SELF) {
		struct map_session_data *sd;
		if (script_hasdata(st, 5)) {
			sd = map->id2sd(script_getnum(st, 5));
		} else {
			sd = script->rid2sd(st);
		}
		if (sd != NULL) {
			clif->removeSpecialEffect_single(bl, type, &sd->bl);
		}
	} else {
		clif->removeSpecialEffect(bl, type, target);
	}

	return true;
}

/*==========================================
 * Nude [Valaris]
 *------------------------------------------*/
static BUILDIN(nude)
{
	struct map_session_data *sd = script->rid2sd(st);
	int i, calcflag = 0;

	if (sd == NULL)
		return true;

	for( i = 0 ; i < EQI_MAX; i++ ) {
		if( sd->equip_index[ i ] >= 0 ) {
			if( !calcflag )
				calcflag = 1;
			pc->unequipitem(sd, sd->equip_index[i], PCUNEQUIPITEM_FORCE);
		}
	}

	if( calcflag )
		status_calc_pc(sd,SCO_NONE);

	return true;
}

/*==========================================
 * gmcommand [MouseJstr]
 *------------------------------------------*/
static BUILDIN(atcommand)
{
	struct map_session_data *sd, *dummy_sd = NULL;
	int fd;
	const char* cmd;
	bool ret = true;

	cmd = script_getstr(st,2);

	if (st->rid) {
		sd = script->rid2sd(st);
		if (sd == NULL)
			return true;
		fd = sd->fd;
	} else { //Use a dummy character.
		sd = dummy_sd = pc->get_dummy_sd();
		fd = 0;

		if (st->oid) {
			struct block_list* bl = map->id2bl(st->oid);
			memcpy(&sd->bl, bl, sizeof(struct block_list));
			if (bl->type == BL_NPC)
				safestrncpy(sd->status.name, BL_UCAST(BL_NPC, bl)->name, NAME_LENGTH);
		}
	}

	if (!atcommand->exec(fd, sd, cmd, false)) {
		ShowWarning("script: buildin_atcommand: failed to execute command '%s'\n", cmd);
		script->reportsrc(st);
		ret = false;
	}
	if (dummy_sd) aFree(dummy_sd);
	return ret;
}

/**
 * Displays a message for the player only (like system messages like "you got an apple")
 *
 * @code
 *   dispbottom "<message>"{,<color>};
 * @endcode
 */
static BUILDIN(dispbottom)
{
	struct map_session_data *sd = script->rid2sd(st);
	const char *message = script_getstr(st,2);

	if (sd == NULL)
		return true;

	if (script_hasdata(st,3)) {
		int color = script_getnum(st,3);
		clif->messagecolor_self(sd->fd, color, message);
	} else {
		clif_disp_onlyself(sd, message);
	}

	return true;
}

/*==========================================
 * All The Players Full Recovery
 * (HP/SP full restore and resurrect if need)
 *------------------------------------------*/
static int buildin_recovery_sub(struct map_session_data *sd)
{
	nullpo_retr(0, sd);

	if (pc_isdead(sd)) {
		status->revive(&sd->bl, 100, 100);
	} else {
		status_percent_heal(&sd->bl, 100, 100);
	}

	return 0;
}

static int buildin_recovery_pc_sub(struct map_session_data *sd, va_list ap)
{
	return script->buildin_recovery_sub(sd);
}

static int buildin_recovery_bl_sub(struct block_list *bl, va_list ap)
{
	return script->buildin_recovery_sub(BL_CAST(BL_PC, bl));
}

static BUILDIN(recovery)
{
	if (script_hasdata(st, 2)) {
		if (script_isstringtype(st, 2)) {
			int16 m = map->mapname2mapid(script_getstr(st, 2));

			if (m == -1) {
				ShowWarning("script:recovery: invalid map!\n");
				return false;
			}

			if (script_hasdata(st, 6)) {
				int16 x1 = script_getnum(st, 3);
				int16 y1 = script_getnum(st, 4);
				int16 x2 = script_getnum(st, 5);
				int16 y2 = script_getnum(st, 6);
				map->foreachinarea(script->buildin_recovery_bl_sub, m, x1, y1, x2, y2, BL_PC);
			} else {
				map->foreachinmap(script->buildin_recovery_bl_sub, m, BL_PC);
			}
		} else {
			struct map_session_data *sd = script->id2sd(st, script_getnum(st, 2));

			if (sd != NULL) {
				script->buildin_recovery_sub(sd);
			}
		}
	} else {
		map->foreachpc(script->buildin_recovery_pc_sub);
	}
	return true;
}

/*
 * Get your current pet information
 */
static BUILDIN(getpetinfo)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	struct pet_data *pd = sd->pd;
	int type = script_getnum(st, 2);
	if (pd == NULL) {
		if (type == PETINFO_NAME)
			script_pushconststr(st, "null");
		else
			script_pushint(st, 0);
		return true;
	}

	switch(type) {
	case PETINFO_ID:
		script_pushint(st, pd->pet.pet_id);
		break;
	case PETINFO_CLASS:
		script_pushint(st, pd->pet.class_);
		break;
	case PETINFO_NAME:
		script_pushstrcopy(st, pd->pet.name);
		break;
	case PETINFO_INTIMACY:
		script_pushint(st, pd->pet.intimate);
		break;
	case PETINFO_HUNGRY:
		script_pushint(st, pd->pet.hungry);
		break;
	case PETINFO_RENAME:
		script_pushint(st, pd->pet.rename_flag);
		break;
	case PETINFO_GID:
		script_pushint(st, pd->bl.id);
		break;
	case PETINFO_EGGITEM:
		script_pushint(st, pd->pet.egg_id);
		break;
	case PETINFO_FOODITEM:
		script_pushint(st, pd->petDB->FoodID);
		break;
	case PETINFO_ACCESSORYITEM:
		script_pushint(st, pd->petDB->AcceID);
		break;
	case PETINFO_ACCESSORYFLAG:
		script_pushint(st, (pd->pet.equip != 0)? 1:0);
		break;
	case PETINFO_EVO_EGGID:
		if (VECTOR_DATA(pd->petDB->evolve_data) != NULL)
			script_pushint(st, VECTOR_DATA(pd->petDB->evolve_data)->petEggId);
		else
			script_pushint(st, 0);
		break;
	case PETINFO_AUTOFEED:
		script_pushint(st, pd->pet.autofeed);
		break;
	default:
		ShowWarning("buildin_getpetinfo: Invalid type %d.\n", type);
		script_pushint(st, 0);
		return false;
	}

	return true;
}

/*==========================================
 * Get your homunculus info: gethominfo(n)
 * n -> 0:hom_id 1:class 2:name
 * 3:friendly 4:hungry, 5: rename flag.
 * 6: level
 *------------------------------------------*/
static BUILDIN(gethominfo)
{
	struct map_session_data *sd = script->rid2sd(st);
	int type = script_getnum(st,2);

	if (sd == NULL || sd->hd == NULL) {
		if (type == 2)
			script_pushconststr(st,"null");
		else
			script_pushint(st,0);
		return true;
	}

	switch(type) {
		case 0: script_pushint(st,sd->hd->homunculus.hom_id); break;
		case 1: script_pushint(st,sd->hd->homunculus.class_); break;
		case 2: script_pushstrcopy(st,sd->hd->homunculus.name); break;
		case 3: script_pushint(st,sd->hd->homunculus.intimacy); break;
		case 4: script_pushint(st,sd->hd->homunculus.hunger); break;
		case 5: script_pushint(st,sd->hd->homunculus.rename_flag); break;
		case 6: script_pushint(st,sd->hd->homunculus.level); break;
		default:
			script_pushint(st,0);
			break;
	}
	return true;
}

/*
 * Retrieves information about character's mercenary
 * getmercinfo <type>{, <char id> };
 */
static BUILDIN(getmercinfo)
{
	struct map_session_data *sd;
	if (script_hasdata(st, 3)) {
		if ((sd = script->charid2sd(st, script_getnum(st, 3))) == NULL) {
			script_pushnil(st);
			return true;
		}
	} else {
		if ((sd = script->rid2sd(st)) == NULL)
			return true;
	}

	struct mercenary_data *md = (sd->status.mer_id && sd->md)? sd->md : NULL;
	int type = script_getnum(st, 2);
	if (md == NULL) {
		if (type == MERCINFO_NAME)
			script_pushconststr(st, "");
		else
			script_pushint(st, 0);
		return true;
	}

	switch (type) {
	case MERCINFO_ID:
		script_pushint(st, md->mercenary.mercenary_id);
		break;
	case MERCINFO_CLASS:
		script_pushint(st, md->mercenary.class_);
		break;
	case MERCINFO_NAME:
		script_pushstrcopy(st, md->db->name);
		break;
	case MERCINFO_FAITH:
		script_pushint(st, mercenary->get_faith(md));
		break;
	case MERCINFO_CALLS:
		script_pushint(st, mercenary->get_calls(md));
		break;
	case MERCINFO_KILLCOUNT:
		script_pushint(st, md->mercenary.kill_count);
		break;
	case MERCINFO_LIFETIME:
		script_pushint(st, mercenary->get_lifetime(md));
		break;
	case MERCINFO_LEVEL:
		script_pushint(st, md->db->lv);
		break;
	case MERCINFO_GID:
		script_pushint(st, md->bl.id);
		break;
	default:
		ShowError("buildin_getmercinfo: Invalid type %d (char_id=%d).\n", type, sd->status.char_id);
		script_pushnil(st);
		return false;
	}

	return true;
}

/*==========================================
 * Shows wether your inventory(and equips) contain
 * selected card or not.
 * checkequipedcard(4001);
 *------------------------------------------*/
static BUILDIN(checkequipedcard)
{
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	int c = script_getnum(st,2);

	for (int i = 0; i < sd->status.inventorySize; i++) {
		if(sd->status.inventory[i].nameid > 0 && sd->status.inventory[i].amount && sd->inventory_data[i]) {
			if (itemdb_isspecial(sd->status.inventory[i].card[0]))
				continue;
			for (int n = 0; n < sd->inventory_data[i]->slot; n++) {
				if(sd->status.inventory[i].card[n]==c) {
					script_pushint(st,1);
					return true;
				}
			}
		}
	}

	script_pushint(st,0);
	return true;
}

static BUILDIN(__jump_zero)
{
	int sel;
	sel=script_getnum(st,2);
	if (!sel) {
		int pos;
		if (!data_islabel(script_getdata(st,3))) {
			ShowError("script: jump_zero: not a label !\n");
			st->state=END;
			return false;
		}

		pos=script_getnum(st,3);
		st->pos=pos;
		st->state=GOTO;
	}
	return true;
}

/*==========================================
 * movenpc [MouseJstr]
 *------------------------------------------*/
static BUILDIN(movenpc)
{
	struct npc_data *nd = NULL;
	const char *npc_name;
	int x,y;

	npc_name = script_getstr(st,2);
	x = script_getnum(st,3);
	y = script_getnum(st,4);

	if ((nd = npc->name2id(npc_name)) == NULL)
		return false;

	if (script_hasdata(st,5))
		nd->dir = script_getnum(st,5) % 8;
	npc->movenpc(nd, x, y);
	return true;
}

/*==========================================
 * message [MouseJstr]
 *------------------------------------------*/
static BUILDIN(message)
{
	const char *message;
	struct map_session_data *sd = NULL;

	if (script_isstringtype(st,2))
		sd = script->nick2sd(st, script_getstr(st,2));
	else
		sd = script->id2sd(st, script_getnum(st,2));

	if (sd == NULL)
		return true;

	message = script_getstr(st,3);
	clif->message(sd->fd, message);

	return true;
}

static BUILDIN(servicemessage)
{
	struct map_session_data *sd = NULL;

	if (script_hasdata(st, 4)) {
		if (script_isstringtype(st, 4))
			sd = script->nick2sd(st, script_getstr(st, 4));
		else
			sd = script->id2sd(st, script_getnum(st, 4));
	} else {
		sd = script->rid2sd(st);
	}

	if (sd == NULL)
		return true;

	const char *message = script_getstr(st, 2);
	const int color = script_getnum(st, 3);
	clif->serviceMessageColor(sd, color, message);

	return true;
}

/*==========================================
 * npctalk (sends message to surrounding area)
 * usage: npctalk("<message>"{, "<npc name>"{, <show_name>}});
 *------------------------------------------*/
static BUILDIN(npctalk)
{
	struct npc_data* nd;
	const char *str = script_getstr(st,2);
	bool show_name = true;

	if (script_hasdata(st, 3)) {
		nd = npc->name2id(script_getstr(st, 3));
	} else {
		nd = map->id2nd(st->oid);
	}

	if (script_hasdata(st, 4)) {
		show_name = (script_getnum(st, 4) != 0) ? true : false;
	}

	if (nd != NULL) {
		char name[NAME_LENGTH], message[256];
		safestrncpy(name, nd->name, sizeof(name));
		strtok(name, "#"); // discard extra name identifier if present
		if (show_name) {
			safesnprintf(message, sizeof(message), "%s : %s", name, str);
		} else {
			safesnprintf(message, sizeof(message), "%s", str);
		}
		clif->disp_overhead(&nd->bl, message, AREA_CHAT_WOC, NULL);
	}

	return true;
}

// change npc walkspeed [Valaris]
static BUILDIN(npcspeed)
{
	struct npc_data *nd = map->id2nd(st->oid);
	int speed = script_getnum(st, 2);

	if (nd != NULL) {
		unit->bl2ud2(&nd->bl); // ensure nd->ud is safe to edit
		if (nd->ud == NULL) {
			ShowWarning("buildin_npcspeed: floating NPC don't have unit data.\n");
			return false;
		}
		nd->speed = speed;
		nd->ud->state.speed_changed = 1;
	}

	return true;
}

// make an npc walk to a position [Valaris]
static BUILDIN(npcwalkto)
{
	struct npc_data *nd = map->id2nd(st->oid);
	int x = script_getnum(st, 2);
	int y = script_getnum(st, 3);

	if (nd != NULL) {
		unit->bl2ud2(&nd->bl); // ensure nd->ud is safe to edit
		if (nd->ud == NULL) {
			ShowWarning("buildin_npcwalkto: floating NPC don't have unit data.\n");
			return false;
		}
		if (!nd->status.hp) {
			status_calc_npc(nd, SCO_FIRST);
		} else {
			status_calc_npc(nd, SCO_NONE);
		}
		unit->walk_toxy(&nd->bl, x, y, 0);
	}

	return true;
}
// stop an npc's movement [Valaris]
static BUILDIN(npcstop)
{
	struct npc_data *nd = map->id2nd(st->oid);

	if (nd != NULL) {
		unit->bl2ud2(&nd->bl); // ensure nd->ud is safe to edit
		if (nd->ud == NULL) {
			ShowWarning("buildin_npcstop: floating NPC don't have unit data.\n");
			return false;
		}
		unit->stop_walking(&nd->bl, STOPWALKING_FLAG_FIXPOS|STOPWALKING_FLAG_NEXTCELL);
	}

	return true;
}

// set click npc distance [4144]
static BUILDIN(setnpcdistance)
{
	struct npc_data *nd = map->id2nd(st->oid);
	if (nd == NULL)
		return false;

	nd->area_size = script_getnum(st, 2);

	return true;
}

// return current npc direction [4144]
static BUILDIN(getnpcdir)
{
	const struct npc_data *nd = NULL;

	if (script_hasdata(st, 2)) {
		nd = npc->name2id(script_getstr(st, 2));
	}
	if (nd == NULL && !st->oid) {
		script_pushint(st, -1);
		return true;
	}

	if (nd == NULL)
		nd = map->id2nd(st->oid);

	if (nd == NULL) {
		script_pushint(st, -1);
		return true;
	}

	script_pushint(st, (int)nd->dir);

	return true;
}

// set npc direction [4144]
static BUILDIN(setnpcdir)
{
	int newdir;
	struct npc_data *nd = NULL;

	if (script_hasdata(st, 3)) {
		nd = npc->name2id(script_getstr(st, 2));
		newdir = script_getnum(st, 3);
	} else if (script_hasdata(st, 2)) {
		if (!st->oid)
			return false;

		nd = map->id2nd(st->oid);
		newdir = script_getnum(st, 2);
	}
	if (nd == NULL)
		return false;

	if (newdir < 0)
		newdir = 0;
	else if (newdir > 7)
		newdir = 7;

	nd->dir = newdir;
	if (nd->ud)
		nd->ud->dir = newdir;

	clif->clearunit_area(&nd->bl, CLR_OUTSIGHT);
	clif->spawn(&nd->bl);

	return true;
}

// return npc class [4144]
static BUILDIN(getnpcclass)
{
	const struct npc_data *nd = NULL;

	if (script_hasdata(st, 2)) {
		nd = npc->name2id(script_getstr(st, 2));
	}
	if (nd == NULL && !st->oid) {
		script_pushint(st, -1);
		return false;
	}

	if (nd == NULL)
		nd = map->id2nd(st->oid);

	if (nd == NULL) {
		script_pushint(st, -1);
		return false;
	}

	script_pushint(st, (int)nd->class_);

	return true;
}

/*==========================================
 * getlook char info. getlook(arg)
 *------------------------------------------*/
static BUILDIN(getlook)
{
	int type,val = -1;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	type=script_getnum(st,2);
	switch(type) {
		case LOOK_HAIR:          val = sd->status.hair;          break; //1
		case LOOK_WEAPON:        val = sd->status.look.weapon;   break; //2
		case LOOK_HEAD_BOTTOM:   val = sd->status.look.head_bottom; break; //3
		case LOOK_HEAD_TOP:      val = sd->status.look.head_top; break; //4
		case LOOK_HEAD_MID:      val = sd->status.look.head_mid; break; //5
		case LOOK_HAIR_COLOR:    val = sd->status.hair_color;    break; //6
		case LOOK_CLOTHES_COLOR: val = sd->status.clothes_color; break; //7
		case LOOK_SHIELD:        val = sd->status.look.shield;   break; //8
		case LOOK_SHOES:                                         break; //9
		case LOOK_ROBE:          val = sd->status.look.robe;     break; //12
		case LOOK_BODY2:         val=sd->status.body;            break; //13
	}

	script_pushint(st,val);
	return true;
}

/*==========================================
 *     get char save point. argument: 0- map name, 1- x, 2- y
 *------------------------------------------*/
static BUILDIN(getsavepoint)
{
	int type;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	type = script_getnum(st,2);

	switch(type) {
		case 0: script_pushstrcopy(st,mapindex_id2name(sd->status.save_point.map)); break;
		case 1: script_pushint(st,sd->status.save_point.x); break;
		case 2: script_pushint(st,sd->status.save_point.y); break;
		default:
			script_pushint(st,0);
			break;
	}
	return true;
}

/*==========================================
 * Get position for  char/npc/pet/mob objects. Added by Lorky
 *
 *     int getMapXY(MapName$,MapX,MapY,type,[CharName$]);
 *             where type:
 *                     MapName$ - String variable for output map name
 *                     MapX     - Integer variable for output coord X
 *                     MapY     - Integer variable for output coord Y
 *                     type     - type of object
 *                                0 - Character coord
 *                                1 - NPC coord
 *                                2 - Pet coord
 *                                3 - Mob coord (not released)
 *                                4 - Homun coord
 *                                5 - Mercenary coord
 *                                6 - Elemental coord
 *                     CharName$ - Name object. If miss or "this" the current object
 *
 *             Return:
 *                     0        - success
 *                     -1       - some error, MapName$,MapX,MapY contains unknown value.
 *------------------------------------------*/
static BUILDIN(getmapxy)
{
	struct block_list *bl = NULL;
	struct map_session_data *sd = NULL;

	int64 num;
	const char *name;
	char prefix;

	int x,y,type;
	char mapname[MAP_NAME_LENGTH];

	if( !data_isreference(script_getdata(st,2)) ) {
		ShowWarning("script: buildin_getmapxy: not mapname variable\n");
		script_pushint(st,-1);
		return false;
	}
	if( !data_isreference(script_getdata(st,3)) ) {
		ShowWarning("script: buildin_getmapxy: not mapx variable\n");
		script_pushint(st,-1);
		return false;
	}
	if( !data_isreference(script_getdata(st,4)) ) {
		ShowWarning("script: buildin_getmapxy: not mapy variable\n");
		script_pushint(st,-1);
		return false;
	}

	if( !is_string_variable(reference_getname(script_getdata(st, 2))) ) {
		ShowWarning("script: buildin_getmapxy: %s is not a string variable\n",reference_getname(script_getdata(st, 2)));
		script_pushint(st,-1);
		return false;
	}

	if( is_string_variable(reference_getname(script_getdata(st, 3))) ) {
		ShowWarning("script: buildin_getmapxy: %s is a string variable, should be int\n",reference_getname(script_getdata(st, 3)));
		script_pushint(st,-1);
		return false;
	}

	if( is_string_variable(reference_getname(script_getdata(st, 4))) ) {
		ShowWarning("script: buildin_getmapxy: %s is a string variable, should be int\n",reference_getname(script_getdata(st, 4)));
		script_pushint(st,-1);
		return false;
	}

	// Possible needly check function parameters on C_STR,C_INT,C_INT
	type=script_getnum(st,5);

	switch (type) {
		case 0: //Get Character Position
			if (script_hasdata(st,6)) {
				if (script_isstringtype(st,6))
					sd = map->nick2sd(script_getstr(st,6), false);
				else
					sd = map->id2sd(script_getnum(st,6));
			} else {
				sd = script->rid2sd(st);
			}

			if (sd)
				bl = &sd->bl;
			break;
		case 1: //Get NPC Position
			if (script_hasdata(st,6)) {
				struct npc_data *nd;
				if (script_isstringtype(st,6))
					nd = npc->name2id(script_getstr(st,6));
				else
					nd = map->id2nd(script_getnum(st,6));
				if (nd)
					bl = &nd->bl;
			} else {
				//In case the origin is not an npc?
				bl = map->id2bl(st->oid);
			}
			break;
		case 2: //Get Pet Position
			if (script_hasdata(st,6)) {
				if (script_isstringtype(st,6))
					sd = map->nick2sd(script_getstr(st,6), false);
				else {
					bl = map->id2bl(script_getnum(st,6));
					break;
				}
			} else {
				sd = script->rid2sd(st);
			}

			if (sd && sd->pd)
				bl = &sd->pd->bl;
			break;
		case 3: //Get Mob Position
			if (script_hasdata(st,6)) {
				if (script_isstringtype(st,6))
					break;
				bl = map->id2bl(script_getnum(st,6));
			}
			break;
		case 4: //Get Homun Position
			if (script_hasdata(st,6)) {
				if (script_isstringtype(st,6)) {
					sd = map->nick2sd(script_getstr(st,6), false);
				} else {
					bl = map->id2bl(script_getnum(st,6));
					break;
				}
			} else {
				sd = script->rid2sd(st);
			}

			if (sd && sd->hd)
				bl = &sd->hd->bl;
			break;
		case 5: //Get Mercenary Position
			if (script_hasdata(st,6)) {
				if (script_isstringtype(st,6)) {
					sd = map->nick2sd(script_getstr(st,6), false);
				} else {
					bl = map->id2bl(script_getnum(st,6));
					break;
				}
			} else {
				sd = script->rid2sd(st);
			}

			if (sd && sd->md)
				bl = &sd->md->bl;
			break;
		case 6: //Get Elemental Position
			if (script_hasdata(st,6)) {
				if (script_isstringtype(st,6)) {
					sd = map->nick2sd(script_getstr(st,6), false);
				} else {
					bl = map->id2bl(script_getnum(st,6));
					break;
				}
			} else {
				sd = script->rid2sd(st);
			}

			if (sd && sd->ed)
				bl = &sd->ed->bl;
			break;
		default:
			ShowWarning("script: buildin_getmapxy: Invalid type %d\n", type);
			script_pushint(st,-1);
			return false;
	}
	if (!bl || bl->m == -1) { //No object found.
		script_pushint(st,-1);
		return true;
	}

	x= bl->x;
	y= bl->y;
	safestrncpy(mapname, map->list[bl->m].name, MAP_NAME_LENGTH);

	//Set MapName$
	num=st->stack->stack_data[st->start+2].u.num;
	name=script->get_str(script_getvarid(num));
	prefix=*name;

	if(not_server_variable(prefix))
		sd=script->rid2sd(st);
	else
		sd=NULL;
	script->set_reg(st, sd, num, name, mapname, script_getref(st, 2));

	//Set MapX
	num=st->stack->stack_data[st->start+3].u.num;
	name=script->get_str(script_getvarid(num));
	prefix=*name;

	if(not_server_variable(prefix))
		sd=script->rid2sd(st);
	else
		sd=NULL;
	script->set_reg(st, sd, num, name, (const void *)h64BPTRSIZE(x), script_getref(st, 3));

	//Set MapY
	num=st->stack->stack_data[st->start+4].u.num;
	name=script->get_str(script_getvarid(num));
	prefix=*name;

	if(not_server_variable(prefix))
		sd=script->rid2sd(st);
	else
		sd=NULL;
	script->set_reg(st, sd, num, name, (const void *)h64BPTRSIZE(y), script_getref(st, 4));

	//Return Success value
	script_pushint(st,0);
	return true;
}

enum logmes_type {
	LOGMES_NPC,
	LOGMES_ATCOMMAND
};

/*==========================================
 * Allows player to write logs (i.e. Bank NPC, etc) [Lupus]
 *------------------------------------------*/
static BUILDIN(logmes)
{
	const char *str = script_getstr(st, 2);
	struct map_session_data *sd = script->rid2sd(st);
	enum logmes_type type = LOGMES_NPC;
	nullpo_retr(false, sd);

	if (script_hasdata(st, 3)) {
		type = script_getnum(st, 3);
	}

	switch (type) {
	case LOGMES_ATCOMMAND:
		logs->atcommand(sd, str);
		break;
	case LOGMES_NPC:
		logs->npc(sd, str);
		break;
	default:
		ShowError("script:logmes: Unknown log type!\n");
		st->state = END;
		return false;
	}

	return true;
}

/**
 * Summons a mob which will act as a slave for the invoking character.
 *
 * @code{.herc}
 *	summon("mob name", <mob id>{, <timeout>{, "event label"}});
 * @endcode
 *
 * @author Celest
 *
 **/
static BUILDIN(summon)
{
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	const int64 tick = timer->gettick();

	clif->skill_poseffect(&sd->bl, AM_CALLHOMUN, 1, sd->bl.x, sd->bl.y, tick);

	const char *event = "";

	if (script_hasdata(st, 5)) {
		event = script_getstr(st, 5);
		script->check_event(st, event);
	}

	const char *name = script_getstr(st, 2);
	const int mob_id = script_getnum(st, 3);
	struct mob_data *md = mob->once_spawn_sub(&sd->bl, sd->bl.m, sd->bl.x, sd->bl.y, name, mob_id, event,
						  SZ_SMALL, AI_NONE, 0);

	if (md != NULL) {
		md->master_id = sd->bl.id;
		md->special_state.ai = AI_ATTACK;

		if (md->deletetimer != INVALID_TIMER)
			timer->delete(md->deletetimer, mob->timer_delete);

		const int timeout = script_hasdata(st, 4) ? script_getnum(st, 4) * 1000 : 60000;

		md->deletetimer = timer->add(tick + ((timeout == 0) ? 60000 : timeout), mob->timer_delete, md->bl.id, 0);
		mob->spawn(md);
		clif->specialeffect(&md->bl, 344, AREA);
		sc_start4(NULL, &md->bl, SC_MODECHANGE, 100, 1, 0, MD_AGGRESSIVE, 0, 60000);
	}

	return true;
}

/*==========================================
 * Checks whether it is daytime/nighttime
 *------------------------------------------*/
static BUILDIN(isnight)
{
	script_pushint(st,(map->night_flag == 1));
	return true;
}

/*================================================
 * Check how many items/cards in the list are
 * equipped - used for 2/15's cards patch [celest]
 *------------------------------------------------*/
static BUILDIN(isequippedcnt)
{
	int i, j, k, id = 1;
	int ret = 0;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	for (i=0; id!=0; i++) {
		script_fetch(st,i+2, id);
		if (id <= 0)
			continue;

		for (j=0; j<EQI_MAX; j++) {
			int index;
			index = sd->equip_index[j];
			if(index < 0) continue;
			if(j == EQI_HAND_R && sd->equip_index[EQI_HAND_L] == index) continue;
			if(j == EQI_HEAD_MID && sd->equip_index[EQI_HEAD_LOW] == index) continue;
			if(j == EQI_HEAD_TOP && (sd->equip_index[EQI_HEAD_MID] == index || sd->equip_index[EQI_HEAD_LOW] == index)) continue;
			if(j == EQI_COSTUME_MID && sd->equip_index[EQI_COSTUME_LOW] == index) continue;
			if(j == EQI_COSTUME_TOP && (sd->equip_index[EQI_COSTUME_MID] == index || sd->equip_index[EQI_COSTUME_LOW] == index)) continue;

			if(!sd->inventory_data[index])
				continue;

			if (itemdb_type(id) != IT_CARD) { //No card. Count amount in inventory.
				if (sd->inventory_data[index]->nameid == id)
					ret+= sd->status.inventory[index].amount;
			} else { //Count cards.
				if (itemdb_isspecial(sd->status.inventory[index].card[0]))
					continue; //No cards
				for(k=0; k<sd->inventory_data[index]->slot; k++) {
					if (sd->status.inventory[index].card[k] == id)
						ret++; //[Lupus]
				}
			}
		}
	}

	script_pushint(st,ret);
	return true;
}

/*================================================
 * Check whether another card has been
 * equipped - used for 2/15's cards patch [celest]
 * -- Items checked cannot be reused in another
 * card set to prevent exploits
 *------------------------------------------------*/
static BUILDIN(isequipped)
{
	int i, j, k, id = 1;
	int index, flag;
	int ret = -1;
	//Original hash to reverse it when full check fails.
	unsigned int setitem_hash = 0, setitem_hash2 = 0;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	setitem_hash = sd->bonus.setitem_hash;
	setitem_hash2 = sd->bonus.setitem_hash2;
	for (i=0; id!=0; i++) {
		script_fetch(st,i+2, id);
		if (id <= 0)
			continue;
		flag = 0;
		for (j=0; j<EQI_MAX; j++) {
			index = sd->equip_index[j];
			if(index < 0) continue;
			if(j == EQI_HAND_R && sd->equip_index[EQI_HAND_L] == index) continue;
			if(j == EQI_HEAD_MID && sd->equip_index[EQI_HEAD_LOW] == index) continue;
			if(j == EQI_HEAD_TOP && (sd->equip_index[EQI_HEAD_MID] == index || sd->equip_index[EQI_HEAD_LOW] == index)) continue;
			if(j == EQI_COSTUME_MID && sd->equip_index[EQI_COSTUME_LOW] == index) continue;
			if(j == EQI_COSTUME_TOP && (sd->equip_index[EQI_COSTUME_MID] == index || sd->equip_index[EQI_COSTUME_LOW] == index)) continue;

			if(!sd->inventory_data[index])
				continue;

			if (itemdb_type(id) != IT_CARD) {
				if (sd->inventory_data[index]->nameid != id)
					continue;
				flag = 1;
				break;
			} else { //Cards
				if (sd->inventory_data[index]->slot == 0 ||
					itemdb_isspecial(sd->status.inventory[index].card[0]))
					continue;

				for (k = 0; k < sd->inventory_data[index]->slot; k++) {
					//New hash system which should support up to 4 slots on any equipment. [Skotlex]
					unsigned int hash = 0;
					if (sd->status.inventory[index].card[k] != id)
						continue;

					hash = 1<<((j<5?j:j-5)*4 + k);
					// check if card is already used by another set
					if ( ( j < 5 ? sd->bonus.setitem_hash : sd->bonus.setitem_hash2 ) & hash)
						continue;

					// We have found a match
					flag = 1;
					// Set hash so this card cannot be used by another
					if (j<5)
						sd->bonus.setitem_hash |= hash;
					else
						sd->bonus.setitem_hash2 |= hash;
					break;
				}
			}
			if (flag) break; //Card found
		}
		if (ret == -1)
			ret = flag;
		else
			ret &= flag;
		if (!ret) break;
	}
	if (!ret) {//When check fails, restore original hash values. [Skotlex]
		sd->bonus.setitem_hash = setitem_hash;
		sd->bonus.setitem_hash2 = setitem_hash2;
	}
	script_pushint(st,ret);
	return true;
}

/*================================================
 * Check how many given inserted cards in the CURRENT
 * weapon - used for 2/15's cards patch [Lupus]
 *------------------------------------------------*/
static BUILDIN(cardscnt)
{
	int i, k, id = 1;
	int ret = 0;
	int index;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	for (i=0; id!=0; i++) {
		script_fetch(st,i+2, id);
		if (id <= 0)
			continue;

		index = status->current_equip_item_index; //we get CURRENT WEAPON inventory index from status.c [Lupus]
		if(index < 0) continue;

		if(!sd->inventory_data[index])
			continue;

		if(itemdb_type(id) != IT_CARD) {
			if (sd->inventory_data[index]->nameid == id)
				ret+= sd->status.inventory[index].amount;
		} else {
			if (itemdb_isspecial(sd->status.inventory[index].card[0]))
				continue;
			for(k=0; k<sd->inventory_data[index]->slot; k++) {
				if (sd->status.inventory[index].card[k] == id)
					ret++;
			}
		}
	}
	script_pushint(st,ret);
	// script_pushint(st,status->current_equip_item_index);
	return true;
}

/*=======================================================
 * Returns the refined number of the current item, or an
 * item with inventory index specified
 *-------------------------------------------------------*/
static BUILDIN(getrefine)
{
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	if (status->current_equip_item_index < 0)
		script_pushint(st, 0);
	else
		script_pushint(st, sd->status.inventory[status->current_equip_item_index].refine);
	return true;
}

/*=======================================================
 * Day/Night controls
 *-------------------------------------------------------*/
static BUILDIN(night)
{
	if (map->night_flag != 1) pc->map_night_timer(pc->night_timer_tid, 0, 0, 1);
	return true;
}
static BUILDIN(day)
{
	if (map->night_flag != 0) pc->map_day_timer(pc->day_timer_tid, 0, 0, 1);
	return true;
}

//=======================================================
// Unequip [Spectre]
//-------------------------------------------------------
static BUILDIN(unequip)
{
	size_t num;
	struct map_session_data *sd;

	num = script_getnum(st,2);
	sd = script->rid2sd(st);
	if (sd != NULL && num >= 1 && num <= ARRAYLENGTH(script->equip)) {
		int i = pc->checkequip(sd,script->equip[num-1]);
		if (i >= 0)
			pc->unequipitem(sd, i, PCUNEQUIPITEM_RECALC|PCUNEQUIPITEM_FORCE);
	}
	return true;
}

static BUILDIN(equip)
{
	int nameid=0,i;
	struct item_data *item_data;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return false;

	nameid=script_getnum(st,2);
	if((item_data = itemdb->exists(nameid)) == NULL)
	{
		ShowError("wrong item ID : equipitem(%d)\n",nameid);
		return false;
	}
	ARR_FIND(0, sd->status.inventorySize, i, sd->status.inventory[i].nameid == nameid && sd->status.inventory[i].equip == 0);
	if (i < sd->status.inventorySize)
		pc->equipitem(sd, i, item_data->equip);

	return true;
}

static BUILDIN(autoequip)
{
	int nameid, flag;
	struct item_data *item_data;
	nameid=script_getnum(st,2);
	flag=script_getnum(st,3);

	if( ( item_data = itemdb->exists(nameid) ) == NULL )
	{
		ShowError("buildin_autoequip: Invalid item '%d'.\n", nameid);
		return false;
	}

	if( !itemdb->isequip2(item_data) )
	{
		ShowError("buildin_autoequip: Item '%d' cannot be equipped.\n", nameid);
		return false;
	}

	item_data->flag.autoequip = flag>0?1:0;
	return true;
}

/*=======================================================
 * Equip2
 * equip2 <item id>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>;
 *-------------------------------------------------------*/
static BUILDIN(equip2)
{
	int i,nameid,ref,attr,c0,c1,c2,c3;
	struct item_data *item_data;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL) {
		script_pushint(st,0);
		return true;
	}

	nameid = script_getnum(st,2);
	if( (item_data = itemdb->exists(nameid)) == NULL )
	{
		ShowError("Wrong item ID : equip2(%i)\n",nameid);
		script_pushint(st,0);
		return false;
	}

	ref  = script_getnum(st, 3);
	attr = script_getnum(st, 4);
	c0   = script_getnum(st, 5);
	c1   = script_getnum(st, 6);
	c2   = script_getnum(st, 7);
	c3   = script_getnum(st, 8);

	ARR_FIND(0, sd->status.inventorySize, i, (sd->status.inventory[i].equip == 0 &&
									sd->status.inventory[i].nameid == nameid &&
									sd->status.inventory[i].refine == ref &&
									sd->status.inventory[i].attribute == attr &&
									sd->status.inventory[i].card[0] == c0 &&
									sd->status.inventory[i].card[1] == c1 &&
									sd->status.inventory[i].card[2] == c2 &&
									sd->status.inventory[i].card[3] == c3));

	if (i < sd->status.inventorySize) {
		script_pushint(st,1);
		pc->equipitem(sd,i,item_data->equip);
	} else {
		script_pushint(st,0);
	}

	return true;
}

static BUILDIN(setbattleflag)
{
	const char *flag, *value;

	flag = script_getstr(st,2);
	value = script_getstr(st,3);  // HACK: Retrieve number as string (auto-converted) for battle_set_value

	if (battle->config_set_value(flag, value) == 0)
		ShowWarning("buildin_setbattleflag: unknown battle_config flag '%s'\n",flag);
	else
		ShowInfo("buildin_setbattleflag: battle_config flag '%s' is now set to '%s'.\n",flag,value);

	return true;
}

static BUILDIN(getbattleflag)
{
	const char *flag;
	int value;

	flag = script_getstr(st,2);

	if (battle->config_get_value(flag, &value)) {
		script_pushint(st,value);
		return true;
	} else {
		script_pushint(st,0);
		ShowWarning("buildin_getbattleflag: non-exist battle config requested %s \n", flag);
		return false;
	}

	return true;
}

//=======================================================
// strlen [Valaris]
//-------------------------------------------------------
static BUILDIN(getstrlen)
{

	const char *str = script_getstr(st,2);
	int len = (str) ? (int)strlen(str) : 0;

	script_pushint(st,len);
	return true;
}

//=======================================================
// isalpha [Valaris]
//-------------------------------------------------------
static BUILDIN(charisalpha)
{
	const char *str=script_getstr(st,2);
	int pos=script_getnum(st,3);

	int val = ( str && pos >= 0 && (unsigned int)pos < strlen(str) ) ? ISALPHA( str[pos] ) != 0 : 0;

	script_pushint(st,val);
	return true;
}

//=======================================================
// charisupper <str>, <index>
//-------------------------------------------------------
static BUILDIN(charisupper)
{
	const char *str = script_getstr(st,2);
	int pos = script_getnum(st,3);

	int val = ( str && pos >= 0 && (unsigned int)pos < strlen(str) ) ? ISUPPER( str[pos] ) : 0;

	script_pushint(st,val);
	return true;
}

//=======================================================
// charislower <str>, <index>
//-------------------------------------------------------
static BUILDIN(charislower)
{
	const char *str = script_getstr(st,2);
	int pos = script_getnum(st,3);

	int val = ( str && pos >= 0 && (unsigned int)pos < strlen(str) ) ? ISLOWER( str[pos] ) : 0;

	script_pushint(st,val);
	return true;
}

//=======================================================
// charat <str>, <index>
//-------------------------------------------------------
static BUILDIN(charat)
{
	const char *str = script_getstr(st,2);
	int pos = script_getnum(st,3);

	if( pos >= 0 && (unsigned int)pos < strlen(str) ) {
		char output[2];
		output[0] = str[pos];
		output[1] = '\0';
		script_pushstrcopy(st, output);
	} else
		script_pushconststr(st, "");
	return true;
}

//=======================================================
// isstr <argument>
//
// returns type:
// 0 - int
// 1 - string
// 2 - other
//-------------------------------------------------------
static BUILDIN(isstr)
{
	if (script_isinttype(st, 2)) {
		script_pushint(st, 0);
	} else if (script_isstringtype(st, 2)) {
		script_pushint(st, 1);
	} else {
		script_pushint(st, 2);
	}
	return true;
}

enum datatype {
	DATATYPE_NIL    = 1 << 7, // we don't start at 1, to leave room for primitives
	DATATYPE_STR    = 1 << 8,
	DATATYPE_INT    = 1 << 9,
	DATATYPE_CONST  = 1 << 10,
	DATATYPE_PARAM  = 1 << 11,
	DATATYPE_VAR    = 1 << 12,
	DATATYPE_LABEL  = 1 << 13,
};

static BUILDIN(getdatatype)
{
	int type;

	if (script_hasdata(st, 2)) {
		struct script_data *data = script_getdata(st, 2);

		if (data_isstring(data)) {
			type = DATATYPE_STR;
			if (data->type == C_CONSTSTR) {
				type |= DATATYPE_CONST;
			}
		} else if (data_isint(data)) {
			type = DATATYPE_INT;
		} else if (data_islabel(data)) {
			type = DATATYPE_LABEL;
		} else if (data_isreference(data)) {
			if (reference_toconstant(data)) {
				type = DATATYPE_CONST | DATATYPE_INT;
			} else if (reference_toparam(data)) {
				type = DATATYPE_PARAM | DATATYPE_INT;
			} else if (reference_tovariable(data)) {
				type = DATATYPE_VAR;
				if (is_string_variable(reference_getname(data))) {
					type |= DATATYPE_STR;
				} else {
					type |= DATATYPE_INT;
				}
			} else {
				ShowError("script:getdatatype: Unknown reference type!\n");
				script->reportdata(data);
				st->state = END;
				return false;
			}
		} else {
			type = data->type; // fallback to primitive type if unknown
		}
	} else {
		type = DATATYPE_NIL; // nothing was passed
	}

	script_pushint(st, type);
	return true;
}

static BUILDIN(data_to_string)
{
	if (script_hasdata(st, 2)) {
		struct script_data *data = script_getdata(st, 2);

		if (data_isstring(data)) {
			script_pushcopy(st, 2);
		} else if (data_isint(data)) {
			char *str = NULL;

			CREATE(str, char, 20);
			safesnprintf(str, 20, "%"PRId64"", data->u.num);
			script_pushstr(st, str);
		} else if (data_islabel(data)) {
			const char *str = "";

			// XXX: because all we have is the label pos we can't be sure which
			//      one is the correct label if more than one has the same pos.
			//      We might want to store both the pos and str_data index in
			//      data->u.num, similar to how C_NAME stores both the array
			//      index and str_data index in u.num with bitmasking. This
			//      would also avoid the awkward for() loops as we could
			//      directly access the string with script->get_str().

			if (st->oid) {
				struct npc_data *nd = map->id2nd(st->oid);

				for (int i = 0; i < nd->u.scr.label_list_num; ++i) {
					if (nd->u.scr.label_list[i].pos == data->u.num) {
						str = nd->u.scr.label_list[i].name;
						break;
					}
				}
			} else {
				for (int i = LABEL_START; script->str_data[i].next != 0; i = script->str_data[i].next) {
					if (script->str_data[i].label == data->u.num) {
						str = script->get_str(i);
						break;
					}
				}
			}

			script_pushconststr(st, str);
		} else if (data_isreference(data)) {
			script_pushstrcopy(st, reference_getname(data));
		} else {
			ShowWarning("script:data_to_string: unknown data type!\n");
			script->reportdata(data);
			script_pushconststr(st, "");
		}
	} else {
		script_pushconststr(st, ""); // NIL
	}

	return true;
}

//=======================================================
// chr <int>
//-------------------------------------------------------
static BUILDIN(chr)
{
	char output[2];
	output[0] = script_getnum(st, 2);
	output[1] = '\0';

	script_pushstrcopy(st, output);
	return true;
}

//=======================================================
// ord <chr>
//-------------------------------------------------------
static BUILDIN(ord)
{
	const char *chr = script_getstr(st, 2);
	script_pushint(st, *chr);
	return true;
}

//=======================================================
// setchar <string>, <char>, <index>
//-------------------------------------------------------
static BUILDIN(setchar)
{
	const char *str = script_getstr(st,2);
	const char *c = script_getstr(st,3);
	int index = script_getnum(st,4);
	char *output = aStrdup(str);

	if(index >= 0 && index < strlen(output))
		output[index] = *c;

	script_pushstr(st, output);
	return true;
}

//=======================================================
// insertchar <string>, <char>, <index>
//-------------------------------------------------------
static BUILDIN(insertchar)
{
	const char *str = script_getstr(st,2);
	const char *c = script_getstr(st,3);
	int index = script_getnum(st,4);
	char *output;
	size_t len = strlen(str);

	if(index < 0)
		index = 0;
	else if(index > len)
		index = (int)len;

	output = (char*)aMalloc(len + 2);

	memcpy(output, str, index);
	output[index] = c[0];
	memcpy(&output[index+1], &str[index], len - index);
	output[len+1] = '\0';

	script_pushstr(st, output);
	return true;
}

//=======================================================
// delchar <string>, <index>
//-------------------------------------------------------
static BUILDIN(delchar)
{
	const char *str = script_getstr(st,2);
	int index = script_getnum(st,3);
	char *output;
	size_t len = strlen(str);

	if(index < 0 || index > len) {
		//return original
		output = aStrdup(str);
		script_pushstr(st, output);
		return true;
	}

	output = (char*)aMalloc(len);

	memcpy(output, str, index);
	memcpy(&output[index], &str[index+1], len - index);

	script_pushstr(st, output);
	return true;
}

//=======================================================
// strtoupper <str>
//-------------------------------------------------------
static BUILDIN(strtoupper)
{
	const char *str = script_getstr(st,2);
	char *output = aStrdup(str);
	char *cursor = output;

	while (*cursor != '\0') {
		*cursor = TOUPPER(*cursor);
		cursor++;
	}

	script_pushstr(st, output);
	return true;
}

//=======================================================
// strtolower <str>
//-------------------------------------------------------
static BUILDIN(strtolower)
{
	const char *str = script_getstr(st,2);
	char *output = aStrdup(str);
	char *cursor = output;

	while (*cursor != '\0') {
		*cursor = TOLOWER(*cursor);
		cursor++;
	}

	script_pushstr(st, output);
	return true;
}

//=======================================================
// substr <str>, <start>, <end>
//-------------------------------------------------------
static BUILDIN(substr)
{
	const char *str = script_getstr(st,2);
	char *output;
	int start = script_getnum(st,3);
	int end = script_getnum(st,4);

	int len = 0;

	if(start >= 0 && end < strlen(str) && start <= end) {
		len = end - start + 1;
		output = (char*)aMalloc(len + 1);
		memcpy(output, &str[start], len);
	} else
		output = (char*)aMalloc(1);

	output[len] = '\0';

	script_pushstr(st, output);
	return true;
}

//=======================================================
// explode <dest_string_array>, <str>, <delimiter>
// Note: delimiter is limited to 1 char
//-------------------------------------------------------
static BUILDIN(explode)
{
	struct script_data* data = script_getdata(st, 2);
	const char *str = script_getstr(st,3);
	const char delimiter = script_getstr(st, 4)[0];
	int32 id;
	size_t len = strlen(str);
	int i = 0, j = 0, k = 0;
	int start;
	char *temp = NULL;
	const char *name;

	struct map_session_data *sd = NULL;

	if (!data_isreference(data)) {
		ShowError("script:explode: not a variable\n");
		script->reportdata(data);
		st->state = END;
		return false;// not a variable
	}

	id = reference_getid(data);
	start = reference_getindex(data);
	name = reference_getname(data);

	if (!is_string_variable(name)) {
		ShowError("script:explode: not string array\n");
		script->reportdata(data);
		st->state = END;
		return false;// data type mismatch
	}

	if (not_server_variable(*name)) {
		sd = script->rid2sd(st);
		if (sd == NULL)
			return true;// no player attached
	}

	temp = aMalloc(len + 1);

	for (i = 0; str[i] != '\0'; i++) {
		if (str[i] == delimiter && (int64)start + k < (int64)(SCRIPT_MAX_ARRAYSIZE-1)) { // FIXME[Haru]: SCRIPT_MAX_ARRAYSIZE should really be unsigned (and INT32_MAX)
			//break at delimiter but ignore after reaching last array index
			temp[j] = '\0';
			script->set_reg(st, sd, reference_uid(id, start + k), name, temp, reference_getref(data));
			k++;
			j = 0;
		} else {
			temp[j++] = str[i];
		}
	}
	//set last string
	temp[j] = '\0';
	script->set_reg(st, sd, reference_uid(id, start + k), name, temp, reference_getref(data));

	aFree(temp);

	script_pushint(st, k + 1);
	return true;
}

//=======================================================
// implode <string_array>
// implode <string_array>, <glue>
//-------------------------------------------------------
static BUILDIN(implode)
{
	struct script_data* data = script_getdata(st, 2);
	const char *name;
	uint32 array_size, id;

	struct map_session_data *sd = NULL;

	char *output;

	if( !data_isreference(data) )
	{
		ShowError("script:implode: not a variable\n");
		script->reportdata(data);
		st->state = END;
		return false;// not a variable
	}

	id = reference_getid(data);
	name = reference_getname(data);

	if( !is_string_variable(name) )
	{
		ShowError("script:implode: not string array\n");
		script->reportdata(data);
		st->state = END;
		return false;// data type mismatch
	}

	if( not_server_variable(*name) )
	{
		sd = script->rid2sd(st);
		if( sd == NULL )
			return true;// no player attached
	}

	//count chars
	array_size = script->array_highest_key(st,sd,name,reference_getref(data)) - 1;

	if (array_size == -1) {
		//empty array check (AmsTaff)
		ShowWarning("script:implode: array length = 0\n");
		output = (char*)aMalloc(sizeof(char)*5);
		sprintf(output,"%s","NULL");
	} else {
		int i;
		size_t len = 0, glue_len = 0, k = 0;
		const char *glue = NULL, *temp;
		for(i = 0; i <= array_size; ++i) {
			temp = script->get_val2(st, reference_uid(id, i), reference_getref(data));
			len += strlen(temp);
			script_removetop(st, -1, 0);
		}

		//allocate mem
		if( script_hasdata(st,3) ) {
			glue = script_getstr(st,3);
			glue_len = strlen(glue);
			len += glue_len * (array_size);
		}
		output = (char*)aMalloc(len + 1);

		//build output
		for(i = 0; i < array_size; ++i) {
			temp = script->get_val2(st, reference_uid(id, i), reference_getref(data));
			len = strlen(temp);
			memcpy(&output[k], temp, len);
			k += len;
			if(glue_len != 0) {
				memcpy(&output[k], glue, glue_len);
				k += glue_len;
			}
			script_removetop(st, -1, 0);
		}
		temp = script->get_val2(st, reference_uid(id, array_size), reference_getref(data));
		len = strlen(temp);
		memcpy(&output[k], temp, len);
		k += len;
		script_removetop(st, -1, 0);

		output[k] = '\0';
	}

	script_pushstr(st, output);
	return true;
}

//=======================================================
// sprintf(<format>, ...);
// Implements C sprintf, except format %n. The resulting string is
// returned, instead of being saved in variable by reference.
//-------------------------------------------------------
static BUILDIN(sprintf)
{
	struct StringBuf buf;
	StrBuf->Init(&buf);

	if (!script->sprintf_helper(st, 2, &buf)) {
		StrBuf->Destroy(&buf);
		script_pushconststr(st, "");
		return false;
	}

	script_pushstrcopy(st, StrBuf->Value(&buf));
	StrBuf->Destroy(&buf);

	return true;
}

//=======================================================
// sscanf(<str>, <format>, ...);
// Implements C sscanf.
//-------------------------------------------------------
static BUILDIN(sscanf)
{
	unsigned int argc, arg = 0;
	struct script_data* data;
	struct map_session_data* sd = NULL;
	const char* str;
	const char* format;
	const char* p;
	const char* q;
	char* buf = NULL;
	char* buf_p;
	char* ref_str = NULL;
	int ref_int;
	size_t len;

	// Get data
	str = script_getstr(st, 2);
	format = script_getstr(st, 3);
	argc = script_lastdata(st)-3;

	len = strlen(format);

	if (len != 0 && strlen(str) == 0) {
		// If the source string is empty but the format string is not, we return -1
		// according to the C specs. (if the format string is also empty, we shall
		// continue and return 0: 0 conversions took place out of the 0 attempted.)
		script_pushint(st, -1);
		return true;
	}

	CREATE(buf, char, len*2+1);

	// Issue sscanf for each parameter
	*buf = 0;
	q = format;
	while((p = strchr(q, '%'))) {
		if(p!=q) {
			strncat(buf, q, (size_t)(p-q));
			q = p;
		}
		p = q+1;
		if(*p=='*' || *p=='%') {  // Skip
			strncat(buf, q, 2);
			q+=2;
			continue;
		}
		if(arg>=argc) {
			ShowError("buildin_sscanf: Not enough arguments passed!\n");
			script_pushint(st, -1);
			aFree(buf);
			if(ref_str) aFree(ref_str);
			return false;
		}
		if((p = strchr(q+1, '%'))==NULL) {
			p = strchr(q, 0);  // EOS
		}
		len = p-q;
		strncat(buf, q, len);
		q = p;

		// Validate output
		data = script_getdata(st, arg+4);
		if(!data_isreference(data) || !reference_tovariable(data)) {
			ShowError("buildin_sscanf: Target argument is not a variable!\n");
			script_pushint(st, -1);
			aFree(buf);
			if(ref_str) aFree(ref_str);
			return false;
		}
		buf_p = reference_getname(data);
		if(not_server_variable(*buf_p) && (sd = script->rid2sd(st))==NULL) {
			script_pushint(st, -1);
			aFree(buf);
			if(ref_str) aFree(ref_str);
			return true;
		}

		// Save value if any
		if(buf_p[strlen(buf_p)-1]=='$') {  // String
			if(ref_str==NULL) {
				CREATE(ref_str, char, strlen(str)+1);
			}
			if(sscanf(str, buf, ref_str)==0) {
				break;
			}
			script->set_reg(st, sd, reference_uid( reference_getid(data), reference_getindex(data) ), buf_p, ref_str, reference_getref(data));
		} else {  // Number
			if(sscanf(str, buf, &ref_int)==0) {
				break;
			}
			script->set_reg(st, sd, reference_uid( reference_getid(data), reference_getindex(data) ), buf_p, (const void *)h64BPTRSIZE(ref_int), reference_getref(data));
		}
		arg++;

		// Disable used format (%... -> %*...)
		buf_p = strchr(buf, 0);
		memmove(buf_p-len+2, buf_p-len+1, len);
		*(buf_p-len+1) = '*';
	}

	script_pushint(st, arg);
	aFree(buf);
	if(ref_str) aFree(ref_str);

	return true;
}

//=======================================================
// strpos(<haystack>, <needle>)
// strpos(<haystack>, <needle>, <offset>)
//
// Implements PHP style strpos. Adapted from code from
// http://www.daniweb.com/code/snippet313.html, Dave Sinkula
//-------------------------------------------------------
static BUILDIN(strpos)
{
	const char *haystack = script_getstr(st,2);
	const char *needle = script_getstr(st,3);
	int i;
	size_t len;

	if( script_hasdata(st,4) )
		i = script_getnum(st,4);
	else
		i = 0;

	if (needle[0] == '\0') {
		script_pushint(st, -1);
		return true;
	}

	len = strlen(haystack);
	for ( ; i < len; ++i ) {
		if ( haystack[i] == *needle ) {
			// matched starting char -- loop through remaining chars
			const char *h, *n;
			for ( h = &haystack[i], n = needle; *h && *n; ++h, ++n ) {
				if ( *h != *n ) {
					break;
				}
			}
			if ( !*n ) { // matched all of 'needle' to null termination
				script_pushint(st, i);
				return true;
			}
		}
	}
	script_pushint(st, -1);
	return true;
}

//===============================================================
// replacestr <input>, <search>, <replace>{, <usecase>{, <count>}}
//
// Note: Finds all instances of <search> in <input> and replaces
// with <replace>. If specified will only replace as many
// instances as specified in <count>. By default will be case
// sensitive.
//---------------------------------------------------------------
static BUILDIN(replacestr)
{
	const char *input = script_getstr(st, 2);
	const char *find = script_getstr(st, 3);
	const char *replace = script_getstr(st, 4);
	size_t inputlen = strlen(input);
	size_t findlen = strlen(find);
	struct StringBuf output;
	bool usecase = true;

	int count = 0;
	int numFinds = 0;
	int i = 0, f = 0;

	if(findlen == 0) {
		ShowError("script:replacestr: Invalid search length.\n");
		st->state = END;
		return false;
	}

	if(script_hasdata(st, 5)) {
		if( script_isinttype(st,5) ) {
			usecase = script_getnum(st, 5) != 0;
		} else {
			ShowError("script:replacestr: Invalid usecase value. Expected int.\n");
			st->state = END;
			return false;
		}
	}

	if(script_hasdata(st, 6)) {
		if (!script_isinttype(st, 6) || (count = script_getnum(st, 6)) == 0) {
			ShowError("script:replacestr: Invalid count value. Expected int.\n");
			st->state = END;
			return false;
		}
	}

	StrBuf->Init(&output);

	for(; i < inputlen; i++) {
		if(count && count == numFinds) {
			break; //found enough, stop looking
		}

		for(f = 0; f <= findlen; f++) {
			if(f == findlen) { //complete match
				numFinds++;
				StrBuf->AppendStr(&output, replace);

				i += findlen - 1;
				break;
			} else {
				if(usecase) {
					if((i + f) > inputlen || input[i + f] != find[f]) {
						StrBuf->Printf(&output, "%c", input[i]);
						break;
					}
				} else {
					if(((i + f) > inputlen || input[i + f] != find[f]) && TOUPPER(input[i+f]) != TOUPPER(find[f])) {
						StrBuf->Printf(&output, "%c", input[i]);
						break;
					}
				}
			}
		}
	}

	//append excess after enough found
	if(i < inputlen)
		StrBuf->AppendStr(&output, &(input[i]));

	script_pushstrcopy(st, StrBuf->Value(&output));
	StrBuf->Destroy(&output);
	return true;
}

//========================================================
// countstr <input>, <search>{, <usecase>}
//
// Note: Counts the number of times <search> occurs in
// <input>. By default will be case sensitive.
//--------------------------------------------------------
static BUILDIN(countstr)
{
	const char *input = script_getstr(st, 2);
	const char *find = script_getstr(st, 3);
	size_t inputlen = strlen(input);
	size_t findlen = strlen(find);
	bool usecase = true;

	int numFinds = 0;
	int i = 0, f = 0;

	if(findlen == 0) {
		ShowError("script:countstr: Invalid search length.\n");
		st->state = END;
		return false;
	}

	if(script_hasdata(st, 4)) {
		if( script_isinttype(st,4) )
			usecase = script_getnum(st, 4) != 0;
		else {
			ShowError("script:countstr: Invalid usecase value. Expected int.\n");
			st->state = END;
			return false;
		}
	}

	for(; i < inputlen; i++) {
		for(f = 0; f <= findlen; f++) {
			if(f == findlen) { //complete match
				numFinds++;
				i += findlen - 1;
				break;
			} else {
				if(usecase) {
					if((i + f) > inputlen || input[i + f] != find[f]) {
						break;
					}
				} else {
					if(((i + f) > inputlen || input[i + f] != find[f]) && TOUPPER(input[i+f]) != TOUPPER(find[f])) {
						break;
					}
				}
			}
		}
	}
	script_pushint(st, numFinds);
	return true;
}

/// Changes the display name and/or display class of the npc.
/// Returns 0 is successful, 1 if the npc does not exist.
///
/// setnpcdisplay("<npc name>", "<new display name>", <new class id>, <new size>) -> <int>
/// setnpcdisplay("<npc name>", "<new display name>", <new class id>) -> <int>
/// setnpcdisplay("<npc name>", "<new display name>") -> <int>
/// setnpcdisplay("<npc name>", <new class id>) -> <int>
static BUILDIN(setnpcdisplay)
{
	const char* name;
	const char* newname = NULL;
	int class_ = -1, size = -1;
	struct npc_data* nd;

	name = script_getstr(st,2);

	if( script_hasdata(st,4) )
		class_ = script_getnum(st,4);
	if( script_hasdata(st,5) )
		size = script_getnum(st,5);

	if( script_isstringtype(st, 3) )
		newname = script_getstr(st, 3);
	else
		class_ = script_getnum(st, 3);

	nd = npc->name2id(name);
	if( nd == NULL )
	{// not found
		script_pushint(st,1);
		return true;
	}

	// update npc
	if( newname )
		npc->setdisplayname(nd, newname);

	if( size != -1 && size != (int)nd->size )
		nd->size = size;
	else
		size = -1;

	if( class_ != -1 && nd->class_ != class_ )
		npc->setclass(nd, class_);
	else if( size != -1 )
	{ // Required to update the visual size
		clif->clearunit_area(&nd->bl, CLR_OUTSIGHT);
		clif->spawn(&nd->bl);
	}

	script_pushint(st,0);
	return true;
}

static BUILDIN(atoi)
{
	const char *value;
	value = script_getstr(st,2);
	script_pushint(st,atoi(value));
	return true;
}

static BUILDIN(axtoi)
{
	const char *hex = script_getstr(st,2);
	long value = strtol(hex, NULL, 16);
#if LONG_MAX > INT_MAX || LONG_MIN < INT_MIN
	value = cap_value(value, INT_MIN, INT_MAX);
#endif
	script_pushint(st, (int)value);
	return true;
}

static BUILDIN(strtol)
{
	const char *string = script_getstr(st, 2);
	int base = script_getnum(st, 3);
	long value = strtol(string, NULL, base);
#if LONG_MAX > INT_MAX || LONG_MIN < INT_MIN
	value = cap_value(value, INT_MIN, INT_MAX);
#endif
	script_pushint(st, (int)value);
	return true;
}

// case-insensitive substring search [lordalfa]
static BUILDIN(compare)
{
	const char *message;
	const char *cmpstring;
	message = script_getstr(st,2);
	cmpstring = script_getstr(st,3);
	script_pushint(st,(stristr(message,cmpstring) != NULL));
	return true;
}

static BUILDIN(strcmp)
{
	const char *str1 = script_getstr(st,2);
	const char *str2 = script_getstr(st,3);
	script_pushint(st,strcmp(str1, str2));
	return true;
}

// List of mathematics commands --->

static BUILDIN(log10)
{
	double i, a;
	i = script_getnum(st,2);
	a = log10(i);
	script_pushint(st,(int)a);
	return true;
}

static BUILDIN(sqrt) //[zBuffer]
{
	double i, a;
	i = script_getnum(st,2);
	if (i < 0) {
		ShowError("sqrt from negative value\n");
		return false;
	}
	a = sqrt(i);
	script_pushint(st,(int)a);
	return true;
}

static BUILDIN(pow) //[zBuffer]
{
	double i, a, b;
	a = script_getnum(st,2);
	b = script_getnum(st,3);
	i = pow(a,b);
	script_pushint(st,(int)i);
	return true;
}

static BUILDIN(distance) //[zBuffer]
{
	int x0, y0, x1, y1;

	x0 = script_getnum(st,2);
	y0 = script_getnum(st,3);
	x1 = script_getnum(st,4);
	y1 = script_getnum(st,5);

	script_pushint(st,distance_xy(x0,y0,x1,y1));
	return true;
}

// <--- List of mathematics commands

static BUILDIN(min)
{
	int i, min;

	min = script_getnum(st, 2);
	for (i = 3; script_hasdata(st, i); i++) {
		int next = script_getnum(st, i);
		if (next < min)
			min = next;
	}
	script_pushint(st, min);

	return true;
}

static BUILDIN(max)
{
	int i, max;

	max = script_getnum(st, 2);
	for (i = 3; script_hasdata(st, i); i++) {
		int next = script_getnum(st, i);
		if (next > max)
			max = next;
	}
	script_pushint(st, max);

	return true;
}

static BUILDIN(cap_value)
{
	int value = script_getnum(st, 2);
	int min = script_getnum(st, 3);
	int max = script_getnum(st, 4);

	script_pushint(st, (int)cap_value(value, min, max));

	return true;
}

static BUILDIN(md5)
{
	const char *tmpstr;
	char *md5str;

	tmpstr = script_getstr(st,2);
	md5str = (char *)aMalloc((32+1)*sizeof(char));
	md5->string(tmpstr, md5str);
	script_pushstr(st, md5str);
	return true;
}

static BUILDIN(swap)
{
	struct map_session_data *sd = NULL;
	struct script_data *data1, *data2;
	struct reg_db *ref1, *ref2;
	const char *varname1, *varname2;
	int64 uid1, uid2;

	data1 = script_getdata(st,2);
	data2 = script_getdata(st,3);

	if (!data_isreference(data1) || !data_isreference(data2) || reference_toconstant(data1) || reference_toconstant(data2)) {
		st->state = END;
		return true; // avoid print error message twice
	}

	if (reference_toparam(data1) || reference_toparam(data2)) {
		ShowError("script:swap: detected parameter type constant.\n");
		if (reference_toparam(data1))
			script->reportdata(data1);
		if (reference_toparam(data2))
			script->reportdata(data2);
		st->state = END;
		return false;
	}

	varname1 = reference_getname(data1);
	varname2 = reference_getname(data2);

	if ((is_string_variable(varname1) && !is_string_variable(varname2)) || (!is_string_variable(varname1) && is_string_variable(varname2))) {
		ShowError("script:swap: both sides must be same integer or string type.\n");
		script->reportdata(data1);
		script->reportdata(data2);
		st->state = END;
		return false;
	}

	if (not_server_variable(*varname1) || not_server_variable(*varname2)) {
		sd = script->rid2sd(st);
		if (sd == NULL)
			return true; // avoid print error message twice
	}

	uid1 = reference_getuid(data1);
	uid2 = reference_getuid(data2);
	ref1 = reference_getref(data1);
	ref2 = reference_getref(data2);

	if (is_string_variable(varname1)) {
		const char *value1, *value2;

		value1 = script_getstr(st,2);
		value2 = script_getstr(st,3);

		if (strcmpi(value1, value2)) {
			script->set_reg(st, sd, uid1, varname1, value2, ref1);
			script->set_reg(st, sd, uid2, varname2, value1, ref2);
		}
	}
	else {
		int value1, value2;

		value1 = script_getnum(st,2);
		value2 = script_getnum(st,3);

		if (value1 != value2) {
			script->set_reg(st, sd, uid1, varname1, (const void *)h64BPTRSIZE(value2), ref1);
			script->set_reg(st, sd, uid2, varname2, (const void *)h64BPTRSIZE(value1), ref2);
		}
	}
	return true;
}

// [zBuffer] List of dynamic var commands --->

static BUILDIN(setd)
{
	struct map_session_data *sd = NULL;
	char varname[100];
	const char *buffer;
	int elem;
	buffer = script_getstr(st, 2);

	if(sscanf(buffer, "%99[^[][%d]", varname, &elem) < 2)
		elem = 0;

	if( not_server_variable(*varname) )
	{
		sd = script->rid2sd(st);
		if( sd == NULL )
		{
			ShowError("script:setd: no player attached for player variable '%s'\n", buffer);
			return true;
		}
	}

	if (is_string_variable(varname)) {
		script->setd_sub(st, sd, varname, elem, script_getstr(st, 3), NULL);
	} else {
		script->setd_sub(st, sd, varname, elem, (const void *)h64BPTRSIZE(script_getnum(st, 3)), NULL);
	}

	return true;
}

static int buildin_query_sql_sub(struct script_state *st, struct Sql *handle)
{
	int i, j;
	struct map_session_data *sd = NULL;
	const char* query;
	struct script_data* data;
	const char* name;
	unsigned int max_rows = SCRIPT_MAX_ARRAYSIZE; // maximum number of rows
	int num_vars;
	int num_cols;

	// check target variables
	for( i = 3; script_hasdata(st,i); ++i ) {
		data = script_getdata(st, i);
		if( data_isreference(data) ) { // it's a variable
			name = reference_getname(data);
			if( not_server_variable(*name) && sd == NULL ) { // requires a player
				sd = script->rid2sd(st);
				if( sd == NULL )// no player attached
					return false;
			}
		} else {
			ShowError("script:query_sql: not a variable\n");
			script->reportdata(data);
			st->state = END;
			return false;
		}
	}
	num_vars = i - 3;

	// Execute the query
	query = script_getstr(st,2);

	if( SQL_ERROR == SQL->QueryStr(handle, query) ) {
		Sql_ShowDebug(handle);
		st->state = END;
		return false;
	}

	if( SQL->NumRows(handle) == 0 ) { // No data received
		SQL->FreeResult(handle);
		script_pushint(st, 0);
		return true;
	}

	// Count the number of columns to store
	num_cols = SQL->NumColumns(handle);
	if( num_vars < num_cols ) {
		ShowWarning("script:query_sql: Too many columns, discarding last %u columns.\n", (unsigned int)(num_cols-num_vars));
		script->reportsrc(st);
	} else if( num_vars > num_cols ) {
		ShowWarning("script:query_sql: Too many variables (%u extra).\n", (unsigned int)(num_vars-num_cols));
		script->reportsrc(st);
	}

	// Store data
	for( i = 0; i < max_rows && SQL_SUCCESS == SQL->NextRow(handle); ++i ) {
		for( j = 0; j < num_vars; ++j ) {
			char* str = NULL;

			if( j < num_cols )
				SQL->GetData(handle, j, &str, NULL);

			data = script_getdata(st, j+3);
			name = reference_getname(data);
			if( is_string_variable(name) )
				script->setd_sub(st, sd, name, i, (void *)(str?str:""), reference_getref(data));
			else
				script->setd_sub(st, sd, name, i, (void *)h64BPTRSIZE((str?atoi(str):0)), reference_getref(data));
		}
	}
	if( i == max_rows && max_rows < SQL->NumRows(handle) ) {
		ShowWarning("script:query_sql: Only %u/%u rows have been stored.\n", max_rows, (unsigned int)SQL->NumRows(handle));
		script->reportsrc(st);
	}

	// Free data
	SQL->FreeResult(handle);
	script_pushint(st, i);

	return true;
}
static BUILDIN(query_sql)
{
	return script->buildin_query_sql_sub(st, map->mysql_handle);
}

static BUILDIN(query_logsql)
{
	if( !logs->config.sql_logs ) {// logs->mysql_handle == NULL
		ShowWarning("buildin_query_logsql: SQL logs are disabled, query '%s' will not be executed.\n", script_getstr(st,2));
		script_pushint(st,-1);
		return false;
	}
	return script->buildin_query_sql_sub(st, logs->mysql_handle);
}

//Allows escaping of a given string.
static BUILDIN(escape_sql)
{
	const char *str;
	char *esc_str;
	size_t len;

	str = script_getstr(st,2);
	len = strlen(str);
	esc_str = (char*)aMalloc(len*2+1);
	SQL->EscapeStringLen(map->mysql_handle, esc_str, str, len);
	script_pushstr(st, esc_str);
	return true;
}

static BUILDIN(getd)
{
	char varname[100];
	const char *buffer;
	int elem;
	int id;

	buffer = script_getstr(st, 2);

	if (sscanf(buffer, "%99[^[][%d]", varname, &elem) < 2)
		elem = 0;

	id = script->add_variable(varname);

	if (script->str_data[id].type != C_NAME && // variable
		script->str_data[id].type != C_PARAM && // param
		script->str_data[id].type != C_INT) { // constant
		ShowError("script:getd: `%s` is already used by something that is not a variable.\n", varname);
		st->state = END;
		return false;
	}

	// Push the 'pointer' so it's more flexible [Lance]
	script->push_val(st->stack, C_NAME, reference_uid(id, elem),NULL);

	return true;
}

// <--- [zBuffer] List of dynamic var commands
// Pet stat [Lance]
static BUILDIN(petstat)
{
	struct pet_data *pd;
	int flag = script_getnum(st,2);
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL || sd->status.pet_id == 0 || sd->pd == NULL) {
		if(flag == 2)
			script_pushconststr(st, "");
		else
			script_pushint(st,0);
		return true;
	}
	pd = sd->pd;
	switch(flag) {
		case 1: script_pushint(st,(int)pd->pet.class_); break;
		case 2: script_pushstrcopy(st, pd->pet.name); break;
		case 3: script_pushint(st,(int)pd->pet.level); break;
		case 4: script_pushint(st,(int)pd->pet.hungry); break;
		case 5: script_pushint(st,(int)pd->pet.intimate); break;
		default:
			script_pushint(st,0);
			break;
	}
	return true;
}

static BUILDIN(callshop)
{
	struct npc_data *nd;
	const char *shopname;
	int flag = 0;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;
	shopname = script_getstr(st, 2);
	if( script_hasdata(st,3) )
		flag = script_getnum(st,3);
	nd = npc->name2id(shopname);
	if( !nd || nd->bl.type != BL_NPC || (nd->subtype != SHOP && nd->subtype != CASHSHOP) )
	{
		ShowError("buildin_callshop: Shop [%s] not found (or NPC is not shop type)\n", shopname);
		script_pushint(st,0);
		return false;
	}

	if( nd->subtype == SHOP )
	{
		// flag the user as using a valid script call for opening the shop (for floating NPCs)
		sd->state.callshop = 1;

		switch( flag )
		{
			case 1: npc->buysellsel(sd,nd->bl.id,0); break; //Buy window
			case 2: npc->buysellsel(sd,nd->bl.id,1); break; //Sell window
			default: clif->npcbuysell(sd,nd->bl.id); break; //Show menu
		}
	}
	else
		clif->cashshop_show(sd, nd);

	sd->npc_shopid = nd->bl.id;
	script_pushint(st,1);
	return true;
}

static BUILDIN(npcshopitem)
{
	const char* npcname = script_getstr(st, 2);
	struct npc_data* nd = npc->name2id(npcname);
	int n, i;
	int amount;

	if( !nd || ( nd->subtype != SHOP && nd->subtype != CASHSHOP ) ) {
		//Not found.
		script_pushint(st,0);
		return true;
	}

	// get the count of new entries
	amount = (script_lastdata(st)-2)/2;

	// generate new shop item list
	RECREATE(nd->u.shop.shop_item, struct npc_item_list, amount);
	for( n = 0, i = 3; n < amount; n++, i+=2 )
	{
		nd->u.shop.shop_item[n].nameid = script_getnum(st,i);
		nd->u.shop.shop_item[n].value = script_getnum(st,i+1);
	}
	nd->u.shop.count = n;

	script_pushint(st,1);
	return true;
}

static BUILDIN(npcshopadditem)
{
	const char* npcname = script_getstr(st,2);
	struct npc_data* nd = npc->name2id(npcname);
	int n, i;
	int amount;

	if( !nd || ( nd->subtype != SHOP && nd->subtype != CASHSHOP ) ) {
		//Not found.
		script_pushint(st,0);
		return true;
	}

	// get the count of new entries
	amount = (script_lastdata(st)-2)/2;

	// append new items to existing shop item list
	RECREATE(nd->u.shop.shop_item, struct npc_item_list, nd->u.shop.count+amount);
	for( n = nd->u.shop.count, i = 3; n < nd->u.shop.count+amount; n++, i+=2 )
	{
		nd->u.shop.shop_item[n].nameid = script_getnum(st,i);
		nd->u.shop.shop_item[n].value = script_getnum(st,i+1);
	}
	nd->u.shop.count = n;

	script_pushint(st,1);
	return true;
}

static BUILDIN(npcshopdelitem)
{
	const char* npcname = script_getstr(st,2);
	struct npc_data* nd = npc->name2id(npcname);
	int n, i;
	int amount;
	int size;

	if (!nd || (nd->subtype != SHOP && nd->subtype != CASHSHOP)) {
		//Not found.
		script_pushint(st,0);
		return true;
	}

	amount = script_lastdata(st)-2;
	size = nd->u.shop.count;

	// remove specified items from the shop item list
	for (i = 3; i < 3 + amount; i++) {
		unsigned int nameid = script_getnum(st,i);

		ARR_FIND(0, size, n, nd->u.shop.shop_item[n].nameid == nameid);
		if (n == size) {
			continue;
		} else if (n < size - 1) {
			memmove(&nd->u.shop.shop_item[n], &nd->u.shop.shop_item[n+1], sizeof(nd->u.shop.shop_item[0]) * (size - n - 1));
		}
		size--;
	}

	RECREATE(nd->u.shop.shop_item, struct npc_item_list, size);
	nd->u.shop.count = size;

	script_pushint(st,1);
	return true;
}

//Sets a script to attach to a shop npc.
static BUILDIN(npcshopattach)
{
	const char* npcname = script_getstr(st,2);
	struct npc_data* nd = npc->name2id(npcname);
	int flag = 1;

	if( script_hasdata(st,3) )
		flag = script_getnum(st,3);

	if( !nd || nd->subtype != SHOP ) {
		//Not found.
		script_pushint(st,0);
		return true;
	}

	if (flag)
		nd->master_nd = map->id2nd(st->oid);
	else
		nd->master_nd = NULL;

	script_pushint(st,1);
	return true;
}

/*==========================================
 * Returns some values of an item [Lupus]
 * Price, Weight, etc...
 * setitemscript(itemID,"{new item bonus script}",[n]);
 * Where n:
 * 0 - script
 * 1 - Equip script
 * 2 - Unequip script
 *------------------------------------------*/
static BUILDIN(setitemscript)
{
	int item_id,n=0;
	const char *new_bonus_script;
	struct item_data *i_data;
	struct script_code **dstscript;

	item_id          = script_getnum(st,2);
	new_bonus_script = script_getstr(st,3);
	if( script_hasdata(st,4) )
		n=script_getnum(st,4);
	i_data = itemdb->exists(item_id);

	if (!i_data || new_bonus_script==NULL || ( new_bonus_script[0] && new_bonus_script[0]!='{' )) {
		script_pushint(st,0);
		return true;
	}
	switch (n) {
		case 2:
			dstscript = &i_data->unequip_script;
			break;
		case 1:
			dstscript = &i_data->equip_script;
			break;
		default:
			dstscript = &i_data->script;
			break;
	}
	if(*dstscript)
		script->free_code(*dstscript);

	*dstscript = new_bonus_script[0] ? script->parse(new_bonus_script, "script_setitemscript", 0, 0, NULL) : NULL;
	script_pushint(st,1);
	return true;
}

/*=======================================================
 * Temporarily add or update a mob drop
 * Original Idea By: [Lupus], [Akinari]
 *
 * addmonsterdrop <mob_id or name>,<item_id>,<rate>;
 *
 * If given an item the mob already drops, the rate
 * is updated to the new rate.  Rate must be in the range [1:10000]
 * Returns 1 if succeeded (added/updated a mob drop)
 *-------------------------------------------------------*/
static BUILDIN(addmonsterdrop)
{
	struct mob_db *monster;
	int item_id, rate, i, c = MAX_MOB_DROP;

	if( script_isstringtype(st,2) )
		monster = mob->db(mob->db_searchname(script_getstr(st,2)));
	else
		monster = mob->db(script_getnum(st,2));

	if( monster == mob->dummy ) {
		if( script_isstringtype(st,2) ) {
			ShowError("buildin_addmonsterdrop: invalid mob name: '%s'.\n", script_getstr(st,2));
		} else {
			ShowError("buildin_addmonsterdrop: invalid mob id: '%d'.\n", script_getnum(st,2));
		}
		return false;
	}

	item_id = script_getnum(st,3);
	if( !itemdb->exists(item_id) ) {
		ShowError("buildin_addmonsterdrop: Invalid item ID: '%d'.\n", item_id);
		return false;
	}

	rate = script_getnum(st,4);
	if( rate < 1 || rate > 10000 ) {
		ShowWarning("buildin_addmonsterdrop: Invalid drop rate '%d'. Capping to the [1:10000] range.\n", rate);
		rate = cap_value(rate,1,10000);
	}

	for( i = 0; i < MAX_MOB_DROP; i++ ) {
		if( monster->dropitem[i].nameid == item_id ) // Item ID found
			break;
		if( c == MAX_MOB_DROP && monster->dropitem[i].nameid < 1 ) // First empty slot
			c = i;
	}
	if( i < MAX_MOB_DROP ) // If the item ID was found, prefer it
		c = i;

	if( c < MAX_MOB_DROP ) {
		// Fill in the slot with the item and rate
		monster->dropitem[c].nameid = item_id;
		monster->dropitem[c].p = rate;
		script_pushint(st,1);
	} else {
		//No place to put the new drop
		script_pushint(st,0);
	}

	return true;
}

/*=======================================================
 * Temporarily remove a mob drop
 * Original Idea By: [Lupus], [Akinari]
 *
 * delmonsterdrop <mob_id or name>,<item_id>;
 *
 * Returns 1 if succeeded (deleted a mob drop)
 *-------------------------------------------------------*/
static BUILDIN(delmonsterdrop)
{
	struct mob_db *monster;
	int item_id, i;

	if( script_isstringtype(st, 2) )
		monster = mob->db(mob->db_searchname(script_getstr(st, 2)));
	else
		monster = mob->db(script_getnum(st, 2));

	if( monster == mob->dummy ) {
		if( script_isstringtype(st, 2) ) {
			ShowError("buildin_delmonsterdrop: invalid mob name: '%s'.\n", script_getstr(st,2));
		} else {
			ShowError("buildin_delmonsterdrop: invalid mob id: '%d'.\n", script_getnum(st,2));
		}
		return false;
	}

	item_id = script_getnum(st,3);
	if( !itemdb->exists(item_id) ) {
		ShowError("buildin_delmonsterdrop: Invalid item ID: '%d'.\n", item_id);
		return false;
	}

	for( i = 0; i < MAX_MOB_DROP; i++ ) {
		if( monster->dropitem[i].nameid == item_id ) {
			monster->dropitem[i].nameid = 0;
			monster->dropitem[i].p = 0;
			script_pushint(st,1);
			return true;
		}
	}
	// No drop on that monster
	script_pushint(st,0);
	return true;
}

/*==========================================
 * Returns some values of a monster [Lupus]
 * Name, Level, race, size, etc...
 * getmonsterinfo(monsterID,queryIndex);
 *------------------------------------------*/
static BUILDIN(getmonsterinfo)
{
	struct mob_db *monster;
	int mob_id;

	mob_id = script_getnum(st,2);
	if (!mob->db_checkid(mob_id)) {
		ShowError("buildin_getmonsterinfo: Wrong Monster ID: %i\n", mob_id);
		if ( !script_getnum(st,3) ) //requested a string
			script_pushconststr(st,"null");
		else
			script_pushint(st,-1);
		return false;
	}
	monster = mob->db(mob_id);
	switch ( script_getnum(st,3) ) {
		case 0:  script_pushstrcopy(st,monster->jname); break;
		case 1:  script_pushint(st,monster->lv); break;
		case 2:  script_pushint(st,monster->status.max_hp); break;
		case 3:  script_pushint(st,monster->base_exp); break;
		case 4:  script_pushint(st,monster->job_exp); break;
		case 5:  script_pushint(st,monster->status.rhw.atk); break;
		case 6:  script_pushint(st,monster->status.rhw.atk2); break;
		case 7:  script_pushint(st,monster->status.def); break;
		case 8:  script_pushint(st,monster->status.mdef); break;
		case 9:  script_pushint(st,monster->status.str); break;
		case 10: script_pushint(st,monster->status.agi); break;
		case 11: script_pushint(st,monster->status.vit); break;
		case 12: script_pushint(st,monster->status.int_); break;
		case 13: script_pushint(st,monster->status.dex); break;
		case 14: script_pushint(st,monster->status.luk); break;
		case 15: script_pushint(st,monster->status.rhw.range); break;
		case 16: script_pushint(st,monster->range2); break;
		case 17: script_pushint(st,monster->range3); break;
		case 18: script_pushint(st,monster->status.size); break;
		case 19: script_pushint(st,monster->status.race); break;
		case 20: script_pushint(st,monster->status.def_ele); break;
		case 21: script_pushint(st,monster->status.mode); break;
		case 22: script_pushint(st,monster->mexp); break;
		case 23: script_pushint(st, monster->dmg_taken_rate); break;
		default: script_pushint(st,-1); //wrong Index
	}
	return true;
}

static BUILDIN(checkvending) // check vending [Nab4]
{
	struct map_session_data *sd = NULL;

	if (script_hasdata(st,2))
		sd = script->nick2sd(st, script_getstr(st,2));
	else
		sd = script->rid2sd(st);

	if (sd != NULL)
		script_pushint(st, sd->state.autotrade ? 2 : sd->state.vending);
	else
		script_pushint(st,0);

	return true;
}

// check chatting [Marka]
static BUILDIN(checkchatting)
{
	struct map_session_data *sd = NULL;

	if (script_hasdata(st,2))
		sd = script->nick2sd(st, script_getstr(st,2));
	else
		sd = script->rid2sd(st);

	if (sd != NULL)
		script_pushint(st, (sd->chat_id != 0));
	else
		script_pushint(st,0);

	return true;
}

static BUILDIN(checkidle)
{
	struct map_session_data *sd = NULL;

	if (script_hasdata(st, 2))
		sd = script->nick2sd(st, script_getstr(st, 2));
	else
		sd = script->rid2sd(st);

	if (sd != NULL)
		script_pushint(st, DIFF_TICK32(sockt->last_tick, sd->idletime)); // TODO: change this to int64 when we'll support 64 bit script values
	else
		script_pushint(st, 0);

	return true;
}

static BUILDIN(searchitem)
{
	struct script_data* data = script_getdata(st, 2);
	const char *itemname = script_getstr(st,3);
	struct item_data *items[MAX_SEARCH];
	int count;

	char* name;
	int32 start;
	int32 id;
	int32 i;
	struct map_session_data *sd = NULL;

	if ((items[0] = itemdb->exists(atoi(itemname)))) {
		count = 1;
	} else {
		count = itemdb->search_name_array(items, ARRAYLENGTH(items), itemname, IT_SEARCH_NAME_PARTIAL);
		if (count > MAX_SEARCH) count = MAX_SEARCH;
	}

	if (!count) {
		script_pushint(st, 0);
		return true;
	}

	if( !data_isreference(data) )
	{
		ShowError("script:searchitem: not a variable\n");
		script->reportdata(data);
		st->state = END;
		return false;// not a variable
	}

	id = reference_getid(data);
	start = reference_getindex(data);
	name = reference_getname(data);

	if( not_server_variable(*name) )
	{
		sd = script->rid2sd(st);
		if( sd == NULL )
			return true;// no player attached
	}

	if( is_string_variable(name) )
	{// string array
		ShowError("script:searchitem: not an integer array reference\n");
		script->reportdata(data);
		st->state = END;
		return false;// not supported
	}

	for( i = 0; i < count; ++start, ++i )
	{// Set array
		const void *v = (const void *)h64BPTRSIZE((int)items[i]->nameid);
		script->set_reg(st, sd, reference_uid(id, start), name, v, reference_getref(data));
	}

	script_pushint(st, count);
	return true;
}

// [zBuffer] List of player cont commands --->
static BUILDIN(rid2name)
{
	struct block_list *bl = NULL;
	int rid = script_getnum(st,2);
	if((bl = map->id2bl(rid))) {
		switch(bl->type) {
			case BL_MOB: script_pushstrcopy(st, BL_UCCAST(BL_MOB, bl)->name); break;
			case BL_PC:  script_pushstrcopy(st, BL_UCCAST(BL_PC, bl)->status.name); break;
			case BL_NPC: script_pushstrcopy(st, BL_UCCAST(BL_NPC, bl)->exname); break;
			case BL_PET: script_pushstrcopy(st, BL_UCCAST(BL_PET, bl)->pet.name); break;
			case BL_HOM: script_pushstrcopy(st, BL_UCCAST(BL_HOM, bl)->homunculus.name); break;
			case BL_MER: script_pushstrcopy(st, BL_UCCAST(BL_MER, bl)->db->name); break;
			default:
				ShowError("buildin_rid2name: BL type unknown.\n");
				script_pushconststr(st,"");
				break;
		}
	} else {
		ShowError("buildin_rid2name: invalid RID\n");
		script_pushconststr(st,"(null)");
	}
	return true;
}

static BUILDIN(pcblockmove)
{
	int id, flag;
	struct map_session_data *sd = NULL;

	id = script_getnum(st,2);
	flag = script_getnum(st,3);

	if (id != 0)
		sd = script->id2sd(st, id);
	else
		sd = script->rid2sd(st);

	if (!sd)
		return true;

	if (flag)
		sd->block_action.move = 1;
	else
		sd->block_action.move = 0;

	return true;
}

static BUILDIN(setpcblock)
{
	struct map_session_data *sd = script_hasdata(st, 4) ? script->id2sd(st, script_getnum(st, 4)) : script->rid2sd(st);
	enum pcblock_action_flag type = script_getnum(st, 2);
	int state = (script_getnum(st, 3) > 0) ? 1 : 0;

	if (sd == NULL) {
		script_pushint(st, 0);
		return true;
	}

	if ((type & PCBLOCK_MOVE) != 0)
		sd->block_action.move = state;

	if ((type & PCBLOCK_ATTACK) != 0)
		sd->block_action.attack = state;

	if ((type & PCBLOCK_SKILL) != 0)
		sd->block_action.skill = state;

	if ((type & PCBLOCK_USEITEM) != 0)
		sd->block_action.useitem = state;

	if ((type & PCBLOCK_CHAT) != 0)
		sd->block_action.chat = state;

	if ((type & PCBLOCK_IMMUNE) != 0)
		sd->block_action.immune = state;

	if ((type & PCBLOCK_SITSTAND) != 0)
		sd->block_action.sitstand = state;

	if ((type & PCBLOCK_COMMANDS) != 0)
		sd->block_action.commands = state;

	if ((type & PCBLOCK_NPC) != 0)
		sd->block_action.npc = state;

	script_pushint(st, 1);
	return true;
}

static BUILDIN(checkpcblock)
{
	struct map_session_data *sd = script_hasdata(st, 2) ? script->id2sd(st, script_getnum(st, 2)) : script->rid2sd(st);
	int retval = PCBLOCK_NONE;

	if (sd == NULL) {
		script_pushint(st, PCBLOCK_NONE);
		return true;
	}

	if (sd->block_action.move != 0)
		retval |= PCBLOCK_MOVE;

	if (sd->block_action.attack != 0)
		retval |= PCBLOCK_ATTACK;

	if (sd->block_action.skill != 0)
		retval |= PCBLOCK_SKILL;

	if (sd->block_action.useitem != 0)
		retval |= PCBLOCK_USEITEM;

	if (sd->block_action.chat != 0)
		retval |= PCBLOCK_CHAT;

	if (sd->block_action.immune != 0)
		retval |= PCBLOCK_IMMUNE;

	if (sd->block_action.sitstand != 0)
		retval |= PCBLOCK_SITSTAND;

	if (sd->block_action.commands != 0)
		retval |= PCBLOCK_COMMANDS;

	if (sd->block_action.npc != 0)
		retval |= PCBLOCK_NPC;

	script_pushint(st, retval);
	return true;
}

static BUILDIN(pcfollow)
{
	int id, targetid;
	struct map_session_data *sd = NULL;

	id = script_getnum(st,2);
	targetid = script_getnum(st,3);

	if (id != 0)
		sd = script->id2sd(st, id);
	else
		sd = script->rid2sd(st);

	if (sd != NULL)
		pc->follow(sd, targetid);

	return true;
}

static BUILDIN(pcstopfollow)
{
	int id;
	struct map_session_data *sd = NULL;

	id = script_getnum(st,2);

	if (id != 0)
		sd = script->id2sd(st, id);
	else
		sd = script->rid2sd(st);

	if (sd != NULL)
		pc->stop_following(sd);

	return true;
}
// <--- [zBuffer] List of player cont commands
// [zBuffer] List of mob control commands --->
//## TODO always return if the request/whatever was successfull [FlavioJS]

static BUILDIN(getunittype)
{
	struct block_list* bl;
	int value;

	bl = map->id2bl(script_getnum(st,2));

	if (!bl) {
		ShowWarning("buildin_getunittype: Error in finding object GID %d!\n", script_getnum(st,2));
		script_pushint(st,-1);
		return false;
	}

	switch (bl->type) {
		case BL_PC:   value = 0; break;
		case BL_NPC:  value = 1; break;
		case BL_PET:  value = 2; break;
		case BL_MOB:  value = 3; break;
		case BL_HOM:  value = 4; break;
		case BL_MER:  value = 5; break;
		case BL_ELEM: value = 6; break;
		default:      value = -1; break;
	}

	script_pushint(st, value);
	return true;
}

/**
 * Sets real-time unit data for a game object.
 *
 * @code{.herc}
 *	setunitdata <GUID>, <DataType>, <Val1>{, <Val2>, <Val3>}
 * @endcode
 *
 * @param1  GUID        GID of the unit.
 * @param2  DataType    Type of Data to be set for the unit.
 * @param3  Value#1     Value to be passed as change in data.
 * @param4  Value#2     Optional int value to be passed for certain data types.
 * @param5  Value#3     Optional int value to be passed for certain data types.
 * @return  1 on success, 0 on failure.
 *
 * Note: Please make this script command only modify ONE INTEGER value.
 *       If need to modify string type data, or having multiple arguments, please introduce a new script command.
 *
 **/
static BUILDIN(setunitdata)
{
	struct block_list *bl = map->id2bl(script_getnum(st, 2));

	if (bl == NULL) {
		ShowWarning("buildin_setunitdata: Error in finding object with given GID %d!\n", script_getnum(st, 2));
		script_pushint(st, 0);
		return false;
	}

	int type = script_getnum(st, 3);

	// Type bounds.
	if (type < UDT_SIZE || type >= UDT_MAX) { // Note: UDT_TYPE is not valid here
		ShowError("buildin_setunitdata: Invalid unit data type %d provided.\n", type);
		script_pushint(st, 0);
		return false;
	}

	const char *mapname = NULL;
	int val = 0;

	// Mandatory argument #3. Subject to deprecate.
	if (type == UDT_MAPIDXY) {
		if (!script_isstringtype(st, 4)) {
			ShowError("buildin_setunitdata: Invalid data type for argument #3.\n");
			script_pushint(st, 0);
			return false;
		}

		mapname = script_getstr(st, 4);
	} else {
		if (script_isstringtype(st, 4)) {
			ShowError("buildin_setunitdata: Invalid data type for argument #3.\n");
			script_pushint(st, 0);
			return false;
		}

		val = script_getnum(st, 4);
	}

/****************************************************************************************************
 * Define temporary macros. [BEGIN]
 ****************************************************************************************************/

// Checks if value is out of bounds.
#define setunitdata_check_bounds(arg, min, max) \
	do { \
		if (script_getnum(st, (arg)) < (min) || script_getnum(st, (arg)) > (max)) { \
			ShowError("buildin_setunitdata: Invalid value %d for argument #%d. (min: %d, max: %d)\n", \
				  script_getnum(st, (arg)), (arg) - 1, (min), (max)); \
			script_pushint(st, 0); \
			return false; \
		} \
	} while(0);

// Checks if value is too low.
#define setunitdata_check_min(arg, min) \
	do { \
		if (script_getnum(st, (arg)) < (min)) { \
			ShowError("buildin_setunitdata: Invalid value %d for argument #%d. (min: %d)\n", \
				  script_getnum(st, (arg)), (arg) - 1, (min)); \
			script_pushint(st, 0); \
			return false; \
		} \
	} while(0);

// Checks if the argument doesn't exist, if required. Also checks if the argument exists, if not required.
#define setunitdata_assert_arg(arg, required) \
	do { \
		if (required && !script_hasdata(st, (arg))) { \
			ShowError("buildin_setunitdata: Type %d reqires argument #%d.\n", type, (arg) - 1); \
			script_pushint(st, 0); \
			return false; \
		} else if (!required && script_hasdata(st, arg)) { \
			ShowError("buildin_setunitdata: Argument %d is not required for type %d.\n", (arg) - 1, type); \
			script_pushint(st, 0); \
			return false; \
		} \
	} while (0);

// Checks if the data is an integer.
#define setunitdata_check_int(arg) \
	do { \
		setunitdata_assert_arg((arg), true); \
		if (script_isstringtype(st, (arg))) { \
			ShowError("buildin_setunitdata: Argument #%d expects integer, string given.\n", (arg) - 1); \
			script_pushint(st, 0); \
			return false; \
		} \
	} while(0);

// Checks if the data is a string.
#define setunitdata_check_string(arg) \
	do { \
		setunitdata_assert_arg((arg), true); \
		if (script_isinttype(st, (arg))) { \
			ShowError("buildin_setunitdata: Argument #%d expects string, integer given.\n", (arg) - 1); \
			script_pushint(st, 0); \
			return false; \
		} \
	} while(0);

/****************************************************************************************************
 * Define temporary macros. [END]
 ****************************************************************************************************/

	if (type != UDT_MAPIDXY && type != UDT_WALKTOXY) {
		setunitdata_assert_arg(5, false);
		setunitdata_assert_arg(6, false);
	}

	int val2 = 0;
	int val3 = 0;

	struct map_session_data *tsd = NULL;

	switch (type) {
	case UDT_SIZE:
		setunitdata_check_bounds(4, SZ_SMALL, SZ_BIG);
		break;
	case UDT_LEVEL:
	case UDT_HP:
	case UDT_MAXHP:
	case UDT_SP:
	case UDT_MAXSP:
	case UDT_CLASS:
	case UDT_HEADBOTTOM:
	case UDT_HEADMIDDLE:
	case UDT_HEADTOP:
	case UDT_CLOTHCOLOR:
	case UDT_SHIELD:
	case UDT_WEAPON:
	case UDT_INTIMACY:
	case UDT_LIFETIME:
	case UDT_MERC_KILLCOUNT:
	case UDT_ROBE:
	case UDT_BODY2:
		setunitdata_check_min(4, 0);
		break;
	case UDT_MASTERAID:
		setunitdata_check_min(4, 0);
		tsd = map->id2sd(val);

		if (tsd == NULL) {
			ShowWarning("buildin_setunitdata: Account ID %d not found for master change!\n", val);
			script_pushint(st, 0);
			return false;
		}

		break;
	case UDT_MASTERCID:
		setunitdata_check_min(4, 0);
		tsd = map->charid2sd(val);

		if (tsd == NULL) {
			ShowWarning("buildin_setunitdata: Character ID %d not found for master change!\n", val);
			script_pushint(st, 0);
			return false;
		}

		break;
	case UDT_MAPIDXY:
		if ((val = map->mapname2mapid(mapname)) == INDEX_NOT_FOUND) {
			ShowError("buildin_setunitdata: Non-existent map %s provided.\n", mapname);
			script_pushint(st, 0);
			return false;
		}

		setunitdata_check_int(5);
		setunitdata_check_int(6);
		setunitdata_check_bounds(5, 0, MAX_MAP_SIZE / 2);
		setunitdata_check_bounds(6, 0, MAX_MAP_SIZE / 2);
		val2 = script_getnum(st, 5);
		val3 = script_getnum(st, 6);
		break;
	case UDT_WALKTOXY:
		setunitdata_assert_arg(6, false);
		setunitdata_check_int(5);
		val2 = script_getnum(st, 5);
		setunitdata_check_bounds(4, 0, MAX_MAP_SIZE / 2);
		setunitdata_check_bounds(5, 0, MAX_MAP_SIZE / 2);
		break;
	case UDT_SPEED:
		setunitdata_check_bounds(4, 0, MAX_WALK_SPEED);
		break;
	case UDT_MODE:
		setunitdata_check_bounds(4, MD_NONE, MD_MASK);
		break;
	case UDT_AI:
		setunitdata_check_bounds(4, AI_NONE, AI_MAX-1);
		break;
	case UDT_SCOPTION:
		setunitdata_check_bounds(4, OPTION_NOTHING, OPTION_COSTUME);
		break;
	case UDT_SEX:
		setunitdata_check_bounds(4, SEX_FEMALE, SEX_MALE);
		break;
	case UDT_HAIRSTYLE:
		setunitdata_check_bounds(4, 0, battle->bc->max_hair_style);
		break;
	case UDT_HAIRCOLOR:
		setunitdata_check_bounds(4, 0, battle->bc->max_hair_color);
		break;
	case UDT_LOOKDIR:
		setunitdata_check_bounds(4, 0, 7);
		break;
	case UDT_CANMOVETICK:
		setunitdata_check_min(4, 0);
		break;
	case UDT_STR:
	case UDT_AGI:
	case UDT_VIT:
	case UDT_INT:
	case UDT_DEX:
	case UDT_LUK:
	case UDT_STATPOINT:
	case UDT_ATKRANGE:
	case UDT_ATKMIN:
	case UDT_ATKMAX:
	case UDT_MATKMIN:
	case UDT_MATKMAX:
	case UDT_AMOTION:
	case UDT_ADELAY:
	case UDT_DMOTION:
		setunitdata_check_bounds(4, 0, USHRT_MAX);
		break;
	case UDT_DEF:
	case UDT_MDEF:
	case UDT_HIT:
	case UDT_FLEE:
	case UDT_PDODGE:
	case UDT_CRIT:
		setunitdata_check_bounds(4, 0, SHRT_MAX);
		break;
	case UDT_HUNGER:
		setunitdata_check_bounds(4, PET_HUNGER_STARVING, PET_HUNGER_STUFFED); // Pets and Homunculi have the same hunger value bounds.
		break;
	case UDT_RACE:
	case UDT_ELETYPE:
	case UDT_ELELEVEL:
		setunitdata_check_bounds(4, 0, CHAR_MAX);
		break;
	case UDT_GROUP:
		setunitdata_check_bounds(4, 0, INT_MAX);

		struct unit_data *ud = unit->bl2ud2(bl);

		if (ud == NULL) {
			ShowError("buildin_setunitdata: ud is NULL!\n");
			script_pushint(st, 0);
			return false;
		}

		ud->groupId = script_getnum(st, 4);
		clif->blname_ack(0, bl); // Send update to client.
		script_pushint(st, 1);
		return true;
	case UDT_DAMAGE_TAKEN_RATE:
		setunitdata_check_bounds(4, 1, INT_MAX);
		break;
	default:
		break;
	}

/****************************************************************************************************
 * Undefine temporary macros. [BEGIN]
 ****************************************************************************************************/

#undef setunitdata_check_bounds
#undef setunitdata_check_min
#undef setunitdata_assert_arg
#undef setunitdata_check_int
#undef setunitdata_check_string

/****************************************************************************************************
 * Undefine temporary macros. [END]
 ****************************************************************************************************/

	// Set the values.
	switch (bl->type) {
	case BL_MOB: {
		struct mob_data *md = BL_UCAST(BL_MOB, bl);

		if (md == NULL) {
			ShowError("buildin_setunitdata: Can't find monster for GID %d!\n", script_getnum(st, 2));
			script_pushint(st, 0);
			return false;
		}

		switch (type) {
		case UDT_SIZE:
			md->status.size = (unsigned char)val;
			break;
		case UDT_LEVEL:
			md->level = val;

			if ((battle_config.show_mob_info & 4) != 0)
				clif->blname_ack(0, &md->bl);

			break;
		case UDT_HP:
			status->set_hp(bl, (unsigned int)val, STATUS_HEAL_DEFAULT);
			clif->blname_ack(0, &md->bl);
			break;
		case UDT_MAXHP:
			md->status.max_hp = (unsigned int)val;
			clif->blname_ack(0, &md->bl);
			break;
		case UDT_SP:
			status->set_sp(bl, (unsigned int)val, STATUS_HEAL_DEFAULT);
			break;
		case UDT_MAXSP:
			md->status.max_sp = (unsigned int)val;
			break;
		case UDT_MASTERAID:
			md->master_id = val;
			break;
		case UDT_MAPIDXY:
			unit->warp(bl, (short)val, (short)val2, (short)val3, CLR_TELEPORT);
			break;
		case UDT_WALKTOXY:
			if (unit->walk_toxy(bl, (short)val, (short)val2, 2) != 0)
				unit->movepos(bl, (short)val, (short)val2, 0, 0);
			break;
		case UDT_SPEED:
			md->status.speed = (unsigned short)val;
			status->calc_misc(bl, &md->status, md->level);
			break;
		case UDT_MODE:
			md->status.mode = (enum e_mode)val;
			break;
		case UDT_AI:
			md->special_state.ai = (enum ai)val;
			break;
		case UDT_SCOPTION:
			md->sc.option = (unsigned int)val;
			break;
		case UDT_SEX:
			md->vd->sex = (char)val;
			break;
		case UDT_CLASS:
			mob->class_change(md, val);
			break;
		case UDT_HAIRSTYLE:
			clif->changelook(bl, LOOK_HAIR, val);
			break;
		case UDT_HAIRCOLOR:
			clif->changelook(bl, LOOK_HAIR_COLOR, val);
			break;
		case UDT_HEADBOTTOM:
			clif->changelook(bl, LOOK_HEAD_BOTTOM, val);
			break;
		case UDT_HEADMIDDLE:
			clif->changelook(bl, LOOK_HEAD_MID, val);
			break;
		case UDT_HEADTOP:
			clif->changelook(bl, LOOK_HEAD_TOP, val);
			break;
		case UDT_CLOTHCOLOR:
			clif->changelook(bl, LOOK_CLOTHES_COLOR, val);
			break;
		case UDT_SHIELD:
			clif->changelook(bl, LOOK_SHIELD, val);
			break;
		case UDT_WEAPON:
			clif->changelook(bl, LOOK_WEAPON, val);
			break;
		case UDT_LOOKDIR:
			unit->set_dir(bl, (enum unit_dir)val);
			break;
		case UDT_CANMOVETICK:
			md->ud.canmove_tick = val;
			break;
		case UDT_STR:
			md->status.str = (unsigned short)val;
			status->calc_misc(bl, &md->status, md->level);
			break;
		case UDT_AGI:
			md->status.agi = (unsigned short)val;
			status->calc_misc(bl, &md->status, md->level);
			break;
		case UDT_VIT:
			md->status.vit = (unsigned short)val;
			status->calc_misc(bl, &md->status, md->level);
			break;
		case UDT_INT:
			md->status.int_ = (unsigned short)val;
			status->calc_misc(bl, &md->status, md->level);
			break;
		case UDT_DEX:
			md->status.dex = (unsigned short)val;
			status->calc_misc(bl, &md->status, md->level);
			break;
		case UDT_LUK:
			md->status.luk = (unsigned short)val;
			status->calc_misc(bl, &md->status, md->level);
			break;
		case UDT_ATKRANGE:
			md->status.rhw.range = (unsigned short)val;
			break;
		case UDT_ATKMIN:
			md->status.rhw.atk = (unsigned short)val;
			break;
		case UDT_ATKMAX:
			md->status.rhw.atk2 = (unsigned short)val;
			break;
		case UDT_MATKMIN:
			md->status.matk_min = (unsigned short)val;
			break;
		case UDT_MATKMAX:
			md->status.matk_max = (unsigned short)val;
			break;
		case UDT_DEF:
			md->status.def = (defType)val;
			break;
		case UDT_MDEF:
			md->status.mdef = (defType)val;
			break;
		case UDT_HIT:
			md->status.hit = (short)val;
			break;
		case UDT_FLEE:
			md->status.flee = (short)val;
			break;
		case UDT_PDODGE:
			md->status.flee2 = (short)val;
			break;
		case UDT_CRIT:
			md->status.cri = (short)val;
			break;
		case UDT_RACE:
			md->status.race = (unsigned char)val;
			break;
		case UDT_ELETYPE:
			md->status.def_ele = (unsigned char)val;
			break;
		case UDT_ELELEVEL:
			md->status.ele_lv = (unsigned char)val;
			break;
		case UDT_AMOTION:
			md->status.amotion = (unsigned short)val;
			break;
		case UDT_ADELAY:
			md->status.adelay = (unsigned short)val;
			break;
		case UDT_DMOTION:
			md->status.dmotion = (unsigned short)val;
			break;
		case UDT_DAMAGE_TAKEN_RATE:
			md->dmg_taken_rate = (int)val;
			break;
		default:
			ShowWarning("buildin_setunitdata: Invalid data type '%d' for mob unit.\n", type);
			script_pushint(st, 0);
			return false;
		}

		break;
	}
	case BL_HOM: {
		struct homun_data *hd = BL_UCAST(BL_HOM, bl);

		if (hd == NULL) {
			ShowError("buildin_setunitdata: Can't find Homunculus for GID %d!\n", script_getnum(st, 2));
			script_pushint(st, 0);
			return false;
		}

		switch (type) {
		case UDT_SIZE:
			hd->base_status.size = (unsigned char)val;
			break;
		case UDT_LEVEL:
			hd->homunculus.level = (short)val;
			break;
		case UDT_HP:
			status->set_hp(bl, (unsigned int)val, STATUS_HEAL_DEFAULT);
			break;
		case UDT_MAXHP:
			hd->homunculus.max_hp = val;
			break;
		case UDT_SP:
			status->set_sp(bl, (unsigned int)val, STATUS_HEAL_DEFAULT);
			break;
		case UDT_MAXSP:
			hd->homunculus.max_sp = val;
			break;
		case UDT_MASTERCID:
			hd->homunculus.char_id = val;
			hd->master = tsd;
			break;
		case UDT_MAPIDXY:
			unit->warp(bl, (short)val, (short)val2, (short)val3, CLR_TELEPORT);
			break;
		case UDT_WALKTOXY:
			if (unit->walk_toxy(bl, (short)val, (short)val2, 2) != 0)
				unit->movepos(bl, (short)val, (short)val2, 0, 0);
			break;
		case UDT_SPEED:
			hd->base_status.speed = (unsigned short)val;
			status->calc_misc(bl, &hd->base_status, hd->homunculus.level);
			break;
		case UDT_LOOKDIR:
			unit->set_dir(bl, (enum unit_dir)val);
			break;
		case UDT_CANMOVETICK:
			hd->ud.canmove_tick = val;
			break;
		case UDT_STR:
			hd->base_status.str = (unsigned short)val;
			status->calc_misc(bl, &hd->base_status, hd->homunculus.level);
			break;
		case UDT_AGI:
			hd->base_status.agi = (unsigned short)val;
			status->calc_misc(bl, &hd->base_status, hd->homunculus.level);
			break;
		case UDT_VIT:
			hd->base_status.vit = (unsigned short)val;
			status->calc_misc(bl, &hd->base_status, hd->homunculus.level);
			break;
		case UDT_INT:
			hd->base_status.int_ = (unsigned short)val;
			status->calc_misc(bl, &hd->base_status, hd->homunculus.level);
			break;
		case UDT_DEX:
			hd->base_status.dex = (unsigned short)val;
			status->calc_misc(bl, &hd->base_status, hd->homunculus.level);
			break;
		case UDT_LUK:
			hd->base_status.luk = (unsigned short)val;
			status->calc_misc(bl, &hd->base_status, hd->homunculus.level);
			break;
		case UDT_ATKRANGE:
			hd->base_status.rhw.range = (unsigned short)val;
			break;
		case UDT_ATKMIN:
			hd->base_status.rhw.atk = (unsigned short)val;
			break;
		case UDT_ATKMAX:
			hd->base_status.rhw.atk2 = (unsigned short)val;
			break;
		case UDT_MATKMIN:
			hd->base_status.matk_min = (unsigned short)val;
			break;
		case UDT_MATKMAX:
			hd->base_status.matk_max = (unsigned short)val;
			break;
		case UDT_DEF:
			hd->base_status.def = (defType)val;
			break;
		case UDT_MDEF:
			hd->base_status.mdef = (defType)val;
			break;
		case UDT_HIT:
			hd->base_status.hit = (short)val;
			break;
		case UDT_FLEE:
			hd->base_status.flee = (short)val;
			break;
		case UDT_PDODGE:
			hd->base_status.flee2 = (short)val;
			break;
		case UDT_CRIT:
			hd->base_status.cri = (short)val;
			break;
		case UDT_RACE:
			hd->base_status.race = (unsigned char)val;
			break;
		case UDT_ELETYPE:
			hd->base_status.def_ele = (unsigned char)val;
			break;
		case UDT_ELELEVEL:
			hd->base_status.ele_lv = (unsigned char)val;
			break;
		case UDT_AMOTION:
			hd->base_status.amotion = (unsigned short)val;
			break;
		case UDT_ADELAY:
			hd->base_status.adelay = (unsigned short)val;
			break;
		case UDT_DMOTION:
			hd->base_status.dmotion = (unsigned short)val;
			break;
		case UDT_HUNGER:
			hd->homunculus.hunger = (short)val;
			clif->send_homdata(hd->master, SP_HUNGRY, hd->homunculus.hunger);
			break;
		case UDT_INTIMACY:
			homun->add_intimacy(hd, (unsigned int)val);
			clif->send_homdata(hd->master, SP_INTIMATE, hd->homunculus.intimacy / 100);
			break;
		default:
			ShowWarning("buildin_setunitdata: Invalid data type '%d' for homunculus unit.\n", type);
			script_pushint(st, 0);
			return false;
		}

		clif->send_homdata(hd->master, SP_ACK, 0); // Send Homunculus data.
		break;
	}
	case BL_PET: {
		struct pet_data *pd = BL_UCAST(BL_PET, bl);

		if (pd == NULL) {
			ShowError("buildin_setunitdata: Can't find pet for GID %d!\n", script_getnum(st, 2));
			script_pushint(st, 0);
			return false;
		}

		switch (type) {
		case UDT_SIZE:
			pd->status.size = (unsigned char)val;
			break;
		case UDT_LEVEL:
			pd->pet.level = (short)val;
			break;
		case UDT_HP:
			status->set_hp(bl, (unsigned int)val, STATUS_HEAL_DEFAULT);
			break;
		case UDT_MAXHP:
			pd->status.max_hp = (unsigned int)val;
			break;
		case UDT_SP:
			status->set_sp(bl, (unsigned int)val, STATUS_HEAL_DEFAULT);
			break;
		case UDT_MAXSP:
			pd->status.max_sp = (unsigned int)val;
			break;
		case UDT_MASTERAID:
			pd->pet.account_id = val;
			pd->msd = tsd;
			break;
		case UDT_MAPIDXY:
			unit->warp(bl, (short)val, (short)val2, (short)val3, CLR_TELEPORT);
			break;
		case UDT_WALKTOXY:
			if (unit->walk_toxy(bl, (short)val, (short)val2, 2) != 0)
				unit->movepos(bl, (short)val, (short)val2, 0, 0);
			break;
		case UDT_SPEED:
			pd->status.speed = (unsigned short)val;
			status->calc_misc(bl, &pd->status, pd->pet.level);
			break;
		case UDT_LOOKDIR:
			unit->set_dir(bl, (enum unit_dir)val);
			break;
		case UDT_CANMOVETICK:
			pd->ud.canmove_tick = val;
			break;
		case UDT_STR:
			pd->status.str = (unsigned short)val;
			status->calc_misc(bl, &pd->status, pd->pet.level);
			break;
		case UDT_AGI:
			pd->status.agi = (unsigned short)val;
			status->calc_misc(bl, &pd->status, pd->pet.level);
			break;
		case UDT_VIT:
			pd->status.vit = (unsigned short)val;
			status->calc_misc(bl, &pd->status, pd->pet.level);
			break;
		case UDT_INT:
			pd->status.int_ = (unsigned short)val;
			status->calc_misc(bl, &pd->status, pd->pet.level);
			break;
		case UDT_DEX:
			pd->status.dex = (unsigned short)val;
			status->calc_misc(bl, &pd->status, pd->pet.level);
			break;
		case UDT_LUK:
			pd->status.luk = (unsigned short)val;
			status->calc_misc(bl, &pd->status, pd->pet.level);
			break;
		case UDT_ATKRANGE:
			pd->status.rhw.range = (unsigned short)val;
			break;
		case UDT_ATKMIN:
			pd->status.rhw.atk = (unsigned short)val;
			break;
		case UDT_ATKMAX:
			pd->status.rhw.atk2 = (unsigned short)val;
			break;
		case UDT_MATKMIN:
			pd->status.matk_min = (unsigned short)val;
			break;
		case UDT_MATKMAX:
			pd->status.matk_max = (unsigned short)val;
			break;
		case UDT_DEF:
			pd->status.def = (defType)val;
			break;
		case UDT_MDEF:
			pd->status.mdef = (defType)val;
			break;
		case UDT_HIT:
			pd->status.hit = (short)val;
			break;
		case UDT_FLEE:
			pd->status.flee = (short)val;
			break;
		case UDT_PDODGE:
			pd->status.flee2 = (short)val;
			break;
		case UDT_CRIT:
			pd->status.cri = (short)val;
			break;
		case UDT_RACE:
			pd->status.race = (unsigned char)val;
			break;
		case UDT_ELETYPE:
			pd->status.def_ele = (unsigned char)val;
			break;
		case UDT_ELELEVEL:
			pd->status.ele_lv = (unsigned char)val;
			break;
		case UDT_AMOTION:
			pd->status.amotion = (unsigned short)val;
			break;
		case UDT_ADELAY:
			pd->status.adelay = (unsigned short)val;
			break;
		case UDT_DMOTION:
			pd->status.dmotion = (unsigned short)val;
			break;
		case UDT_INTIMACY:
			pet->set_intimate(pd, val);
			clif->send_petdata(pd->msd, pd, 1, pd->pet.intimate);
			break;
		case UDT_HUNGER:
			pet->set_hunger(pd, val);
			break;
		default:
			ShowWarning("buildin_setunitdata: Invalid data type '%d' for pet unit.\n", type);
			script_pushint(st, 0);
			return false;
		}

		clif->send_petstatus(pd->msd); // Send pet data.
		break;
	}
	case BL_MER: {
		struct mercenary_data *mc = BL_UCAST(BL_MER, bl);

		if (mc == NULL) {
			ShowError("buildin_setunitdata: Can't find mercenary for GID %d!\n", script_getnum(st, 2));
			script_pushint(st, 0);
			return false;
		}

		switch (type) {
		case UDT_SIZE:
			mc->base_status.size = (unsigned char)val;
			break;
		case UDT_HP:
			status->set_hp(bl, (unsigned int)val, STATUS_HEAL_DEFAULT);
			break;
		case UDT_MAXHP:
			mc->base_status.max_hp = (unsigned int)val;
			break;
		case UDT_SP:
			status->set_sp(bl, (unsigned int)val, STATUS_HEAL_DEFAULT);
			break;
		case UDT_MAXSP:
			mc->base_status.max_sp = (unsigned int)val;
			break;
		case UDT_MASTERCID:
			mc->mercenary.char_id = val;
			break;
		case UDT_MAPIDXY:
			unit->warp(bl, (short)val, (short)val2, (short)val3, CLR_TELEPORT);
			break;
		case UDT_WALKTOXY:
			if (unit->walk_toxy(bl, (short)val, (short)val2, 2) != 0)
				unit->movepos(bl, (short)val, (short)val2, 0, 0);
			break;
		case UDT_SPEED:
			mc->base_status.size = (unsigned char)val;
			status->calc_misc(bl, &mc->base_status, mc->db->lv);
			break;
		case UDT_LOOKDIR:
			unit->set_dir(bl, (enum unit_dir)val);
			break;
		case UDT_CANMOVETICK:
			mc->ud.canmove_tick = val;
			break;
		case UDT_STR:
			mc->base_status.str = (unsigned short)val;
			status->calc_misc(bl, &mc->base_status, mc->db->lv);
			break;
		case UDT_AGI:
			mc->base_status.agi = (unsigned short)val;
			status->calc_misc(bl, &mc->base_status, mc->db->lv);
			break;
		case UDT_VIT:
			mc->base_status.vit = (unsigned short)val;
			status->calc_misc(bl, &mc->base_status, mc->db->lv);
			break;
		case UDT_INT:
			mc->base_status.int_ = (unsigned short)val;
			status->calc_misc(bl, &mc->base_status, mc->db->lv);
			break;
		case UDT_DEX:
			mc->base_status.dex = (unsigned short)val;
			status->calc_misc(bl, &mc->base_status, mc->db->lv);
			break;
		case UDT_LUK:
			mc->base_status.luk = (unsigned short)val;
			status->calc_misc(bl, &mc->base_status, mc->db->lv);
			break;
		case UDT_ATKRANGE:
			mc->base_status.rhw.range = (unsigned short)val;
			break;
		case UDT_ATKMIN:
			mc->base_status.rhw.atk = (unsigned short)val;
			break;
		case UDT_ATKMAX:
			mc->base_status.rhw.atk2 = (unsigned short)val;
			break;
		case UDT_MATKMIN:
			mc->base_status.matk_min = (unsigned short)val;
			break;
		case UDT_MATKMAX:
			mc->base_status.matk_max = (unsigned short)val;
			break;
		case UDT_DEF:
			mc->base_status.def = (defType)val;
			break;
		case UDT_MDEF:
			mc->base_status.mdef = (defType)val;
			break;
		case UDT_HIT:
			mc->base_status.hit = (short)val;
			break;
		case UDT_FLEE:
			mc->base_status.flee = (short)val;
			break;
		case UDT_PDODGE:
			mc->base_status.flee2 = (short)val;
			break;
		case UDT_CRIT:
			mc->base_status.cri = (short)val;
			break;
		case UDT_RACE:
			mc->base_status.race = (unsigned char)val;
			break;
		case UDT_ELETYPE:
			mc->base_status.def_ele = (unsigned char)val;
			break;
		case UDT_ELELEVEL:
			mc->base_status.ele_lv = (unsigned char)val;
			break;
		case UDT_AMOTION:
			mc->base_status.amotion = (unsigned short)val;
			break;
		case UDT_ADELAY:
			mc->base_status.adelay = (unsigned short)val;
			break;
		case UDT_DMOTION:
			mc->base_status.dmotion = (unsigned short)val;
			break;
		case UDT_MERC_KILLCOUNT:
			mc->mercenary.kill_count = (unsigned int)val;
			break;
		case UDT_LIFETIME:
			mc->mercenary.life_time = (unsigned int)val;
			break;
		default:
			ShowWarning("buildin_setunitdata: Invalid data type '%d' for mercenary unit.\n", type);
			script_pushint(st, 0);
			return false;
		}

		// Send mercenary data.
		clif->mercenary_info(map->charid2sd(mc->mercenary.char_id));
		clif->mercenary_skillblock(map->charid2sd(mc->mercenary.char_id));
		break;
	}
	case BL_ELEM: {
		struct elemental_data *ed = BL_UCAST(BL_ELEM, bl);

		if (ed == NULL) {
			ShowError("buildin_setunitdata: Can't find Elemental for GID %d!\n", script_getnum(st, 2));
			script_pushint(st, 0);
			return false;
		}

		switch (type) {
		case UDT_SIZE:
			ed->base_status.size = (unsigned char)val;
			break;
		case UDT_HP:
			status->set_hp(bl, (unsigned int)val, STATUS_HEAL_DEFAULT);
			break;
		case UDT_MAXHP:
			ed->base_status.max_hp = (unsigned int)val;
			break;
		case UDT_SP:
			status->set_sp(bl, (unsigned int)val, STATUS_HEAL_DEFAULT);
			break;
		case UDT_MAXSP:
			ed->base_status.max_sp = (unsigned int)val;
			break;
		case UDT_MASTERCID:
			ed->elemental.char_id = val;
			break;
		case UDT_MAPIDXY:
			unit->warp(bl, (short)val, (short)val2, (short)val3, CLR_TELEPORT);
			break;
		case UDT_WALKTOXY:
			if (unit->walk_toxy(bl, (short)val, (short)val2, 2) != 0)
				unit->movepos(bl, (short)val, (short)val2, 0, 0);
			break;
		case UDT_SPEED:
			ed->base_status.speed = (unsigned short)val;
			status->calc_misc(bl, &ed->base_status, ed->db->lv);
			break;
		case UDT_LOOKDIR:
			unit->set_dir(bl, (enum unit_dir)val);
			break;
		case UDT_CANMOVETICK:
			ed->ud.canmove_tick = val;
			break;
		case UDT_STR:
			ed->base_status.str = (unsigned short)val;
			status->calc_misc(bl, &ed->base_status, ed->db->lv);
			break;
		case UDT_AGI:
			ed->base_status.agi = (unsigned short)val;
			status->calc_misc(bl, &ed->base_status, ed->db->lv);
			break;
		case UDT_VIT:
			ed->base_status.vit = (unsigned short)val;
			status->calc_misc(bl, &ed->base_status, ed->db->lv);
			break;
		case UDT_INT:
			ed->base_status.int_ = (unsigned short)val;
			status->calc_misc(bl, &ed->base_status, ed->db->lv);
			break;
		case UDT_DEX:
			ed->base_status.dex = (unsigned short)val;
			status->calc_misc(bl, &ed->base_status, ed->db->lv);
			break;
		case UDT_LUK:
			ed->base_status.luk = (unsigned short)val;
			status->calc_misc(bl, &ed->base_status, ed->db->lv);
			break;
		case UDT_ATKRANGE:
			ed->base_status.rhw.range = (unsigned short)val;
			break;
		case UDT_ATKMIN:
			ed->base_status.rhw.atk = (unsigned short)val;
			break;
		case UDT_ATKMAX:
			ed->base_status.rhw.atk2 = (unsigned short)val;
			break;
		case UDT_MATKMIN:
			ed->base_status.matk_min = (unsigned short)val;
			break;
		case UDT_MATKMAX:
			ed->base_status.matk_max = (unsigned short)val;
			break;
		case UDT_DEF:
			ed->base_status.def = (defType)val;
			break;
		case UDT_MDEF:
			ed->base_status.mdef = (defType)val;
			break;
		case UDT_HIT:
			ed->base_status.hit = (short)val;
			break;
		case UDT_FLEE:
			ed->base_status.flee = (short)val;
			break;
		case UDT_PDODGE:
			ed->base_status.flee2 = (short)val;
			break;
		case UDT_CRIT:
			ed->base_status.cri = (short)val;
			break;
		case UDT_RACE:
			ed->base_status.race = (unsigned char)val;
			break;
		case UDT_ELETYPE:
			ed->base_status.def_ele = (unsigned char)val;
			break;
		case UDT_ELELEVEL:
			ed->base_status.ele_lv = (unsigned char)val;
			break;
		case UDT_AMOTION:
			ed->base_status.amotion = (unsigned short)val;
			break;
		case UDT_ADELAY:
			ed->base_status.adelay = (unsigned short)val;
			break;
		case UDT_DMOTION:
			ed->base_status.dmotion = (unsigned short)val;
			break;
		case UDT_LIFETIME:
			ed->elemental.life_time = val;
			break;
		default:
			ShowWarning("buildin_setunitdata: Invalid data type '%d' for elemental unit.\n", type);
			script_pushint(st, 0);
			return false;
		}

		clif->elemental_info(ed->master); // Send Elemental data.
		break;
	}
	case BL_NPC: {
		struct npc_data *nd = BL_UCAST(BL_NPC, bl);

		if (nd == NULL) {
			ShowError("buildin_setunitdata: Can't find NPC for GID %d!\n", script_getnum(st, 2));
			script_pushint(st, 0);
			return false;
		}

		switch (type) {
		case UDT_SIZE:
			nd->status.size = (unsigned char)val;
			break;
		case UDT_LEVEL:
			nd->level = (unsigned short)val;
			break;
		case UDT_HP:
			status->set_hp(bl, (unsigned int)val, STATUS_HEAL_DEFAULT);
			break;
		case UDT_MAXHP:
			nd->status.max_hp = (unsigned int)val;
			break;
		case UDT_SP:
			status->set_sp(bl, (unsigned int)val, STATUS_HEAL_DEFAULT);
			break;
		case UDT_MAXSP:
			nd->status.max_sp = (unsigned int)val;
			break;
		case UDT_MAPIDXY:
			unit->warp(bl, (short)val, (short)val2, (short)val3, CLR_TELEPORT);
			break;
		case UDT_WALKTOXY:
			if (unit->walk_toxy(bl, (short)val, (short)val2, 2) != 0)
				unit->movepos(bl, (short)val, (short)val2, 0, 0);
			break;
		case UDT_CLASS:
			npc->setclass(nd, (short)val);
			break;
		case UDT_SPEED:
			nd->speed = (short)val;
			status->calc_misc(bl, &nd->status, nd->level);
			break;
		case UDT_LOOKDIR:
			unit->set_dir(bl, (enum unit_dir)val);
			break;
		case UDT_STR:
			nd->status.str = (unsigned short)val;
			status->calc_misc(bl, &nd->status, nd->level);
			break;
		case UDT_AGI:
			nd->status.agi = (unsigned short)val;
			status->calc_misc(bl, &nd->status, nd->level);
			break;
		case UDT_VIT:
			nd->status.vit = (unsigned short)val;
			status->calc_misc(bl, &nd->status, nd->level);
			break;
		case UDT_INT:
			nd->status.int_ = (unsigned short)val;
			status->calc_misc(bl, &nd->status, nd->level);
			break;
		case UDT_DEX:
			nd->status.dex = (unsigned short)val;
			status->calc_misc(bl, &nd->status, nd->level);
			break;
		case UDT_LUK:
			nd->status.luk = (unsigned short)val;
			status->calc_misc(bl, &nd->status, nd->level);
			break;
		case UDT_STATPOINT:
			nd->stat_point = (unsigned short)val;
			break;
		case UDT_ATKRANGE:
			nd->status.rhw.range = (unsigned short)val;
			break;
		case UDT_ATKMIN:
			nd->status.rhw.atk = (unsigned short)val;
			break;
		case UDT_ATKMAX:
			nd->status.rhw.atk2 = (unsigned short)val;
			break;
		case UDT_MATKMIN:
			nd->status.matk_min = (unsigned short)val;
			break;
		case UDT_MATKMAX:
			nd->status.matk_max = (unsigned short)val;
			break;
		case UDT_DEF:
			nd->status.def = (defType)val;
			break;
		case UDT_MDEF:
			nd->status.mdef = (defType)val;
			break;
		case UDT_HIT:
			nd->status.hit = (short)val;
			break;
		case UDT_FLEE:
			nd->status.flee = (short)val;
			break;
		case UDT_PDODGE:
			nd->status.flee2 = (short)val;
			break;
		case UDT_CRIT:
			nd->status.cri = (short)val;
			break;
		case UDT_RACE:
			nd->status.race = (unsigned char)val;
			break;
		case UDT_ELETYPE:
			nd->status.def_ele = (unsigned char)val;
			break;
		case UDT_ELELEVEL:
			nd->status.ele_lv = (unsigned char)val;
			break;
		case UDT_AMOTION:
			nd->status.amotion = (unsigned short)val;
			break;
		case UDT_ADELAY:
			nd->status.adelay = (unsigned short)val;
			break;
		case UDT_DMOTION:
			nd->status.dmotion = (unsigned short)val;
			break;
		case UDT_SEX:
			nd->vd.sex = (char)val;
			npc->refresh(nd);
			break;
		case UDT_HAIRSTYLE:
			clif->changelook(bl, LOOK_HAIR, val);
			break;
		case UDT_HAIRCOLOR:
			clif->changelook(bl, LOOK_HAIR_COLOR, val);
			break;
		case UDT_HEADBOTTOM:
			clif->changelook(bl, LOOK_HEAD_BOTTOM, val);
			break;
		case UDT_HEADMIDDLE:
			clif->changelook(bl, LOOK_HEAD_MID, val);
			break;
		case UDT_HEADTOP:
			clif->changelook(bl, LOOK_HEAD_TOP, val);
			break;
		case UDT_CLOTHCOLOR:
			clif->changelook(bl, LOOK_CLOTHES_COLOR, val);
			break;
		case UDT_SHIELD:
			clif->changelook(bl, LOOK_SHIELD, val);
			break;
		case UDT_WEAPON:
			clif->changelook(bl, LOOK_WEAPON, val);
			break;
		case UDT_ROBE:
			clif->changelook(bl, LOOK_ROBE, val);
			break;
		case UDT_BODY2:
			clif->changelook(bl, LOOK_BODY2, val);
			break;
		default:
			ShowWarning("buildin_setunitdata: Invalid data type '%d' for NPC unit.\n", type);
			script_pushint(st, 0);
			return false;
		}

		break;
	}
	default:
		ShowError("buildin_setunitdata: Unknown object!\n");
		script_pushint(st, 0);
		return false;
	} // End of bl->type switch.

	script_pushint(st, 1);

	return true;
}

/**
 * Retrieves real-time data for a game object.
 * Getunitdata <GUID>,<DataType>{,<Variable>}
 * @param1  GUID        Game object unique Id.
 * @param2  DataType    Type of Data to be set for the unit.
 * @param3  Variable    array reference to store data into. (used for UDT_MAPIDXY)
 * @return  0 on failure, <value> on success

 Note: Please make this script command only return ONE INTEGER value.
 If the unit data having multiple arguments, or need to return in array,
 please introduce a new script command.
 */
static BUILDIN(getunitdata)
{
	struct block_list *bl;
	const char *udtype = NULL;
	const struct map_session_data *sd = NULL;
	int type = 0;
	char* name = NULL;
	struct script_data *data = script_hasdata(st,4)?script_getdata(st, 4):NULL;

	bl = map->id2bl(script_getnum(st, 2));

	if (bl == NULL) {
		ShowWarning("buildin_getunitdata: Error in finding object with given GID %d!\n", script_getnum(st, 2));
		script_pushint(st, -1);
		return false;
	}

	type = script_getnum(st, 3);

	/* Type check */
	if (type < UDT_TYPE || type >= UDT_MAX) {
		ShowError("buildin_getunitdata: Invalid unit data type %d provided.\n", type);
		script_pushint(st, -1);
		return false;
	}

	/* Argument checks. Subject to deprecate */
	if (type == UDT_MAPIDXY) {
		if (data == NULL || !data_isreference(data)) {
			ShowWarning("buildin_getunitdata: Error in argument 3. Please provide a reference variable to store values in.\n");
			script_pushint(st, -1);
			return false;
		}

		name = reference_getname(data);

		if (not_server_variable(*name)) {
			sd = script->rid2sd(st);
			if (sd == NULL) {
				ShowWarning("buildin_getunitdata: Player not attached! Cannot use player variable %s.\n",name);
				script_pushint(st, -1);
				return true;// no player attached
			}
		}
	} else if (type == UDT_GROUP) {
		struct unit_data *ud = unit->bl2ud(bl);
		if (ud == NULL) {
			ShowError("buildin_setunitdata: ud is NULL!\n");
			script_pushint(st, -1);
			return false;
		}
		script_pushint(st, ud->groupId);
		return true;
	}

#define getunitdata_sub(idx__,var__) script->setd_sub(st,NULL,name,(idx__),(void *)h64BPTRSIZE((int)(var__)),data->ref);

	switch (bl->type) {
	case BL_MOB:
	{
		const struct mob_data *md = BL_UCAST(BL_MOB, bl);

		nullpo_retr(false, md);

		switch (type)
		{
		case UDT_TYPE:        script_pushint(st, BL_MOB); break;
		case UDT_SIZE:        script_pushint(st, md->status.size); break;
		case UDT_LEVEL:       script_pushint(st, md->level); break;
		case UDT_HP:          script_pushint(st, md->status.hp); break;
		case UDT_MAXHP:       script_pushint(st, md->status.max_hp); break;
		case UDT_SP:          script_pushint(st, md->status.sp); break;
		case UDT_MAXSP:       script_pushint(st, md->status.max_sp); break;
		case UDT_MAPIDXY:
			getunitdata_sub(0, md->bl.m);
			getunitdata_sub(1, md->bl.x);
			getunitdata_sub(2, md->bl.y);
			break;
		case UDT_SPEED:       script_pushint(st, md->status.speed); break;
		case UDT_MODE:        script_pushint(st, md->status.mode); break;
		case UDT_AI:          script_pushint(st, md->special_state.ai); break;
		case UDT_SCOPTION:    script_pushint(st, md->sc.option); break;
		case UDT_SEX:         script_pushint(st, md->vd->sex); break;
		case UDT_CLASS:       script_pushint(st, md->vd->class); break;
		case UDT_HAIRSTYLE:   script_pushint(st, md->vd->hair_style); break;
		case UDT_HAIRCOLOR:   script_pushint(st, md->vd->hair_color); break;
		case UDT_HEADBOTTOM:  script_pushint(st, md->vd->head_bottom); break;
		case UDT_HEADMIDDLE:  script_pushint(st, md->vd->head_mid); break;
		case UDT_HEADTOP:     script_pushint(st, md->vd->head_top); break;
		case UDT_CLOTHCOLOR:  script_pushint(st, md->vd->cloth_color); break;
		case UDT_SHIELD:      script_pushint(st, md->vd->shield); break;
		case UDT_WEAPON:      script_pushint(st, md->vd->weapon); break;
		case UDT_LOOKDIR:     script_pushint(st, md->ud.dir); break;
		case UDT_CANMOVETICK: script_pushint(st, md->ud.canmove_tick); break;
		case UDT_STR:         script_pushint(st, md->status.str); break;
		case UDT_AGI:         script_pushint(st, md->status.agi); break;
		case UDT_VIT:         script_pushint(st, md->status.vit); break;
		case UDT_INT:         script_pushint(st, md->status.int_); break;
		case UDT_DEX:         script_pushint(st, md->status.dex); break;
		case UDT_LUK:         script_pushint(st, md->status.luk); break;
		case UDT_ATKRANGE:    script_pushint(st, md->status.rhw.range); break;
		case UDT_ATKMIN:      script_pushint(st, md->status.rhw.atk); break;
		case UDT_ATKMAX:      script_pushint(st, md->status.rhw.atk2); break;
		case UDT_MATKMIN:     script_pushint(st, md->status.matk_min); break;
		case UDT_MATKMAX:     script_pushint(st, md->status.matk_max); break;
		case UDT_DEF:         script_pushint(st, md->status.def); break;
		case UDT_MDEF:        script_pushint(st, md->status.mdef); break;
		case UDT_HIT:         script_pushint(st, md->status.hit); break;
		case UDT_FLEE:        script_pushint(st, md->status.flee); break;
		case UDT_PDODGE:      script_pushint(st, md->status.flee2); break;
		case UDT_CRIT:        script_pushint(st, md->status.cri); break;
		case UDT_RACE:        script_pushint(st, md->status.race); break;
		case UDT_ELETYPE:     script_pushint(st, md->status.def_ele); break;
		case UDT_ELELEVEL:    script_pushint(st, md->status.ele_lv); break;
		case UDT_AMOTION:     script_pushint(st, md->status.amotion); break;
		case UDT_ADELAY:      script_pushint(st, md->status.adelay); break;
		case UDT_DMOTION:     script_pushint(st, md->status.dmotion); break;
		case UDT_DAMAGE_TAKEN_RATE: script_pushint(st, md->dmg_taken_rate); break;
		default:
			ShowWarning("buildin_getunitdata: Invalid data type '%s' for Mob unit.\n", udtype);
			script_pushint(st, -1);
			return false;
		}
	}
		break;
	case BL_HOM:
	{
		const struct homun_data *hd = BL_UCAST(BL_HOM, bl);

		nullpo_retr(false, hd);

		switch (type)
		{
		case UDT_TYPE:        script_pushint(st, BL_HOM); break;
		case UDT_SIZE:        script_pushint(st, hd->base_status.size); break;
		case UDT_LEVEL:       script_pushint(st, hd->homunculus.level); break;
		case UDT_HP:          script_pushint(st, hd->base_status.hp); break;
		case UDT_MAXHP:       script_pushint(st, hd->base_status.max_hp); break;
		case UDT_SP:          script_pushint(st, hd->base_status.sp); break;
		case UDT_MAXSP:       script_pushint(st, hd->base_status.max_sp); break;
		case UDT_MAPIDXY:
			getunitdata_sub(0, hd->bl.m);
			getunitdata_sub(1, hd->bl.x);
			getunitdata_sub(2, hd->bl.y);
			break;
		case UDT_SPEED:       script_pushint(st, hd->base_status.speed); break;
		case UDT_LOOKDIR:     script_pushint(st, hd->ud.dir); break;
		case UDT_CANMOVETICK: script_pushint(st, hd->ud.canmove_tick); break;
		case UDT_MODE:        script_pushint(st, hd->base_status.mode); break;
		case UDT_STR:         script_pushint(st, hd->base_status.str); break;
		case UDT_AGI:         script_pushint(st, hd->base_status.agi); break;
		case UDT_VIT:         script_pushint(st, hd->base_status.vit); break;
		case UDT_INT:         script_pushint(st, hd->base_status.int_); break;
		case UDT_DEX:         script_pushint(st, hd->base_status.dex); break;
		case UDT_LUK:         script_pushint(st, hd->base_status.luk); break;
		case UDT_ATKRANGE:    script_pushint(st, hd->base_status.rhw.range); break;
		case UDT_ATKMIN:      script_pushint(st, hd->base_status.rhw.atk); break;
		case UDT_ATKMAX:      script_pushint(st, hd->base_status.rhw.atk2); break;
		case UDT_MATKMIN:     script_pushint(st, hd->base_status.matk_min); break;
		case UDT_MATKMAX:     script_pushint(st, hd->base_status.matk_max); break;
		case UDT_DEF:         script_pushint(st, hd->base_status.def); break;
		case UDT_MDEF:        script_pushint(st, hd->base_status.mdef); break;
		case UDT_HIT:         script_pushint(st, hd->base_status.hit); break;
		case UDT_FLEE:        script_pushint(st, hd->base_status.flee); break;
		case UDT_PDODGE:      script_pushint(st, hd->base_status.flee2); break;
		case UDT_CRIT:        script_pushint(st, hd->base_status.cri); break;
		case UDT_RACE:        script_pushint(st, hd->base_status.race); break;
		case UDT_ELETYPE:     script_pushint(st, hd->base_status.def_ele); break;
		case UDT_ELELEVEL:    script_pushint(st, hd->base_status.ele_lv); break;
		case UDT_AMOTION:     script_pushint(st, hd->base_status.amotion); break;
		case UDT_ADELAY:      script_pushint(st, hd->base_status.adelay); break;
		case UDT_DMOTION:     script_pushint(st, hd->base_status.dmotion); break;
		case UDT_MASTERCID:   script_pushint(st, hd->homunculus.char_id); break;
		case UDT_HUNGER:      script_pushint(st, hd->homunculus.hunger); break;
		case UDT_INTIMACY:    script_pushint(st, hd->homunculus.intimacy); break;
		default:
			ShowWarning("buildin_getunitdata: Invalid data type '%s' for Homunculus unit.\n", udtype);
			script_pushint(st, -1);
			return false;
		}
	}
		break;
	case BL_PET:
	{
		const struct pet_data *pd = BL_UCAST(BL_PET, bl);

		nullpo_retr(false, pd);

		switch (type)
		{
		case UDT_TYPE:        script_pushint(st, BL_PET); break;
		case UDT_SIZE:        script_pushint(st, pd->status.size); break;
		case UDT_LEVEL:       script_pushint(st, pd->pet.level); break;
		case UDT_HP:          script_pushint(st, pd->status.hp); break;
		case UDT_MAXHP:       script_pushint(st, pd->status.max_hp); break;
		case UDT_SP:          script_pushint(st, pd->status.sp); break;
		case UDT_MAXSP:       script_pushint(st, pd->status.max_sp); break;
		case UDT_MAPIDXY:
			getunitdata_sub(0, pd->bl.m);
			getunitdata_sub(1, pd->bl.x);
			getunitdata_sub(2, pd->bl.y);
			break;
		case UDT_SPEED:       script_pushint(st, pd->status.speed); break;
		case UDT_LOOKDIR:     script_pushint(st, pd->ud.dir); break;
		case UDT_CANMOVETICK: script_pushint(st, pd->ud.canmove_tick); break;
		case UDT_MODE:        script_pushint(st, pd->status.mode); break;
		case UDT_STR:         script_pushint(st, pd->status.str); break;
		case UDT_AGI:         script_pushint(st, pd->status.agi); break;
		case UDT_VIT:         script_pushint(st, pd->status.vit); break;
		case UDT_INT:         script_pushint(st, pd->status.int_); break;
		case UDT_DEX:         script_pushint(st, pd->status.dex); break;
		case UDT_LUK:         script_pushint(st, pd->status.luk); break;
		case UDT_ATKRANGE:    script_pushint(st, pd->status.rhw.range); break;
		case UDT_ATKMIN:      script_pushint(st, pd->status.rhw.atk); break;
		case UDT_ATKMAX:      script_pushint(st, pd->status.rhw.atk2); break;
		case UDT_MATKMIN:     script_pushint(st, pd->status.matk_min); break;
		case UDT_MATKMAX:     script_pushint(st, pd->status.matk_max); break;
		case UDT_DEF:         script_pushint(st, pd->status.def); break;
		case UDT_MDEF:        script_pushint(st, pd->status.mdef); break;
		case UDT_HIT:         script_pushint(st, pd->status.hit); break;
		case UDT_FLEE:        script_pushint(st, pd->status.flee); break;
		case UDT_PDODGE:      script_pushint(st, pd->status.flee2); break;
		case UDT_CRIT:        script_pushint(st, pd->status.cri); break;
		case UDT_RACE:        script_pushint(st, pd->status.race); break;
		case UDT_ELETYPE:     script_pushint(st, pd->status.def_ele); break;
		case UDT_ELELEVEL:    script_pushint(st, pd->status.ele_lv); break;
		case UDT_AMOTION:     script_pushint(st, pd->status.amotion); break;
		case UDT_ADELAY:      script_pushint(st, pd->status.adelay); break;
		case UDT_DMOTION:     script_pushint(st, pd->status.dmotion); break;
		case UDT_MASTERAID:   script_pushint(st, pd->pet.account_id); break;
		case UDT_HUNGER:      script_pushint(st, pd->pet.hungry); break;
		case UDT_INTIMACY:    script_pushint(st, pd->pet.intimate); break;
		default:
			ShowWarning("buildin_getunitdata: Invalid data type '%s' for Pet unit.\n", udtype);
			script_pushint(st, -1);
			return false;
		}
	}
		break;
	case BL_MER:
	{
		const struct mercenary_data *mc = BL_UCAST(BL_MER, bl);

		nullpo_retr(false, mc);

		switch (type)
		{
		case UDT_TYPE:        script_pushint(st, BL_MER); break;
		case UDT_SIZE:        script_pushint(st, mc->base_status.size); break;
		case UDT_HP:          script_pushint(st, mc->base_status.hp); break;
		case UDT_MAXHP:       script_pushint(st, mc->base_status.max_hp); break;
		case UDT_SP:          script_pushint(st, mc->base_status.sp); break;
		case UDT_MAXSP:       script_pushint(st, mc->base_status.max_sp); break;
		case UDT_MAPIDXY:
			getunitdata_sub(0, mc->bl.m);
			getunitdata_sub(1, mc->bl.x);
			getunitdata_sub(2, mc->bl.y);
			break;
		case UDT_SPEED:       script_pushint(st, mc->base_status.speed); break;
		case UDT_LOOKDIR:     script_pushint(st, mc->ud.dir); break;
		case UDT_CANMOVETICK: script_pushint(st, mc->ud.canmove_tick); break;
		case UDT_MODE:        script_pushint(st, mc->base_status.mode); break;
		case UDT_STR:         script_pushint(st, mc->base_status.str); break;
		case UDT_AGI:         script_pushint(st, mc->base_status.agi); break;
		case UDT_VIT:         script_pushint(st, mc->base_status.vit); break;
		case UDT_INT:         script_pushint(st, mc->base_status.int_); break;
		case UDT_DEX:         script_pushint(st, mc->base_status.dex); break;
		case UDT_LUK:         script_pushint(st, mc->base_status.luk); break;
		case UDT_ATKRANGE:    script_pushint(st, mc->base_status.rhw.range); break;
		case UDT_ATKMIN:      script_pushint(st, mc->base_status.rhw.atk); break;
		case UDT_ATKMAX:      script_pushint(st, mc->base_status.rhw.atk2); break;
		case UDT_MATKMIN:     script_pushint(st, mc->base_status.matk_min); break;
		case UDT_MATKMAX:     script_pushint(st, mc->base_status.matk_max); break;
		case UDT_DEF:         script_pushint(st, mc->base_status.def); break;
		case UDT_MDEF:        script_pushint(st, mc->base_status.mdef); break;
		case UDT_HIT:         script_pushint(st, mc->base_status.hit); break;
		case UDT_FLEE:        script_pushint(st, mc->base_status.flee); break;
		case UDT_PDODGE:      script_pushint(st, mc->base_status.flee2); break;
		case UDT_CRIT:        script_pushint(st, mc->base_status.cri); break;
		case UDT_RACE:        script_pushint(st, mc->base_status.race); break;
		case UDT_ELETYPE:     script_pushint(st, mc->base_status.def_ele); break;
		case UDT_ELELEVEL:    script_pushint(st, mc->base_status.ele_lv); break;
		case UDT_AMOTION:     script_pushint(st, mc->base_status.amotion); break;
		case UDT_ADELAY:      script_pushint(st, mc->base_status.adelay); break;
		case UDT_DMOTION:     script_pushint(st, mc->base_status.dmotion); break;
		case UDT_MASTERCID:   script_pushint(st, mc->mercenary.char_id); break;
		case UDT_MERC_KILLCOUNT: script_pushint(st, mc->mercenary.kill_count); break;
		case UDT_LIFETIME:    script_pushint(st, mc->mercenary.life_time); break;
		default:
			ShowWarning("buildin_getunitdata: Invalid data type '%s' for Mercenary unit.\n", udtype);
			script_pushint(st, -1);
			return false;
		}
	}
		break;
	case BL_ELEM:
	{
		const struct elemental_data *ed = BL_UCAST(BL_ELEM, bl);

		nullpo_retr(false, ed);

		switch (type)
		{
		case UDT_TYPE:        script_pushint(st, BL_ELEM); break;
		case UDT_SIZE:        script_pushint(st, ed->base_status.size); break;
		case UDT_HP:          script_pushint(st, ed->base_status.hp); break;
		case UDT_MAXHP:       script_pushint(st, ed->base_status.max_hp); break;
		case UDT_SP:          script_pushint(st, ed->base_status.sp); break;
		case UDT_MAXSP:       script_pushint(st, ed->base_status.max_sp); break;
		case UDT_MAPIDXY:
			getunitdata_sub(0, ed->bl.m);
			getunitdata_sub(1, ed->bl.x);
			getunitdata_sub(2, ed->bl.y);
			break;
		case UDT_SPEED:       script_pushint(st, ed->base_status.speed); break;
		case UDT_LOOKDIR:     script_pushint(st, ed->ud.dir); break;
		case UDT_CANMOVETICK: script_pushint(st, ed->ud.canmove_tick); break;
		case UDT_MODE:        script_pushint(st, ed->base_status.mode); break;
		case UDT_STR:         script_pushint(st, ed->base_status.str); break;
		case UDT_AGI:         script_pushint(st, ed->base_status.agi); break;
		case UDT_VIT:         script_pushint(st, ed->base_status.vit); break;
		case UDT_INT:         script_pushint(st, ed->base_status.int_); break;
		case UDT_DEX:         script_pushint(st, ed->base_status.dex); break;
		case UDT_LUK:         script_pushint(st, ed->base_status.luk); break;
		case UDT_ATKRANGE:    script_pushint(st, ed->base_status.rhw.range); break;
		case UDT_ATKMIN:      script_pushint(st, ed->base_status.rhw.atk); break;
		case UDT_ATKMAX:      script_pushint(st, ed->base_status.rhw.atk2); break;
		case UDT_MATKMIN:     script_pushint(st, ed->base_status.matk_min); break;
		case UDT_MATKMAX:     script_pushint(st, ed->base_status.matk_max); break;
		case UDT_DEF:         script_pushint(st, ed->base_status.def); break;
		case UDT_MDEF:        script_pushint(st, ed->base_status.mdef); break;
		case UDT_HIT:         script_pushint(st, ed->base_status.hit); break;
		case UDT_FLEE:        script_pushint(st, ed->base_status.flee); break;
		case UDT_PDODGE:      script_pushint(st, ed->base_status.flee2); break;
		case UDT_CRIT:        script_pushint(st, ed->base_status.cri); break;
		case UDT_RACE:        script_pushint(st, ed->base_status.race); break;
		case UDT_ELETYPE:     script_pushint(st, ed->base_status.def_ele); break;
		case UDT_ELELEVEL:    script_pushint(st, ed->base_status.ele_lv); break;
		case UDT_AMOTION:     script_pushint(st, ed->base_status.amotion); break;
		case UDT_ADELAY:      script_pushint(st, ed->base_status.adelay); break;
		case UDT_DMOTION:     script_pushint(st, ed->base_status.dmotion); break;
		case UDT_MASTERCID:   script_pushint(st, ed->elemental.char_id); break;
		default:
			ShowWarning("buildin_getunitdata: Invalid data type '%s' for Elemental unit.\n", udtype);
			script_pushint(st, -1);
			return false;
		}
	}
		break;
	case BL_NPC:
	{
		const struct npc_data *nd = BL_UCAST(BL_NPC, bl);

		nullpo_retr(false, nd);

		switch (type)
		{
		case UDT_TYPE:        script_pushint(st, BL_NPC); break;
		case UDT_SIZE:        script_pushint(st, nd->status.size); break;
		case UDT_HP:          script_pushint(st, nd->status.hp); break;
		case UDT_MAXHP:       script_pushint(st, nd->status.max_hp); break;
		case UDT_SP:          script_pushint(st, nd->status.sp); break;
		case UDT_MAXSP:       script_pushint(st, nd->status.max_sp); break;
		case UDT_MAPIDXY:
			getunitdata_sub(0, bl->m);
			getunitdata_sub(1, bl->x);
			getunitdata_sub(2, bl->y);
			break;
		case UDT_SPEED:       script_pushint(st, nd->status.speed); break;
		case UDT_LOOKDIR:     script_pushint(st, nd->ud->dir); break;
		case UDT_CANMOVETICK: script_pushint(st, nd->ud->canmove_tick); break;
		case UDT_MODE:        script_pushint(st, nd->status.mode); break;
		case UDT_STR:         script_pushint(st, nd->status.str); break;
		case UDT_AGI:         script_pushint(st, nd->status.agi); break;
		case UDT_VIT:         script_pushint(st, nd->status.vit); break;
		case UDT_INT:         script_pushint(st, nd->status.int_); break;
		case UDT_DEX:         script_pushint(st, nd->status.dex); break;
		case UDT_LUK:         script_pushint(st, nd->status.luk); break;
		case UDT_ATKRANGE:    script_pushint(st, nd->status.rhw.range); break;
		case UDT_ATKMIN:      script_pushint(st, nd->status.rhw.atk); break;
		case UDT_ATKMAX:      script_pushint(st, nd->status.rhw.atk2); break;
		case UDT_MATKMIN:     script_pushint(st, nd->status.matk_min); break;
		case UDT_MATKMAX:     script_pushint(st, nd->status.matk_max); break;
		case UDT_DEF:         script_pushint(st, nd->status.def); break;
		case UDT_MDEF:        script_pushint(st, nd->status.mdef); break;
		case UDT_HIT:         script_pushint(st, nd->status.hit); break;
		case UDT_FLEE:        script_pushint(st, nd->status.flee); break;
		case UDT_PDODGE:      script_pushint(st, nd->status.flee2); break;
		case UDT_CRIT:        script_pushint(st, nd->status.cri); break;
		case UDT_RACE:        script_pushint(st, nd->status.race); break;
		case UDT_ELETYPE:     script_pushint(st, nd->status.def_ele); break;
		case UDT_ELELEVEL:    script_pushint(st, nd->status.ele_lv); break;
		case UDT_AMOTION:     script_pushint(st, nd->status.amotion); break;
		case UDT_ADELAY:      script_pushint(st, nd->status.adelay); break;
		case UDT_DMOTION:     script_pushint(st, nd->status.dmotion); break;
		case UDT_SEX:         script_pushint(st, nd->vd.sex); break;
		case UDT_CLASS:       script_pushint(st, nd->vd.class); break;
		case UDT_HAIRSTYLE:   script_pushint(st, nd->vd.hair_style); break;
		case UDT_HAIRCOLOR:   script_pushint(st, nd->vd.hair_color); break;
		case UDT_HEADBOTTOM:  script_pushint(st, nd->vd.head_bottom); break;
		case UDT_HEADMIDDLE:  script_pushint(st, nd->vd.head_mid); break;
		case UDT_HEADTOP:     script_pushint(st, nd->vd.head_top); break;
		case UDT_CLOTHCOLOR:  script_pushint(st, nd->vd.cloth_color); break;
		case UDT_SHIELD:      script_pushint(st, nd->vd.shield); break;
		case UDT_WEAPON:      script_pushint(st, nd->vd.weapon); break;
		case UDT_ROBE:        script_pushint(st, nd->vd.robe); break;
		case UDT_BODY2:       script_pushint(st, nd->vd.body_style); break;
		default:
			ShowWarning("buildin_getunitdata: Invalid data type '%s' for NPC unit.\n", udtype);
			script_pushint(st, -1);
			return false;
		}
	}
		break;
	default:
		ShowError("buildin_getunitdata: Unknown object!\n");
		script_pushint(st, -1);
		return false;
	} // end of bl->type switch

#undef getunitdata_sub

	return true;
}

/**
 * Gets the name of a Unit.
 * Supported types are [MOB|HOM|PET|NPC].
 * MER and ELEM don't support custom names.
 *
 * @command getunitname <GUID>;
 * @param GUID Game Object Unique ID.
 * @return boolean or Name of the game object.
 */
static BUILDIN(getunitname)
{
	const struct block_list* bl = NULL;

	bl = map->id2bl(script_getnum(st, 2));

	if (bl == NULL) {
		ShowWarning("buildin_getunitname: Error in finding object with given game ID %d!\n", script_getnum(st, 2));
		script_pushconststr(st, "Unknown");
		return false;
	}

	script_pushstrcopy(st, status->get_name(bl));

	return true;
}

/**
 * Changes the name of a bl.
 * Supported types are [MOB|HOM|PET].
 * For NPC see 'setnpcdisplay', MER and ELEM don't support custom names.
 *
 * @command setunitname <GUID>,<name>;
 * @param GUID  Game object unique ID.
 * @param Name as string.
 * @return boolean.
 */
static BUILDIN(setunitname)
{
	struct block_list* bl = map->id2bl(script_getnum(st, 2));

	if (bl == NULL) {
		ShowWarning("buildin_setunitname: Game object with ID %d was not found!\n", script_getnum(st, 2));
		script_pushint(st, 0);
		return false;
	}

	switch (bl->type) {
		case BL_MOB:
		{
			struct mob_data *md = BL_UCAST(BL_MOB, bl);
			if (md == NULL) {
				ShowWarning("buildin_setunitname: Error in finding object BL_MOB!\n");
				script_pushint(st, 0);
				return false;
			}
			safestrncpy(md->name, script_getstr(st, 3), NAME_LENGTH);
		}
			break;
		case BL_HOM:
		{
			struct homun_data *hd = BL_UCAST(BL_HOM, bl);
			if (hd == NULL) {
				ShowWarning("buildin_setunitname: Error in finding object BL_HOM!\n");
				script_pushint(st, 0);
				return false;
			}
			safestrncpy(hd->homunculus.name, script_getstr(st, 3), NAME_LENGTH);
		}
			break;
		case BL_PET:
		{
			struct pet_data *pd = BL_UCAST(BL_PET, bl);
			if (pd == NULL) {
				ShowWarning("buildin_setunitname: Error in finding object BL_PET!\n");
				script_pushint(st, 0);
				return false;
			}
			safestrncpy(pd->pet.name, script_getstr(st, 3), NAME_LENGTH);
		}
			break;
		default:
			script_pushint(st, 0);
			ShowWarning("buildin_setunitname: Unknown object type!\n");
			return false;
	}

	script_pushint(st, 1);
	clif->blname_ack(0, bl); // Send update to client.

	return true;
}

static BUILDIN(setunittitle)
{
	struct block_list *bl = map->id2bl(script_getnum(st, 2));
	if (bl == NULL) {
		ShowWarning("buildin_setunittitle: Error in finding object with given game ID %d!\n", script_getnum(st, 2));
		return false;
	}

	struct unit_data *ud = unit->bl2ud2(bl);
	if (ud == NULL) {
		ShowWarning("buildin_setunittitle: Error in finding unit_data for given game ID %d!\n", script_getnum(st, 2));
		return false;
	}

	safestrncpy(ud->title, script_getstr(st, 3), NAME_LENGTH);
	clif->blname_ack(0, bl); // Send update to client.

	return true;
}

static BUILDIN(getunittitle)
{
	struct block_list *bl = map->id2bl(script_getnum(st, 2));
	if (bl == NULL) {
		ShowWarning("buildin_getunitname: Error in finding object with given game ID %d!\n", script_getnum(st, 2));
		script_pushconststr(st, "Unknown");
		return false;
	}

	struct unit_data *ud = unit->bl2ud(bl);
	if (ud == NULL) {
		ShowWarning("buildin_setunittitle: Error in finding unit_data for given game ID %d!\n", script_getnum(st, 2));
		return false;
	}

	script_pushstrcopy(st, ud->title);

	return true;
}

/// Makes the unit walk to target position or target id
/// Returns if it was successfull
///
/// unitwalk(<unit_id>,<x>,<y>) -> <bool>
/// unitwalk(<unit_id>,<target_id>) -> <bool>
static BUILDIN(unitwalk)
{
	struct block_list *bl = map->id2bl(script_getnum(st, 2));

	if (bl == NULL) {
		script_pushint(st, 0);
		return true;
	}

	if (bl->type == BL_NPC) {
		struct unit_data *ud = unit->bl2ud2(bl); // ensure the ((struct npc_data*)bl)->ud is safe to edit
		if (ud == NULL) {
			ShowWarning("buildin_unitwalk: floating NPC don't have unit data.\n");
			return false;
		}
	}
	if (script_hasdata(st, 4)) {
		int x = script_getnum(st, 3);
		int y = script_getnum(st, 4);
		if (unit->walk_toxy(bl, x, y, 0) == 0) // We'll use harder calculations.
			script_pushint(st, 1);
		else
			script_pushint(st, 0);
	}
	else {
		int target_id = script_getnum(st, 3);
		script_pushint(st, unit->walktobl(bl, map->id2bl(target_id), 1, 1));
	}

	return true;
}

/**
 * Checks if a unit is walking.
 *
 * Returns 1 if unit is walking, 0 if unit is not walking and -1 on error.
 *
 * @code{.herc}
 *	unitiswalking({<GID>});
 * @endcode
 *
 **/
static BUILDIN(unitiswalking)
{
	int gid = script_hasdata(st, 2) ? script_getnum(st, 2) : st->rid;
	struct block_list *bl = map->id2bl(gid);

	if (bl == NULL) {
		ShowWarning("buildin_unitiswalking: Error in finding object for GID %d!\n", gid);
		script_pushint(st, -1);
		return false;
	}

	if (unit->bl2ud(bl) == NULL) {
		ShowWarning("buildin_unitiswalking: Error in finding unit_data for GID %d!\n", gid);
		script_pushint(st, -1);
		return false;
	}

	script_pushint(st, unit->is_walking(bl));

	return true;
}

/// Kills the unit
///
/// unitkill <unit_id>;
static BUILDIN(unitkill)
{
	struct block_list* bl = map->id2bl(script_getnum(st,2));
	if( bl != NULL )
		status_kill(bl);

	return true;
}

/// Warps the unit to the target position in the target map
/// Returns if it was successfull
///
/// unitwarp(<unit_id>,"<map name>",<x>,<y>) -> <bool>
static BUILDIN(unitwarp)
{
	int unit_id = script_getnum(st, 2);
	const char *mapname = script_getstr(st, 3);
	short x = (short)script_getnum(st, 4);
	short y = (short)script_getnum(st, 5);
	int mapid;
	struct block_list *bl;

	if (!unit_id) //Warp the script's runner
		bl = map->id2bl(st->rid);
	else
		bl = map->id2bl(unit_id);

	if (strcmp(mapname, "this") == 0)
		mapid = bl ? bl->m : -1;
	else
		mapid = map->mapname2mapid(mapname);

	if (mapid >= 0 && bl != NULL) {
		struct unit_data *ud = unit->bl2ud2(bl); // ensure ((struct npc_data *)bl)->ud is safe to edit
		if (bl->type == BL_NPC) {
			if (ud == NULL) {
				ShowWarning("buildin_unitwarp: floating NPC don't have unit data.\n");
				return false;
			}
		}
		script_pushint(st, unit->warp(bl, mapid, x, y, CLR_OUTSIGHT));
	}
	else {
		script_pushint(st, 0);
	}

	return true;
}

/// Makes the unit attack the target.
/// If the unit is a player and <action type> is not 0, it does a continuous
/// attack instead of a single attack.
/// Returns if the request was successfull.
///
/// unitattack(<unit_id>,"<target name>"{,<action type>}) -> <bool>
/// unitattack(<unit_id>,<target_id>{,<action type>}) -> <bool>
static BUILDIN(unitattack)
{
	struct block_list* unit_bl;
	struct block_list* target_bl = NULL;
	int actiontype = 0;

	// get unit
	unit_bl = map->id2bl(script_getnum(st,2));
	if( unit_bl == NULL ) {
		script_pushint(st, 0);
		return true;
	}

	if (script_isstringtype(st, 3)) {
		struct map_session_data *sd = script->nick2sd(st, script_getstr(st, 3));
		if (sd != NULL)
			target_bl = &sd->bl;
	} else {
		target_bl = map->id2bl(script_getnum(st, 3));
	}
	// request the attack
	if( target_bl == NULL )
	{
		script_pushint(st, 0);
		return true;
	}

	// get actiontype
	if( script_hasdata(st,4) )
		actiontype = script_getnum(st,4);

	switch( unit_bl->type )
	{
		case BL_PC:
			clif->pActionRequest_sub(BL_UCAST(BL_PC, unit_bl), actiontype > 0 ? 0x07 : 0x00, target_bl->id, timer->gettick());
			script_pushint(st, 1);
			return true;
		case BL_MOB:
			BL_UCAST(BL_MOB, unit_bl)->target_id = target_bl->id;
			break;
		case BL_PET:
			BL_UCAST(BL_PET, unit_bl)->target_id = target_bl->id;
			break;
		default:
			ShowError("script:unitattack: unsupported source unit type %u\n", unit_bl->type);
			script_pushint(st, 0);
			return false;
	}
	script_pushint(st, unit->walktobl(unit_bl, target_bl, 65025, 2));
	return true;
}

/// Makes the unit stop attacking and moving
///
/// unitstop <unit_id>;
static BUILDIN(unitstop)
{
	struct block_list *bl = map->id2bl(script_getnum(st, 2));

	if (bl != NULL) {
		struct unit_data *ud = unit->bl2ud2(bl); // ensure ((struct npc_data *)bl)->ud is safe to edit
		if (bl->type == BL_NPC) {
			if (ud == NULL) {
				ShowWarning("buildin_unitstop: floating NPC don't have unit data.\n");
				return false;
			}
		}
		unit->stop_attack(bl);
		unit->stop_walking(bl, STOPWALKING_FLAG_NEXTCELL);
		if (bl->type == BL_MOB)
			BL_UCAST(BL_MOB, bl)->target_id = 0;
	}

	return true;
}

/// Makes the unit say the message
///
/// unittalk(<unit_id>,"<message>"{, show_name{, <send_target>{, <target_id>}}});
static BUILDIN(unittalk)
{
	int unit_id;
	const char* message;
	struct block_list *bl, *target_bl = NULL;
	bool show_name = true;
	enum send_target target = AREA_CHAT_WOC;

	unit_id = script_getnum(st,2);
	message = script_getstr(st, 3);

	if (script_hasdata(st, 4)) {
		show_name = (script_getnum(st, 4) != 0) ? true : false;
	}

	if (script_hasdata(st, 5)) {
		target = script_getnum(st, 5);
	}

	if (script_hasdata(st, 6)) {
		target_bl = map->id2bl(script_getnum(st, 6));
	}

	bl = map->id2bl(unit_id);
	if( bl != NULL ) {
		struct StringBuf sbuf;
		char blname[NAME_LENGTH];
		StrBuf->Init(&sbuf);
		safestrncpy(blname, clif->get_bl_name(bl), sizeof(blname));
		if(bl->type == BL_NPC)
			strtok(blname, "#");
		if (show_name) {
			StrBuf->Printf(&sbuf, "%s : %s", blname, message);
		} else {
			StrBuf->Printf(&sbuf, "%s", message);
		}

		if (bl->type == BL_PC && target == SELF && (target_bl == NULL || bl == target_bl)) {
			clif->notify_playerchat(bl, StrBuf->Value(&sbuf));
		} else {
			clif->disp_overhead(bl, StrBuf->Value(&sbuf), target, target_bl);
		}
		StrBuf->Destroy(&sbuf);
	}

	return true;
}

/// Makes the unit do an emotion
///
/// unitemote <unit_id>,<emotion>;
///
/// @see e_* in const.txt
static BUILDIN(unitemote)
{
	int unit_id;
	int emotion;
	struct block_list* bl;

	unit_id = script_getnum(st,2);
	emotion = script_getnum(st,3);
	bl = map->id2bl(unit_id);
	if( bl != NULL )
		clif->emotion(bl, emotion);

	return true;
}

/// Makes the unit cast the skill on the target or self if no target is specified
///
/// unitskilluseid <unit_id>,<skill_id>,<skill_lv>{,<target_id>};
/// unitskilluseid <unit_id>,"<skill name>",<skill_lv>{,<target_id>};
static BUILDIN(unitskilluseid)
{
	int unit_id;
	uint16 skill_id;
	uint16 skill_lv;
	int target_id;
	struct block_list* bl;

	unit_id  = script_getnum(st,2);
	skill_id = ( script_isstringtype(st, 3) ? skill->name2id(script_getstr(st, 3)) : script_getnum(st, 3) );
	skill_lv = script_getnum(st,4);
	target_id = ( script_hasdata(st,5) ? script_getnum(st,5) : unit_id );

	bl = map->id2bl(unit_id);

	if (bl != NULL) {
		if (bl->type == BL_NPC) {
			struct npc_data *nd = BL_UCAST(BL_NPC, bl);
			if (nd->status.hp == 0) {
				status_calc_npc(nd, SCO_FIRST);
			} else {
				status_calc_npc(nd, SCO_NONE);
			}
		} else if (bl->type == BL_PC) {
			pc->autocast_clear(BL_UCAST(BL_PC, bl));
		}

		unit->skilluse_id(bl, target_id, skill_id, skill_lv);
	}

	return true;
}

/// Makes the unit cast the skill on the target position.
///
/// unitskillusepos <unit_id>,<skill_id>,<skill_lv>,<target_x>,<target_y>;
/// unitskillusepos <unit_id>,"<skill name>",<skill_lv>,<target_x>,<target_y>;
static BUILDIN(unitskillusepos)
{
	int unit_id;
	uint16 skill_id;
	uint16 skill_lv;
	int skill_x;
	int skill_y;
	struct block_list* bl;

	unit_id  = script_getnum(st,2);
	skill_id = ( script_isstringtype(st, 3) ? skill->name2id(script_getstr(st, 3)) : script_getnum(st, 3) );
	skill_lv = script_getnum(st,4);
	skill_x  = script_getnum(st,5);
	skill_y  = script_getnum(st,6);

	bl = map->id2bl(unit_id);

	if (bl != NULL) {
		if (bl->type == BL_NPC) {
			struct npc_data *nd = BL_UCAST(BL_NPC, bl);
			if (nd->status.hp == 0) {
				status_calc_npc(nd, SCO_FIRST);
			} else {
				status_calc_npc(nd, SCO_NONE);
			}
		} else if (bl->type == BL_PC) {
			pc->autocast_clear(BL_UCAST(BL_PC, bl));
		}

		unit->skilluse_pos(bl, skill_x, skill_y, skill_id, skill_lv);
	}

	return true;
}

// <--- [zBuffer] List of mob control commands

/// Pauses the execution of the script, detaching the player
///
/// sleep <mili seconds>;
static BUILDIN(sleep)
{
	int ticks;

	ticks = script_getnum(st,2);

	// detach the player
	script->detach_rid(st);

	if( ticks <= 0 )
	{// do nothing
	}
	else if( st->sleep.tick == 0 )
	{// sleep for the target amount of time
		st->state = RERUNLINE;
		st->sleep.tick = ticks;
	}
	else
	{// sleep time is over
		st->state = RUN;
		st->sleep.tick = 0;
	}
	return true;
}

/// Pauses the execution of the script, keeping the player attached
/// Returns if a player is still attached
///
/// sleep2(<mili secconds>) -> <bool>
static BUILDIN(sleep2)
{
	int ticks;

	ticks = script_getnum(st,2);

	if( ticks <= 0 ) {
		// do nothing
		script_pushint(st, (map->id2sd(st->rid)!=NULL));
	} else if( !st->sleep.tick ) {
		// sleep for the target amount of time
		st->state = RERUNLINE;
		st->sleep.tick = ticks;
	} else {
		// sleep time is over
		st->state = RUN;
		st->sleep.tick = 0;
		script_pushint(st, (map->id2sd(st->rid)!=NULL));
	}
	return true;
}

/// Awakes all the sleep timers of the target npc
///
/// awake "<npc name>";
static BUILDIN(awake)
{
	struct DBIterator *iter;
	struct script_state *tst;
	struct npc_data* nd;

	if( ( nd = npc->name2id(script_getstr(st, 2)) ) == NULL ) {
		ShowError("awake: NPC \"%s\" not found\n", script_getstr(st, 2));
		return false;
	}

	iter = db_iterator(script->st_db);

	for( tst = dbi_first(iter); dbi_exists(iter); tst = dbi_next(iter) ) {
		if( tst->oid == nd->bl.id ) {
			struct map_session_data *sd = map->id2sd(tst->rid);

			if( tst->sleep.timer == INVALID_TIMER ) {// already awake ???
				continue;
			}
			if( (sd && sd->status.char_id != tst->sleep.charid) || (tst->rid && !sd)) {
				// char not online anymore / another char of the same account is online - Cancel execution
				tst->state = END;
				tst->rid = 0;
			}

			timer->delete(tst->sleep.timer, script->run_timer);
			tst->sleep.timer = INVALID_TIMER;
			if(tst->state != RERUNLINE)
				tst->sleep.tick = 0;
			script->run_main(tst);
		}
	}

	dbi_destroy(iter);

	return true;
}

/// Returns a reference to a variable of the target NPC.
/// Returns 0 if an error occurs.
///
/// getvariableofnpc(<variable>, "<npc name>") -> <reference>
static BUILDIN(getvariableofnpc)
{
	struct script_data* data;
	const char* name;
	struct npc_data* nd;

	data = script_getdata(st,2);
	if( !data_isreference(data) )
	{// Not a reference (aka varaible name)
		ShowError("script:getvariableofnpc: not a variable\n");
		script->reportdata(data);
		script_pushnil(st);
		st->state = END;
		return false;
	}

	name = reference_getname(data);
	if( *name != '.' || name[1] == '@' )
	{// not a npc variable
		ShowError("script:getvariableofnpc: invalid scope (not npc variable)\n");
		script->reportdata(data);
		script_pushnil(st);
		st->state = END;
		return false;
	}

	nd = npc->name2id(script_getstr(st,3));
	if( nd == NULL || nd->subtype != SCRIPT || nd->u.scr.script == NULL )
	{// NPC not found or has no script
		ShowError("script:getvariableofnpc: can't find npc %s\n", script_getstr(st,3));
		script_pushnil(st);
		st->state = END;
		return false;
	}

	if( !nd->u.scr.script->local.vars )
		nd->u.scr.script->local.vars = i64db_alloc(DB_OPT_RELEASE_DATA);

	script->push_val(st->stack, C_NAME, reference_getuid(data), &nd->u.scr.script->local);
	return true;
}

static BUILDIN(getvariableofpc)
{
	const char* name;
	struct script_data* data = script_getdata(st, 2);
	struct map_session_data *sd = map->id2sd(script_getnum(st, 3));

	if (!data_isreference(data)) {
		ShowError("script:getvariableofpc: not a variable\n");
		script->reportdata(data);
		script_pushnil(st);
		st->state = END;
		return false;
	}

	name = reference_getname(data);

	switch (*name)
	{
	case '$':
	case '.':
	case '\'':
		ShowError("script:getvariableofpc: illegal scope (not pc variable)\n");
		script->reportdata(data);
		script_pushnil(st);
		st->state = END;
		return false;
	}

	if (sd == NULL)
	{
		// player not found, return default value
		if (script_hasdata(st, 4)) {
			script_pushcopy(st, 4);
		} else if (is_string_variable(name)) {
			script_pushconststr(st, "");
		} else {
			script_pushint(st, 0);
		}
		return true;
	}

	if (!sd->regs.vars)
		sd->regs.vars = i64db_alloc(DB_OPT_BASE);

	script->push_val(st->stack, C_NAME, reference_getuid(data), &sd->regs);
	return true;
}

/// Opens a warp portal.
/// Has no "portal opening" effect/sound, it opens the portal immediately.
///
/// warpportal <source x>,<source y>,"<target map>",<target x>,<target y>;
///
/// @author blackhole89
static BUILDIN(warpportal)
{
	int spx;
	int spy;
	unsigned short map_index;
	int tpx;
	int tpy;
	struct skill_unit_group* group;
	struct block_list* bl;

	bl = map->id2bl(st->oid);
	if( bl == NULL ) {
		ShowError("script:warpportal: npc is needed\n");
		return false;
	}

	spx = script_getnum(st,2);
	spy = script_getnum(st,3);
	map_index = script->mapindexname2id(st,script_getstr(st, 4));
	tpx = script_getnum(st,5);
	tpy = script_getnum(st,6);

	if( map_index == 0 )
		return true;// map not found

	if( bl->type == BL_NPC )
		unit->bl2ud2(bl); // ensure nd->ud is safe to edit

	group = skill->unitsetting(bl, AL_WARP, 4, spx, spy, 0);
	if( group == NULL )
		return true;// failed
	group->val1 = (group->val1<<16)|(short)0;
	group->val2 = (tpx<<16) | tpy;
	group->val3 = map_index;

	return true;
}

static BUILDIN(openmail)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	mail->openmail(sd);

	return true;
}

static BUILDIN(openauction)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	clif->auction_openwindow(sd);

	return true;
}

/// Retrieves the value of the specified flag of the specified cell.
///
/// checkcell("<map name>",<x>,<y>,<type>) -> <bool>
///
/// @see cell_chk* constants in const.txt for the types
static BUILDIN(checkcell)
{
	int16 m = map->mapname2mapid(script_getstr(st,2));
	int16 x = script_getnum(st,3);
	int16 y = script_getnum(st,4);
	cell_chk type = (cell_chk)script_getnum(st,5);

	if ( m == -1 ) {
		ShowWarning("checkcell: Attempted to run on unexsitent map '%s', type %u, x/y %d,%d\n", script_getstr(st,2), type, x, y);
		return true;
	}

	script_pushint(st, map->getcell(m, NULL, x, y, type));

	return true;
}

/// Modifies flags of cells in the specified area.
///
/// setcell "<map name>",<x1>,<y1>,<x2>,<y2>,<type>,<flag>;
///
/// @see cell_* constants in const.txt for the types
static BUILDIN(setcell)
{
	int16 m = map->mapname2mapid(script_getstr(st,2));
	int16 x1 = script_getnum(st,3);
	int16 y1 = script_getnum(st,4);
	int16 x2 = script_getnum(st,5);
	int16 y2 = script_getnum(st,6);
	cell_t type = (cell_t)script_getnum(st,7);
	bool flag = (bool)script_getnum(st,8);

	int x,y;

	if ( m == -1 ) {
		ShowWarning("setcell: Attempted to run on unexistent map '%s', type %u, x1/y1 - %d,%d | x2/y2 - %d,%d\n", script_getstr(st, 2), type, x1, y1, x2, y2);
		return true;
	}

	if( x1 > x2 ) swap(x1,x2);
	if( y1 > y2 ) swap(y1,y2);

	for( y = y1; y <= y2; ++y )
		for( x = x1; x <= x2; ++x )
			map->list[m].setcell(m, x, y, type, flag);

	return true;
}

/*==========================================
 * Mercenary Commands
 *------------------------------------------*/
static BUILDIN(mercenary_create)
{
	struct map_session_data *sd;
	int class_, contract_time;

	if( (sd = script->rid2sd(st)) == NULL || sd->md || sd->status.mer_id != 0 )
		return true;

	class_ = script_getnum(st,2);

	if( !mercenary->class(class_) )
		return true;

	contract_time = script_getnum(st,3);
	mercenary->create(sd, class_, contract_time);
	return true;
}

static BUILDIN(mercenary_heal)
{
	struct map_session_data *sd = script->rid2sd(st);
	int hp, sp;

	if( sd == NULL || sd->md == NULL )
		return true;
	hp = script_getnum(st,2);
	sp = script_getnum(st,3);

	status->heal(&sd->md->bl, hp, sp, STATUS_HEAL_DEFAULT);
	return true;
}

static BUILDIN(mercenary_sc_start)
{
	struct map_session_data *sd = script->rid2sd(st);
	enum sc_type type;
	int tick, val1;

	if( sd == NULL || sd->md == NULL )
		return true;

	type = (sc_type)script_getnum(st,2);
	tick = script_getnum(st,3);
	val1 = script_getnum(st,4);

	status->change_start(NULL, &sd->md->bl, type, 10000, val1, 0, 0, 0, tick, SCFLAG_FIXEDTICK);
	return true;
}

static BUILDIN(mercenary_get_calls)
{
	struct map_session_data *sd = script->rid2sd(st);
	int guild_id;

	if( sd == NULL )
		return true;

	guild_id = script_getnum(st,2);
	switch( guild_id ) {
		case ARCH_MERC_GUILD:
			script_pushint(st,sd->status.arch_calls);
			break;
		case SPEAR_MERC_GUILD:
			script_pushint(st,sd->status.spear_calls);
			break;
		case SWORD_MERC_GUILD:
			script_pushint(st,sd->status.sword_calls);
			break;
		default:
			script_pushint(st,0);
			break;
	}

	return true;
}

static BUILDIN(mercenary_set_calls)
{
	struct map_session_data *sd = script->rid2sd(st);
	int guild_id, value, *calls;

	if( sd == NULL )
		return true;

	guild_id = script_getnum(st,2);
	value = script_getnum(st,3);

	switch( guild_id ) {
		case ARCH_MERC_GUILD:
			calls = &sd->status.arch_calls;
			break;
		case SPEAR_MERC_GUILD:
			calls = &sd->status.spear_calls;
			break;
		case SWORD_MERC_GUILD:
			calls = &sd->status.sword_calls;
			break;
		default:
			return true; // Invalid Guild
	}

	*calls += value;
	*calls = cap_value(*calls, 0, INT_MAX);

	return true;
}

static BUILDIN(mercenary_get_faith)
{
	struct map_session_data *sd = script->rid2sd(st);
	int guild_id;

	if( sd == NULL )
		return true;

	guild_id = script_getnum(st,2);
	switch( guild_id ) {
		case ARCH_MERC_GUILD:
			script_pushint(st,sd->status.arch_faith);
			break;
		case SPEAR_MERC_GUILD:
			script_pushint(st,sd->status.spear_faith);
			break;
		case SWORD_MERC_GUILD:
			script_pushint(st,sd->status.sword_faith);
			break;
		default:
			script_pushint(st,0);
			break;
	}

	return true;
}

static BUILDIN(mercenary_set_faith)
{
	struct map_session_data *sd = script->rid2sd(st);
	int guild_id, value, *calls;

	if( sd == NULL )
		return true;

	guild_id = script_getnum(st,2);
	value = script_getnum(st,3);

	switch( guild_id ) {
		case ARCH_MERC_GUILD:
			calls = &sd->status.arch_faith;
			break;
		case SPEAR_MERC_GUILD:
			calls = &sd->status.spear_faith;
			break;
		case SWORD_MERC_GUILD:
			calls = &sd->status.sword_faith;
			break;
		default:
			return true; // Invalid Guild
	}

	*calls += value;
	*calls = cap_value(*calls, 0, INT_MAX);
	if( mercenary->get_guild(sd->md) == guild_id )
		clif->mercenary_updatestatus(sd,SP_MERCFAITH);

	return true;
}

/*------------------------------------------
 * Book Reading
 *------------------------------------------*/
static BUILDIN(readbook)
{
	struct map_session_data *sd;
	int book_id, page;

	if( (sd = script->rid2sd(st)) == NULL )
		return true;

	book_id = script_getnum(st,2);
	page = script_getnum(st,3);

	clif->readbook(sd->fd, book_id, page);
	return true;
}

/****************************
 * Questlog script commands *
 ****************************/

static BUILDIN(questinfo)
{
	struct npc_data *nd = map->id2nd(st->oid);
	struct questinfo qi = { 0 };
	int icon = script_getnum(st, 2);

	if (nd == NULL)
		return true;

	if (nd->bl.m == -1) {
		ShowWarning("buildin_questinfo: questinfo cannot be set for an npc with no valid map.\n");
		return false;
	}

	qi.icon = quest->questinfo_validate_icon(icon);
	if (script_hasdata(st, 3)) {
		int color = script_getnum(st, 3);
		if (color < 0 || color > 3) {
			ShowWarning("buildin_questinfo: invalid color '%d', defaulting to 0.\n", color);
			script->reportfunc(st);
			color = 0;
		}
		qi.color = (unsigned char)color;
	}

	VECTOR_ENSURE(nd->qi_data, 1, 1);
	VECTOR_PUSH(nd->qi_data, qi);
	map->add_questinfo(nd->bl.m, nd);
	return true;
}

static BUILDIN(setquestinfo)
{
	struct npc_data *nd = map->id2nd(st->oid);
	struct questinfo *qi = NULL;
	uint32 type = script_getnum(st, 2);

	if (nd == NULL)
		return true;

	if (nd->bl.m == -1) {
		ShowWarning("buildin_setquestinfo: questinfo cannot be set for an npc with no valid map.\n");
		return false;
	}

	if (VECTOR_LENGTH(nd->qi_data) == 0) {
		ShowWarning("buildin_setquestinfo: no valide questinfo data has been found for this npc.\n");
		return false;
	}

	qi = &VECTOR_LAST(nd->qi_data);

	switch (type) {
	case QINFO_JOB:
	{
		int jobid = script_getnum(st, 3);
		if (!pc->db_checkid(jobid)) {
			ShowWarning("buildin_setquestinfo: invalid job id given (%d).\n", jobid);
			return false;
		}
		qi->hasJob = true;
		qi->job = jobid;
		break;
	}
	case QINFO_SEX:
	{
		int sex = script_getnum(st, 3);
		if (sex != SEX_MALE && sex != SEX_FEMALE) {
			ShowWarning("buildin_setquestinfo: unsupported sex has been given (%d).\n", sex);
			return false;
		}
		qi->sex_enabled = true;
		qi->sex = sex;
		break;
	}
	case QINFO_BASE_LEVEL:
	{
		int min = script_getnum(st, 3);
		int max = script_getnum(st, 4);
		if (min > max) {
			ShowWarning("buildin_setquestinfo: minimal level (%d) is bigger than the maximal level (%d).\n", min, max);
			return false;
		}
		qi->base_level.min = min;
		qi->base_level.max = max;
		break;
	}
	case QINFO_JOB_LEVEL:
	{
		int min = script_getnum(st, 3);
		int max = script_getnum(st, 4);
		if (min > max) {
			ShowWarning("buildin_setquestinfo: minimal level (%d) is bigger than the maximal level (%d).\n", min, max);
			return false;
		}
		qi->job_level.min = min;
		qi->job_level.max = max;
		break;
	}
	case QINFO_ITEM:
	{
		struct questinfo_itemreq item = { 0 };

		item.nameid = script_getnum(st, 3);
		item.min = script_hasdata(st, 4) ? script_getnum(st, 4) : 0;
		item.max = script_hasdata(st, 5) ? script_getnum(st, 5) : 0;

		if (itemdb->exists(item.nameid) == NULL) {
			ShowWarning("buildin_setquestinfo: non existing item (%d) have been given.\n", item.nameid);
			return false;
		}
		if (item.min > item.max) {
			ShowWarning("buildin_setquestinfo: minimal amount (%d) is bigger than the maximal amount (%d).\n", item.min, item.max);
			return false;
		}
		if (item.min < 0 || item.min > MAX_AMOUNT) {
			ShowWarning("buildin_setquestinfo: given amount (%d) must be bigger than or equal to 0 and smaller than %d.\n", item.min, MAX_AMOUNT + 1);
			return false;
		}
		if (item.max < 0 || item.max > MAX_AMOUNT) {
			ShowWarning("buildin_setquestinfo: given amount (%d) must be bigger than or equal to 0 and smaller than %d.\n", item.max, MAX_AMOUNT + 1);
			return false;
		}
		if (VECTOR_LENGTH(qi->items) == 0)
			VECTOR_INIT(qi->items);
		VECTOR_ENSURE(qi->items, 1, 1);
		VECTOR_PUSH(qi->items, item);
		break;
	}
	case QINFO_HOMUN_LEVEL:
	{
		int min = script_getnum(st, 3);
		if (min > battle_config.hom_max_level && min > battle_config.hom_S_max_level) {
			ShowWarning("buildin_setquestinfo: minimum homunculus level given (%d) is bigger than the max possible level.\n", min);
			return false;
		}
		qi->homunculus.level = min;
		break;
	}
	case QINFO_HOMUN_TYPE:
	{
		int hom_type = script_getnum(st, 3);
		if (hom_type < HT_REG || hom_type > HT_S) {
			ShowWarning("buildin_setquestinfo: invalid homunculus type (%d).\n", hom_type);
			return false;
		}
		qi->homunculus_type = hom_type;
		break;
	}
	case QINFO_QUEST:
	{
		struct questinfo_qreq quest_req = { 0 };
		struct quest_db *quest_data = NULL;

		quest_req.id = script_getnum(st, 3);
		quest_req.state = script_getnum(st, 4);

		quest_data = quest->db(quest_req.id);
		if (quest_data == &quest->dummy) {
			ShowWarning("buildin_setquestinfo: invalid quest given (%d).\n", quest_req.id);
			return false;
		}
		if (quest_req.state < Q_INACTIVE || quest_req.state > Q_COMPLETE) {
			ShowWarning("buildin_setquestinfo: invalid quest state given (%d).\n", quest_req.state);
			return false;
		}

		if (VECTOR_LENGTH(qi->quest_requirement) == 0)
			VECTOR_INIT(qi->quest_requirement);
		VECTOR_ENSURE(qi->quest_requirement, 1, 1);
		VECTOR_PUSH(qi->quest_requirement, quest_req);
		break;
	}
	case QINFO_MERCENARY_CLASS:
	{
		int mer_class = script_getnum(st, 3);

		if (!mercenary->class(mer_class)) {
			ShowWarning("buildin_setquestinfo: invalid mercenary class given (%d).\n", mer_class);
			return false;
		}
		qi->mercenary_class = mer_class;
		break;
	}
	default:
		ShowWarning("buildin_setquestinfo: invalid type given (%u).\n", type);
		return false;
	}
	return true;
}

static BUILDIN(setquest)
{
	int quest_id;
	unsigned int time_limit;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	quest_id = script_getnum(st, 2);
	time_limit = script_hasdata(st, 3) ? script_getnum(st, 3) : 0;

	quest->add(sd, quest_id, time_limit);
	return true;
}

static BUILDIN(erasequest)
{
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	if (script_hasdata(st, 3)) {
		int quest_id;
		if (script_getnum(st, 3) < script_getnum(st, 2)) {
			ShowError("buildin_erasequest: The second quest id must be greater than the id of the first.\n");
			return false;
		}
		for (quest_id = script_getnum(st, 2); quest_id < script_getnum(st, 3); quest_id++) {
			quest->delete(sd, quest_id);
		}
	} else {
		quest->delete(sd, script_getnum(st, 2));
	}

	return true;
}

static BUILDIN(completequest)
{
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	if (script_hasdata(st, 3)) {
		int quest_id;
		if (script_getnum(st, 3) < script_getnum(st, 2)) {
			ShowError("buildin_completequest: The second quest id must be greater than the id of the first.\n");
			return false;
		}
		for (quest_id = script_getnum(st, 2); quest_id < script_getnum(st, 3); quest_id++) {
			quest->update_status(sd, quest_id, Q_COMPLETE);
		}
	} else {
		quest->update_status(sd, script_getnum(st, 2), Q_COMPLETE);
	}

	return true;
}

static BUILDIN(changequest)
{
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	quest->change(sd, script_getnum(st, 2),script_getnum(st, 3));
	return true;
}

static BUILDIN(questactive)
{
	struct map_session_data *sd = script->rid2sd(st);
	int qid, i;

	if (sd == NULL) {
		return true;
	}

	qid = script_getnum(st, 2);

	ARR_FIND(0, sd->avail_quests, i, sd->quest_log[i].quest_id == qid );

	if( i >= sd->avail_quests ) {
		script_pushint(st, 0);
		return true;
	}

	if(sd->quest_log[i].state == Q_ACTIVE)
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return true;
}

static BUILDIN(questprogress)
{
	struct map_session_data *sd = script->rid2sd(st);
	enum quest_check_type type = HAVEQUEST;
	int quest_progress = 0;

	if (sd == NULL)
		return true;

	if (script_hasdata(st, 3))
		type = (enum quest_check_type)script_getnum(st, 3);

	quest_progress = quest->check(sd, script_getnum(st, 2), type);

	// "Fix" returned quest state value to make more sense.
	// 0 = Not Started, 1 = In Progress, 2 = Completed.
	if (quest_progress == -1) // Not found
		quest_progress = 0;
	else if (quest_progress == 0 || quest_progress == 1)
		quest_progress = 1;
	else
		quest_progress = 2;

	script_pushint(st, quest_progress);

	return true;
}

static BUILDIN(showevent)
{
	struct map_session_data *sd = script->rid2sd(st);
	struct npc_data *nd = map->id2nd(st->oid);
	int icon, color = 0;

	if( sd == NULL || nd == NULL )
		return true;

	icon = script_getnum(st, 2);
	if( script_hasdata(st, 3) ) {
		color = script_getnum(st, 3);
		if( color < 0 || color > 3 ) {
			ShowWarning("buildin_showevent: invalid color '%d', changing to 0\n",color);
			script->reportfunc(st);
			color = 0;
		}
	}

	icon = quest->questinfo_validate_icon(icon);

	clif->quest_show_event(sd, &nd->bl, icon, color);
	return true;
}

/*==========================================
 * Achievement System [Smokexyz/Hercules]
 *-----------------------------------------*/
/**
 * Validates an objective index for the given achievement.
 * Can be used for any achievement type.
 * @command achievement_progress(<ach_id>,<obj_idx>,<progress>,<incremental?>{,<char_id>});
 * @param aid         - achievement ID
 * @param obj_idx     - achievement objective index.
 * @param progress    - objective progress towards goal.
 * @Param incremental - (boolean) true to add the progress towards the goal,
 *                      false to use the progress only as a comparand.
 * @param account_id  - (optional) character ID to perform on.
 * @return true on success, false on failure.
 * @push 1 on success, 0 on failure.
 */
static BUILDIN(achievement_progress)
{
	struct map_session_data *sd = script->rid2sd(st);
	int aid = script_getnum(st, 2);
	int obj_idx = script_getnum(st, 3);
	int progress = script_getnum(st, 4);
	int incremental = script_getnum(st, 5);
	int account_id = script_hasdata(st, 6) ? script_getnum(st, 6) : 0;
	const struct achievement_data *ad = NULL;

	if ((ad = achievement->get(aid)) == NULL) {
		ShowError("buildin_achievement_progress: Invalid achievement ID %d received.\n", aid);
		script_pushint(st, 0);
		return false;
	}

	if (obj_idx <= 0 || obj_idx > VECTOR_LENGTH(ad->objective)) {
		ShowError("buildin_achievement_progress: Invalid objective index %d received. (min: %d, max: %d)\n", obj_idx, 0, VECTOR_LENGTH(ad->objective));
		script_pushint(st, 0);
		return false;
	}

	obj_idx--; // convert to array index.

	if (progress <= 0 || progress > VECTOR_INDEX(ad->objective, obj_idx).goal) {
		ShowError("buildin_achievement_progress: Progress exceeds goal limit for achievement id %d.\n", aid);
		script_pushint(st, 0);
		return false;
	}

	if (incremental < 0 || incremental > 1) {
		ShowError("buildin_achievement_progress: Argument 4 expects boolean (0/1). provided value: %d\n", incremental);
		script_pushint(st, 0);
		return false;
	}

	if (script_hasdata(st, 6)) {
		if (account_id <= 0) {
			ShowError("buildin_achievement_progress: Invalid Account id %d provided.\n", account_id);
			script_pushint(st, 0);
			return false;
		} else if ((sd = map->id2sd(account_id)) == NULL) {
			ShowError("buildin_achievement_progress: Account with id %d was not found.\n", account_id);
			script_pushint(st, 0);
			return false;
		}
	}

	if (achievement->validate(sd, aid, obj_idx, progress, incremental ? true : false))
		script_pushint(st, progress);
	else
		script_pushint(st, 0);

	return true;
}

static BUILDIN(achievement_iscompleted)
{
	struct map_session_data *sd = script_hasdata(st, 3) ? map->id2sd(script_getnum(st, 3)) : script->rid2sd(st);
	if (sd == NULL)
		return false;

	int aid = script_getnum(st, 2);
	const struct achievement_data *ad = achievement->get(aid);
	if (ad == NULL) {
		ShowError("buildin_achievement_iscompleted: Invalid Achievement %d provided.\n", aid);
		return false;
	}

	script_pushint(st, achievement->check_complete(sd, ad));
	return true;
}

/*==========================================
 * BattleGround System
 *------------------------------------------*/
static BUILDIN(waitingroom2bg)
{
	struct npc_data *nd;
	struct chat_data *cd;
	const char *map_name, *ev = "", *dev = "";
	int x, y, i, map_index = 0, bg_id, n;

	if( script_hasdata(st,7) )
		nd = npc->name2id(script_getstr(st,7));
	else
		nd = map->id2nd(st->oid);

	if (nd == NULL || (cd = map->id2cd(nd->chat_id)) == NULL) {
		script_pushint(st,0);
		return true;
	}

	map_name = script_getstr(st,2);
	if( strcmp(map_name,"-") != 0 )
	{
		map_index = script->mapindexname2id(st,map_name);
		if( map_index == 0 )
		{ // Invalid Map
			script_pushint(st,0);
			return true;
		}
	}

	x = script_getnum(st,3);
	y = script_getnum(st,4);
	ev = script_getstr(st,5); // Logout Event
	dev = script_getstr(st,6); // Die Event

	if ((bg_id = bg->create(map_index, x, y, ev, dev)) == 0) {
		// Creation failed
		script_pushint(st,0);
		return true;
	}

	Assert_retr(false, cd->users < MAX_CHAT_USERS);
	n = cd->users; // This is always < MAX_CHAT_USERS

	for (i = 0; i < n && i < MAX_BG_MEMBERS; i++) {
		struct map_session_data *sd = cd->usersd[i];
		if (sd != NULL && bg->team_join(bg_id, sd))
			mapreg->setreg(reference_uid(script->add_variable("$@arenamembers"), i), sd->bl.id);
		else
			mapreg->setreg(reference_uid(script->add_variable("$@arenamembers"), i), 0);
	}

	mapreg->setreg(script->add_variable("$@arenamembersnum"), i);
	script_pushint(st,bg_id);
	return true;
}

static BUILDIN(waitingroom2bg_single)
{
	const char* map_name;
	struct npc_data *nd;
	struct chat_data *cd;
	struct map_session_data *sd;
	int x, y, map_index, bg_id;

	bg_id = script_getnum(st,2);
	map_name = script_getstr(st,3);
	if( (map_index = script->mapindexname2id(st,map_name)) == 0 )
		return true; // Invalid Map

	x = script_getnum(st,4);
	y = script_getnum(st,5);
	nd = npc->name2id(script_getstr(st,6));

	if (nd == NULL || (cd = map->id2cd(nd->chat_id)) == NULL || cd->users <= 0)
		return true;

	if( (sd = cd->usersd[0]) == NULL )
		return true;

	if( bg->team_join(bg_id, sd) )
	{
		pc->setpos(sd, map_index, x, y, CLR_TELEPORT);
		script_pushint(st,1);
	}
	else
		script_pushint(st,0);

	return true;
}

static BUILDIN(bg_team_setxy)
{
	struct battleground_data *bgd;
	int bg_id;

	bg_id = script_getnum(st,2);
	if( (bgd = bg->team_search(bg_id)) == NULL )
		return true;

	bgd->x = script_getnum(st,3);
	bgd->y = script_getnum(st,4);
	return true;
}

static BUILDIN(bg_warp)
{
	int x, y, map_index, bg_id;
	const char* map_name;

	bg_id = script_getnum(st,2);
	map_name = script_getstr(st,3);
	if( (map_index = script->mapindexname2id(st,map_name)) == 0 )
		return true; // Invalid Map
	x = script_getnum(st,4);
	y = script_getnum(st,5);
	bg->team_warp(bg_id, map_index, x, y);
	return true;
}

/**
 * Spawns a mob with allegiance to the given battle group.
 *
 * @code{.herc}
 *	bg_monster(<battle group>, "<map name>", <x>, <y>, "<name to show>", <mob id>{, "<event label>"});
 * @endcode
 *
 **/
static BUILDIN(bg_monster)
{
	const char *event = "";

	if (script_hasdata(st, 8)) {
		event = script_getstr(st, 8);
		script->check_event(st, event);
	}

	const char *mapname = script_getstr(st, 3);
	const char *name = script_getstr(st, 6);
	const int bg_id = script_getnum(st, 2);
	const int x = script_getnum(st, 4);
	const int y = script_getnum(st, 5);
	const int mob_id = script_getnum(st, 7);

	script_pushint(st, mob->spawn_bg(mapname, x, y, name, mob_id, event, bg_id, st->oid));
	return true;
}

static BUILDIN(bg_monster_set_team)
{
	int id = script_getnum(st,2),
	bg_id = script_getnum(st,3);
	struct block_list *mbl = map->id2bl(id); // TODO: Why does this not use map->id2md?
	struct mob_data *md = BL_CAST(BL_MOB, mbl);

	if (md == NULL)
		return true;
	md->bg_id = bg_id;

	mob_stop_attack(md);
	mob_stop_walking(md, STOPWALKING_FLAG_NONE);
	md->target_id = md->attacked_id = 0;
	clif->blname_ack(0, &md->bl);

	return true;
}

static BUILDIN(bg_leave)
{
	struct map_session_data *sd = script->rid2sd(st);
	if( sd == NULL || !sd->bg_id )
		return true;

	bg->team_leave(sd,BGTL_LEFT);
	return true;
}

static BUILDIN(bg_destroy)
{
	int bg_id = script_getnum(st,2);
	bg->team_delete(bg_id);
	return true;
}

static BUILDIN(bg_getareausers)
{
	const char *str;
	int16 m, x0, y0, x1, y1;
	int bg_id;
	int i = 0, c = 0;
	struct battleground_data *bgd = NULL;

	bg_id = script_getnum(st,2);
	str = script_getstr(st,3);

	if( (bgd = bg->team_search(bg_id)) == NULL || (m = map->mapname2mapid(str)) < 0 ) {
		script_pushint(st,0);
		return true;
	}

	x0 = script_getnum(st,4);
	y0 = script_getnum(st,5);
	x1 = script_getnum(st,6);
	y1 = script_getnum(st,7);

	for (i = 0; i < MAX_BG_MEMBERS; i++) {
		struct map_session_data *sd = bgd->members[i].sd;
		if (sd == NULL)
			continue;
		if (sd->bl.m != m || sd->bl.x < x0 || sd->bl.y < y0 || sd->bl.x > x1 || sd->bl.y > y1)
			continue;
		c++;
	}

	script_pushint(st,c);
	return true;
}

static BUILDIN(bg_updatescore)
{
	const char *str;
	int16 m;

	str = script_getstr(st,2);
	if( (m = map->mapname2mapid(str)) < 0 )
		return true;

	map->list[m].bgscore_lion = script_getnum(st,3);
	map->list[m].bgscore_eagle = script_getnum(st,4);

	clif->bg_updatescore(m);
	return true;
}

static BUILDIN(bg_get_data)
{
	struct battleground_data *bgd;
	int bg_id = script_getnum(st,2),
	type = script_getnum(st,3);

	if( (bgd = bg->team_search(bg_id)) == NULL )
	{
		script_pushint(st,0);
		return true;
	}

	switch( type )
	{
		case 0: script_pushint(st, bgd->count); break;
		default:
			ShowError("script:bg_get_data: unknown data identifier %d\n", type);
			break;
	}

	return true;
}

/*==========================================
 * Instancing Script Commands
 *------------------------------------------*/

static BUILDIN(instance_create)
{
	const char *name;
	int owner_id, res;
	int type = IOT_PARTY;
	struct map_session_data *sd = map->id2sd(st->rid);

	name = script_getstr(st, 2);
	owner_id = script_getnum(st, 3);
	if( script_hasdata(st,4) ) {
		type = script_getnum(st, 4);
		if( type < IOT_NONE || type >= IOT_MAX ) {
			ShowError("buildin_instance_create: unknown instance type %d for '%s'\n",type,name);
			return true;
		}
	}

	res = instance->create(owner_id, name, (enum instance_owner_type) type);
	if (sd != NULL) {
		switch (res) {
		case -4: // Already exists
			clif->msgtable_str(sd, MSG_MDUNGEON_SUBSCRIPTION_ERROR_DUPLICATE, name);
			break;
		case -3: // No free instances
			clif->msgtable_str(sd, MSG_MDUNGEON_SUBSCRIPTION_ERROR_EXIST, name);
			break;
		case -2: // Invalid type
			clif->msgtable_str(sd, MSG_MDUNGEON_SUBSCRIPTION_ERROR_RIGHT, name);
			break;
		case -1: // Unknown
			clif->msgtable_str(sd, MSG_MDUNGEON_SUBSCRIPTION_ERROR_UNKNOWN, name);
			break;
		default:
			if (res < 0)
				ShowError("buildin_instance_create: failed to unknown reason [%d].\n", res);
		}
	} else {
		const char *err;
		switch (res) {
		case -3:
			err = "No free instances";
			break;
		case -2:
			err = "Invalid party ID";
			break;
		case -1:
			err = "Invalid type";
			break;
		default:
			err = "Unknown";
			break;
		}
		if (res < 0)
			ShowError("buildin_instance_create: %s [%d].\n", err, res);
	}
	script_pushint(st, res);
	return true;
}

static BUILDIN(instance_destroy)
{
	int instance_id = -1;

	if( script_hasdata(st, 2) )
		instance_id = script_getnum(st, 2);
	else if( st->instance_id >= 0 )
		instance_id = st->instance_id;
	else return true;

	if( !instance->valid(instance_id) ) {
		ShowError("buildin_instance_destroy: Trying to destroy invalid instance %d.\n", instance_id);
		return true;
	}

	instance->destroy(instance_id);
	return true;
}

static BUILDIN(instance_attachmap)
{
	const char *map_name = NULL;
	int16 m;
	bool usebasename = false;
	const char *name = script_getstr(st,2);
	int instance_id = script_getnum(st,3);

	if (script_hasdata(st,4) && script_getnum(st,4) > 0)
		usebasename = true;

	if (script_hasdata(st, 5))
		map_name = script_getstr(st, 5);

	if ((m = instance->add_map(name, instance_id, usebasename, map_name)) < 0) { // [Saithis]
		ShowError("buildin_instance_attachmap: instance creation failed (%s): %d\n", name, m);
		script_pushconststr(st, "");
		return true;
	}
	script_pushconststr(st, map->list[m].name);

	return true;
}

static BUILDIN(instance_detachmap)
{
	const char *str;
	int16 m;
	int instance_id = -1;

	str = script_getstr(st, 2);
	if( script_hasdata(st, 3) )
		instance_id = script_getnum(st, 3);
	else if( st->instance_id >= 0 )
		instance_id = st->instance_id;
	else return true;

	if( (m = map->mapname2mapid(str)) < 0 || (m = instance->map2imap(m,instance_id)) < 0 ) {
		ShowError("buildin_instance_detachmap: Trying to detach invalid map %s\n", str);
		return true;
	}

	instance->del_map(m);
	return true;
}

static BUILDIN(instance_attach)
{
	int instance_id = script_getnum(st, 2);

	if( !instance->valid(instance_id) )
		return true;

	st->instance_id = instance_id;
	return true;
}

static BUILDIN(instance_id)
{
	script_pushint(st, st->instance_id);
	return true;
}

static BUILDIN(instance_set_timeout)
{
	int progress_timeout, idle_timeout;
	int instance_id = -1;

	progress_timeout = script_getnum(st, 2);
	idle_timeout = script_getnum(st, 3);

	if( script_hasdata(st, 4) )
		instance_id = script_getnum(st, 4);
	else if( st->instance_id >= 0 )
		instance_id = st->instance_id;
	else return true;

	if( instance_id >= 0 )
		instance->set_timeout(instance_id, progress_timeout, idle_timeout);

	return true;
}

static BUILDIN(instance_init)
{
	int instance_id = script_getnum(st, 2);

	if( !instance->valid(instance_id) ) {
		ShowError("instance_init: invalid instance id %d.\n",instance_id);
		return true;
	}

	if( instance->list[instance_id].state != INSTANCE_IDLE ) {
		ShowError("instance_init: instance already initialized.\n");
		return true;
	}

	instance->start(instance_id);
	return true;
}

static BUILDIN(instance_announce)
{
	int         instance_id = script_getnum(st,2);
	const char *mes         = script_getstr(st,3);
	int         flag        = script_getnum(st,4);
	const char *fontColor   = script_hasdata(st,5) ? script_getstr(st,5) : NULL;
	int         fontType    = script_hasdata(st,6) ? script_getnum(st,6) : 0x190; // default fontType (FW_NORMAL)
	int         fontSize    = script_hasdata(st,7) ? script_getnum(st,7) : 12;    // default fontSize
	int         fontAlign   = script_hasdata(st,8) ? script_getnum(st,8) : 0;     // default fontAlign
	int         fontY       = script_hasdata(st,9) ? script_getnum(st,9) : 0;     // default fontY
	int i;
	size_t len = strlen(mes);
	Assert_retr(false, len < INT_MAX);

	if( instance_id == -1 ) {
		if( st->instance_id >= 0 )
			instance_id = st->instance_id;
		else
			return true;
	}

	if( !instance->valid(instance_id) )
		return true;

	for( i = 0; i < instance->list[instance_id].num_map; i++ )
		map->foreachinmap(script->buildin_announce_sub, instance->list[instance_id].map[i], BL_PC,
		                  mes, (int)len+1, flag&BC_COLOR_MASK, fontColor, fontType, fontSize, fontAlign, fontY);

	return true;
}

static BUILDIN(instance_npcname)
{
	const char *str;
	int instance_id = -1;
	struct npc_data *nd;

	str = script_getstr(st, 2);
	if( script_hasdata(st, 3) )
		instance_id = script_getnum(st, 3);
	else if( st->instance_id >= 0 )
		instance_id = st->instance_id;

	if( instance_id >= 0 && (nd = npc->name2id(str)) != NULL ) {
		static char npcname[NAME_LENGTH];
		snprintf(npcname, sizeof(npcname), "dup_%d_%d", instance_id, nd->bl.id);
		script_pushconststr(st,npcname);
	} else {
		ShowError("script:instance_npcname: invalid instance NPC (instance_id: %d, NPC name: \"%s\".)\n", instance_id, str);
		st->state = END;
		return false;
	}

	return true;
}

static BUILDIN(has_instance)
{
	struct map_session_data *sd;
	const char *str;
	int16 m;
	int instance_id = -1;
	int i = 0, j = 0;
	bool type = strcmp(script->getfuncname(st),"has_instance2") == 0 ? true : false;

	str = script_getstr(st, 2);

	if ((m = map->mapname2mapid(str)) < 0) {
		if (type) {
			script_pushint(st, -1);
		} else {
			script_pushconststr(st, "");
		}
		return true;
	}

	if (script_hasdata(st, 3))
		instance_id = script_getnum(st, 3);
	else if (st->instance_id >= 0)
		instance_id = st->instance_id;
	else if ((sd = script->rid2sd(st)) != NULL) {
		struct party_data *p;
		if (sd->instances) {
			for (i = 0; i < sd->instances; i++) {
				if (sd->instance[i] >= 0) {
					ARR_FIND(0, instance->list[sd->instance[i]].num_map, j, map->list[instance->list[sd->instance[i]].map[j]].instance_src_map == m);
					if (j != instance->list[sd->instance[i]].num_map)
						break;
				}
			}
			if (i != sd->instances) {
				instance_id = sd->instance[i];
			}
		}
		if (instance_id == -1 && sd->status.party_id && (p = party->search(sd->status.party_id)) != NULL && p->instances) {
			for (i = 0; i < p->instances; i++) {
				if (p->instance[i] >= 0) {
					ARR_FIND(0, instance->list[p->instance[i]].num_map, j, map->list[instance->list[p->instance[i]].map[j]].instance_src_map == m);
					if (j != instance->list[p->instance[i]].num_map)
						break;
				}
			}
			if (i != p->instances) {
				instance_id = p->instance[i];
			}
		}
		if (instance_id == -1 && sd->guild && sd->guild->instances) {
			for (i = 0; i < sd->guild->instances; i++) {
				if (sd->guild->instance[i] >= 0) {
					ARR_FIND(0, instance->list[sd->guild->instance[i]].num_map, j, map->list[instance->list[sd->guild->instance[i]].map[j]].instance_src_map == m);
					if (j != instance->list[sd->guild->instance[i]].num_map)
						break;
				}
			}
			if (i != sd->guild->instances)
				instance_id = sd->guild->instance[i];
		}
	}

	if (instance_id == -1) {
		for (i = 0; i < instance->instances; i++) {
			if (instance->list[i].state != INSTANCE_FREE && instance->list[i].owner_type == IOT_NONE && instance->list[i].num_map > 0) {
				ARR_FIND(0, instance->list[i].num_map, j, map->list[instance->list[i].map[j]].instance_src_map == m);
				if (j != instance->list[i].num_map)
					break;
			}
		}
		if (i != instance->instances) {
			instance_id = instance->list[i].id;
		}
	}

	if (!instance->valid(instance_id) || (m = instance->map2imap(m, instance_id)) < 0) {
		if (type) {
			script_pushint(st, -1);
		} else {
			script_pushconststr(st, "");
		}
		return true;
	}

	if (type) {
		script_pushint(st, instance_id);
	} else {
		script_pushconststr(st, map->list[m].name);
	}
	return true;
}

static int buildin_instance_warpall_sub(struct block_list *bl, va_list ap)
{
	struct map_session_data *sd = NULL;
	int map_index = va_arg(ap,int);
	int x = va_arg(ap,int);
	int y = va_arg(ap,int);

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_PC);
	sd = BL_UCAST(BL_PC, bl);

	pc->setpos(sd,map_index,x,y,CLR_TELEPORT);

	return 0;
}

static BUILDIN(instance_warpall)
{
	int16 m;
	int instance_id = -1;
	const char *mapn;
	int x, y;
	int map_index;

	mapn = script_getstr(st,2);
	x    = script_getnum(st,3);
	y    = script_getnum(st,4);

	if( script_hasdata(st,5) )
		instance_id = script_getnum(st,5);
	else if( st->instance_id >= 0 )
		instance_id = st->instance_id;
	else
		return true;

	if( (m = map->mapname2mapid(mapn)) < 0 || (map->list[m].flag.src4instance && (m = instance->mapid2imapid(m, instance_id)) < 0) )
		return true;

	map_index = map_id2index(m);

	map->foreachininstance(script->buildin_instance_warpall_sub, instance_id, BL_PC,map_index,x,y);

	return true;
}

/*==========================================
 * instance_check_party [malufett]
 * Values:
 * party_id : Party ID of the invoking character. [Required Parameter]
 * amount : Amount of needed Partymembers for the Instance. [Optional Parameter]
 * min : Minimum Level needed to join the Instance. [Optional Parameter]
 * max : Maxium Level allowed to join the Instance. [Optional Parameter]
 * Example: instance_check_party (getcharid(1){,amount}{,min}{,max});
 * Example 2: instance_check_party (getcharid(1),1,1,99);
 *------------------------------------------*/
static BUILDIN(instance_check_party)
{
	int amount, min, max, i, party_id, c = 0;
	struct party_data *p = NULL;

	amount = script_hasdata(st,3) ? script_getnum(st,3) : 1; // Amount of needed Partymembers for the Instance.
	min = script_hasdata(st,4) ? script_getnum(st,4) : 1; // Minimum Level needed to join the Instance.
	max  = script_hasdata(st,5) ? script_getnum(st,5) : MAX_LEVEL; // Maxium Level allowed to join the Instance.

	if( min < 1 || min > MAX_LEVEL) {
		ShowError("instance_check_party: Invalid min level, %d\n", min);
		return true;
	} else if(  max < 1 || max > MAX_LEVEL) {
		ShowError("instance_check_party: Invalid max level, %d\n", max);
		return true;
	}

	if( script_hasdata(st,2) )
		party_id = script_getnum(st,2);
	else return true;

	if( !(p = party->search(party_id)) ) {
		script_pushint(st, 0); // Returns false if party does not exist.
		return true;
	}

	for (i = 0; i < MAX_PARTY; i++) {
		struct map_session_data *pl_sd = p->data[i].sd;
		if (pl_sd && map->id2bl(pl_sd->bl.id)) {
			if (pl_sd->status.base_level < min) {
				script_pushint(st, 0);
				return true;
			}
			if (pl_sd->status.base_level > max) {
				script_pushint(st, 0);
				return true;
			}
			c++;
		}
	}

	if(c < amount) {
		script_pushint(st, 0); // Not enough Members in the Party to join Instance.
	} else
		script_pushint(st, 1);

	return true;
}

/*==========================================
 * instance_check_guild
 * Values:
 * guild_id : Guild ID of the invoking character. [Required Parameter]
 * amount : Amount of needed Guild Members for the Instance. [Optional Parameter]
 * min : Minimum Level needed to join the Instance. [Optional Parameter]
 * max : Maxium Level allowed to join the Instance. [Optional Parameter]
 * Example: instance_check_guild (getcharid(2){,amount}{,min}{,max});
 * Example 2: instance_check_guild (getcharid(2),1,1,99);
 *------------------------------------------*/
static BUILDIN(instance_check_guild)
{
	int amount, min, max, i, guild_id, c = 0;
	struct guild *g = NULL;

	amount = script_hasdata(st,3) ? script_getnum(st,3) : 1;
	min = script_hasdata(st,4) ? script_getnum(st,4) : 1;
	max = script_hasdata(st,5) ? script_getnum(st,5) : MAX_LEVEL;

	if( min < 1 || min > MAX_LEVEL ){
		ShowError("instance_check_guild: Invalid min level, %d\n", min);
		return true;
	} else if( max < 1 || max > MAX_LEVEL ){
		ShowError("instance_check_guild: Invalid max level, %d\n", max);
		return true;
	}

	if( script_hasdata(st,2) )
		guild_id = script_getnum(st,2);
	else return true;

	if( !(g = guild->search(guild_id)) ){
		script_pushint(st,0);
		return true;
	}

	for( i = 0; i < MAX_GUILD; i++ ) {
		struct map_session_data *pl_sd = g->member[i].sd;
		if (pl_sd && map->id2bl(pl_sd->bl.id)) {
			if (pl_sd->status.base_level < min) {
				script_pushint(st,0);
				return true;
			}
			if (pl_sd->status.base_level > max) {
				script_pushint(st,0);
				return true;
			}
			c++;
		}
	}

	if( c < amount )
		script_pushint(st,0);
	else
		script_pushint(st,1);

	return true;
}

/*==========================================
 * Custom Fonts
 *------------------------------------------*/
static BUILDIN(setfont)
{
	struct map_session_data *sd = script->rid2sd(st);
	int font = script_getnum(st,2);
	if( sd == NULL )
		return true;

	if( sd->status.font != font )
		sd->status.font = font;
	else
		sd->status.font = 0;

	clif->font(sd);
	return true;
}

static BUILDIN(getfont)
{
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL) {
		script_pushint(st, 0);
		return true;
	}

	script_pushint(st, sd->status.font);
	return true;
}

static int buildin_mobuseskill_sub(struct block_list *bl, va_list ap)
{
	struct mob_data *md = NULL;
	struct block_list *tbl;
	int mobid       = va_arg(ap,int);
	uint16 skill_id = va_arg(ap,int);
	uint16 skill_lv = va_arg(ap,int);
	int casttime    = va_arg(ap,int);
	int cancel      = va_arg(ap,int);
	int emotion     = va_arg(ap,int);
	int target      = va_arg(ap,int);

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_MOB);
	md = BL_UCAST(BL_MOB, bl);

	if( md->class_ != mobid )
		return 0;

	// 0:self, 1:target, 2:master, default:random
	switch( target )
	{
		case 0:  tbl = map->id2bl(md->bl.id); break;
		case 1:  tbl = map->id2bl(md->target_id); break;
		case 2:  tbl = map->id2bl(md->master_id); break;
		default: tbl = battle->get_enemy(&md->bl, DEFAULT_ENEMY_TYPE(md),skill->get_range2(&md->bl, skill_id, skill_lv)); break;
	}

	if( !tbl )
		return 0;

	if( md->ud.skilltimer != INVALID_TIMER ) // Cancel the casting skill.
		unit->skillcastcancel(bl,0);

	if( skill->get_casttype(skill_id) == CAST_GROUND )
		unit->skilluse_pos2(&md->bl, tbl->x, tbl->y, skill_id, skill_lv, casttime, cancel);
	else
		unit->skilluse_id2(&md->bl, tbl->id, skill_id, skill_lv, casttime, cancel);

	clif->emotion(&md->bl, emotion);

	return 0;
}

/*==========================================
 * areamobuseskill "Map Name",<x>,<y>,<range>,<Mob ID>,"Skill Name"/<Skill ID>,<Skill Lv>,<Cast Time>,<Cancelable>,<Emotion>,<Target Type>;
 *------------------------------------------*/
static BUILDIN(areamobuseskill)
{
	struct block_list center;
	int16 m;
	int range,mobid,skill_id,skill_lv,casttime,emotion,target,cancel;

	if( (m = map->mapname2mapid(script_getstr(st,2))) < 0 ) {
		ShowError("areamobuseskill: invalid map name.\n");
		return true;
	}

	if( map->list[m].flag.src4instance && st->instance_id >= 0 && (m = instance->mapid2imapid(m, st->instance_id)) < 0 )
		return true;

	center.m = m;
	center.x = script_getnum(st,3);
	center.y = script_getnum(st,4);
	range = script_getnum(st,5);
	mobid = script_getnum(st,6);
	skill_id = ( script_isstringtype(st, 7) ? skill->name2id(script_getstr(st, 7)) : script_getnum(st, 7) );
	skill_lv = script_getnum(st,8);
	casttime = script_getnum(st,9);
	cancel = script_getnum(st,10);
	emotion = script_getnum(st,11);
	target = script_getnum(st,12);

	map->foreachinrange(script->buildin_mobuseskill_sub, &center, range, BL_MOB, mobid, skill_id, skill_lv, casttime, cancel, emotion, target);
	return true;
}

static BUILDIN(progressbar)
{
	struct map_session_data * sd = script->rid2sd(st);
	const char * color;
	unsigned int second;

	if( !st || !sd )
		return true;

	st->state = STOP;

	color = script_getstr(st,2);
	second = script_getnum(st,3);

	sd->progressbar.npc_id = st->oid;
	sd->progressbar.timeout = timer->gettick() + second*1000;
	sd->state.workinprogress = 3;

	clif->progressbar(sd, (unsigned int)strtoul(color, (char **)NULL, 0), second);
	return true;
}
static BUILDIN(progressbar_unit)
{
	const char *color = script_getstr(st, 2);
	uint32 second = script_getnum(st, 3);

	if (script_hasdata(st, 4)) {
		struct block_list *bl = map->id2bl(script_getnum(st, 4));

		if (bl == NULL) {
			ShowWarning("buildin_progressbar_unit: Error in finding object with given GID %d!\n", script_getnum(st, 4));
			return true;
		}
		clif->progressbar_unit(bl, (unsigned int)strtoul(color, (char **)NULL, 0), second);
	} else {
		struct map_session_data *sd = script->rid2sd(st);

		if (sd == NULL)
			return false;

		clif->progressbar_unit(&sd->bl, (unsigned int)strtoul(color, (char **)NULL, 0), second);
	}
	return true;
}
static BUILDIN(pushpc)
{
	int cells, dx, dy;
	struct map_session_data* sd;

	if((sd = script->rid2sd(st))==NULL)
	{
		return true;
	}

	enum unit_dir dir = script_getnum(st, 2);
	cells = script_getnum(st,3);

	if (dir >= UNIT_DIR_MAX) {
		ShowWarning("buildin_pushpc: Invalid direction %d specified.\n", dir);
		script->reportsrc(st);

		dir %= UNIT_DIR_MAX;  // trim spin-over
	}

	if(!cells)
	{// zero distance
		return true;
	}
	else if(cells<0)
	{// pushing backwards
		dir   = unit_get_opposite_dir(dir);
		cells = -cells;
	}

	Assert_retr(false, dir >= UNIT_DIR_FIRST && dir < UNIT_DIR_MAX);
	dx = dirx[dir];
	dy = diry[dir];

	unit->blown(&sd->bl, dx, dy, cells, 0);
	return true;
}

/// Invokes buying store preparation window
/// buyingstore <slots>;
static BUILDIN(buyingstore)
{
	struct map_session_data* sd;

	if( ( sd = script->rid2sd(st) ) == NULL ) {
		return true;
	}

	buyingstore->setup(sd, script_getnum(st,2));
	return true;
}

/// Invokes search store info window
/// searchstores <uses>,<effect>;
static BUILDIN(searchstores)
{
	unsigned short effect;
	unsigned int uses;
	struct map_session_data* sd;

	if( ( sd = script->rid2sd(st) ) == NULL )
	{
		return true;
	}

	uses   = script_getnum(st,2);
	effect = script_getnum(st,3);

	if( !uses )
	{
		ShowError("buildin_searchstores: Amount of uses cannot be zero.\n");
		return false;
	}

	if( effect > 1 )
	{
		ShowError("buildin_searchstores: Invalid effect id %hu, specified.\n", effect);
		return false;
	}

	searchstore->open(sd, uses, effect);
	return true;
}
/// Displays a number as large digital clock.
/// showdigit <value>[,<type>];
static BUILDIN(showdigit)
{
	unsigned int type = 0;
	int value;
	struct map_session_data* sd;

	if( ( sd = script->rid2sd(st) ) == NULL )
	{
		return true;
	}

	value = script_getnum(st,2);

	if( script_hasdata(st,3) )
	{
		type = script_getnum(st,3);

		if( type > 3 )
		{
			ShowError("buildin_showdigit: Invalid type %u.\n", type);
			return false;
		}
	}

	clif->showdigit(sd, (unsigned char)type, value);
	return true;
}
/**
 * Rune Knight
 **/
static BUILDIN(makerune)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;
	clif->skill_produce_mix_list(sd,RK_RUNEMASTERY,24);
	sd->itemid = script_getnum(st,2);
	return true;
}

/**
 * hascashmount() returns 1 if mounting a cash mount or 0 otherwise
 **/
static BUILDIN(hascashmount)
{
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	if (sd->sc.data[SC_ALL_RIDING]) {
		script_pushint(st, 1);
	} else {
		script_pushint(st, 0);
	}

	return true;
}

/**
 * setcashmount() returns 1 on success or 0 otherwise
 *
 * - Toggles cash mounts on a player when he can mount
 * - Will fail if the player is already riding a standard mount e.g. dragon, peco, wug, mado, etc.
 * - Will unmount the player is he is already mounting a cash mount
 **/
static BUILDIN(setcashmount)
{
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	if (pc_hasmount(sd)) {
#if PACKETVER >= 20110531
		clif->msgtable(sd, MSG_FAIELD_RIDING_OVERLAPPED);
#endif
		script_pushint(st, 0); // Can't mount with one of these
	} else {
		if (sd->sc.data[SC_ALL_RIDING]) {
			status_change_end(&sd->bl, SC_ALL_RIDING, INVALID_TIMER);
		} else {
			sc_start(NULL, &sd->bl, SC_ALL_RIDING, 100, battle_config.boarding_halter_speed, INFINITE_DURATION);
		}
		script_pushint(st, 1); // In both cases, return 1.
	}

	return true;
}

/**
 * Retrieves quantity of arguments provided to callfunc/callsub.
 * getargcount() -> amount of arguments received in a function
 **/
static BUILDIN(getargcount)
{
	struct script_retinfo* ri;

	if( st->stack->defsp < 1 || st->stack->stack_data[st->stack->defsp - 1].type != C_RETINFO ) {
		ShowError("script:getargcount: used out of function or callsub label!\n");
		st->state = END;
		return false;
	}
	ri = st->stack->stack_data[st->stack->defsp - 1].u.ri;

	script_pushint(st, ri->nargs);

	return true;
}

/**
 * getcharip(<account ID>/<character ID>/<character name>)
 **/
static BUILDIN(getcharip)
{
	struct map_session_data* sd = NULL;

	/* check if a character name is specified */
	if (script_hasdata(st, 2)) {
		if (script_isstringtype(st, 2)) {
			sd = map->nick2sd(script_getstr(st, 2), false);
		} else {
			int id = script_getnum(st, 2);
			sd = (map->id2sd(id) ? map->id2sd(id) : map->charid2sd(id));
		}
	} else if ((sd = script->rid2sd(st)) == NULL) {
		script_pushconststr(st, "");
		return true;
	}

	if (sd == NULL) {
		ShowWarning("buildin_getcharip: Player not found!\n");
		script_pushconststr(st, "");
		script->reportfunc(st);
		return false;
	}

	if (sd->fd <= 0 || sockt->session[sd->fd] == NULL || sockt->session[sd->fd]->client_addr == 0) {
		script_pushconststr(st, "");
	} else {
		uint32 ip = sockt->session[sd->fd]->client_addr;
		const char *ip_addr = sockt->ip2str(ip, NULL);
		script_pushstrcopy(st, ip_addr);
	}

	return true;
}

enum function_type {
	FUNCTION_IS_NONE = 0,
	FUNCTION_IS_COMMAND,
	FUNCTION_IS_GLOBAL,
	FUNCTION_IS_LOCAL,
	FUNCTION_IS_LABEL,
};

/**
 * is_function(<function name>)
 **/
static BUILDIN(is_function)
{
	const char *str = script_getstr(st, 2);
	enum function_type type = FUNCTION_IS_NONE;

	// TODO: add support for exported functions (#2142)

	if (strdb_exists(script->userfunc_db, str)) {
		type = FUNCTION_IS_GLOBAL;
	} else {
		int n = script->search_str(str);

		if (n >= 0) {
			switch (script->str_data[n].type) {
			case C_FUNC:
				type = FUNCTION_IS_COMMAND;
				break;
			case C_USERFUNC:
			case C_USERFUNC_POS:
				type = FUNCTION_IS_LOCAL;
				break;
			case C_POS:
				type = FUNCTION_IS_LABEL;
				break;
			case C_NAME:
				if (script->str_data[n].label >= 0) {
					// WTF... ?
					// for some reason local functions can have type C_NAME
					type = FUNCTION_IS_LOCAL;
				}
			}
		}
	}

	script_pushint(st, type);
	return true;
}

/**
 * freeloop(<toggle>) -> toggles this script instance's looping-check ability
 **/
static BUILDIN(freeloop)
{
	if( script_getnum(st,2) )
		st->freeloop = 1;
	else
		st->freeloop = 0;

	script_pushint(st, st->freeloop);

	return true;
}

static BUILDIN(sit)
{
	struct map_session_data *sd = NULL;

	if (script_hasdata(st, 2))
		sd = script->nick2sd(st, script_getstr(st, 2));
	else
		sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	if (!pc_issit(sd))
	{
		pc_setsit(sd);
		skill->sit(sd,1);
		clif->sitting(&sd->bl);
	}
	return true;
}

static BUILDIN(stand)
{
	struct map_session_data *sd = NULL;

	if (script_hasdata(st, 2))
		sd = script->nick2sd(st, script_getstr(st, 2));
	else
		sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	if (pc_issit(sd))
	{
		pc->setstand(sd);
		skill->sit(sd,0);
		clif->standing(&sd->bl);
	}
	return true;
}

static BUILDIN(issit)
{
	struct map_session_data *sd = NULL;

	if (script_hasdata(st, 2))
		sd = script->nick2sd(st, script_getstr(st, 2));
	else
		sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	if (pc_issit(sd))
		script_pushint(st, 1);
	else
		script_pushint(st, 0);
	return true;
}

static BUILDIN(add_group_command)
{
	AtCommandInfo *acmd_d;
	struct atcmd_binding_data *bcmd_d;
	GroupSettings *group;
	int group_index;
	const char *atcmd = script_getstr(st, 2);
	int group_id = script_getnum(st, 3);
	bool self_perm = (script_getnum(st, 4) == 1);
	bool char_perm = (script_getnum(st, 5) == 1);

	if (!pcg->exists(group_id)) {
		ShowWarning("script:add_group_command: group does not exist: %i\n", group_id);
		script_pushint(st, 0);
		return false;
	}

	group = pcg->id2group(group_id);
	group_index = pcg->get_idx(group);

	if ((bcmd_d = atcommand->get_bind_byname(atcmd)) != NULL) {
		bcmd_d->at_groups[group_index] = self_perm;
		bcmd_d->char_groups[group_index] = char_perm;
		script_pushint(st, 1);
		return true;
	} else if ((acmd_d = atcommand->get_info_byname(atcmd)) != NULL) {
		acmd_d->at_groups[group_index] = self_perm;
		acmd_d->char_groups[group_index] = char_perm;
		script_pushint(st, 1);
		return true;
	}

	ShowWarning("script:add_group_command: command does not exist: %s\n", atcmd);
	script_pushint(st, 0);
	return false;
}

/**
 * @commands (script based)
 **/
static BUILDIN(bindatcmd)
{
	const char* atcmd;
	const char* eventName;
	int i, group_lv = 0, group_lv_char = 99;
	bool log = false;
	bool create = false;

	atcmd = script_getstr(st,2);
	eventName = script_getstr(st,3);

	if( *atcmd == atcommand->at_symbol || *atcmd == atcommand->char_symbol )
		atcmd++;

	if( script_hasdata(st,4) ) group_lv = script_getnum(st,4);
	if( script_hasdata(st,5) ) group_lv_char = script_getnum(st,5);
	if( script_hasdata(st,6) ) log = script_getnum(st,6) ? true : false;

	if( atcommand->binding_count == 0 ) {
		CREATE(atcommand->binding,struct atcmd_binding_data*,1);

		create = true;
	} else {
		ARR_FIND(0, atcommand->binding_count, i, strcmp(atcommand->binding[i]->command,atcmd) == 0);
		if( i < atcommand->binding_count ) {/* update existent entry */
			safestrncpy(atcommand->binding[i]->npc_event, eventName, ATCOMMAND_LENGTH);
			atcommand->binding[i]->group_lv = group_lv;
			atcommand->binding[i]->group_lv_char = group_lv_char;
			atcommand->binding[i]->log = log;
		} else
			create = true;
	}

	if( create ) {
		i = atcommand->binding_count;

		if( atcommand->binding_count++ != 0 )
			RECREATE(atcommand->binding,struct atcmd_binding_data*,atcommand->binding_count);

		CREATE(atcommand->binding[i],struct atcmd_binding_data,1);

		safestrncpy(atcommand->binding[i]->command, atcmd, 50);
		safestrncpy(atcommand->binding[i]->npc_event, eventName, 50);
		atcommand->binding[i]->group_lv = group_lv;
		atcommand->binding[i]->group_lv_char = group_lv_char;
		atcommand->binding[i]->log = log;
		CREATE(atcommand->binding[i]->at_groups, char, db_size(pcg->db));
		CREATE(atcommand->binding[i]->char_groups, char, db_size(pcg->db));
	}

	return true;
}

static BUILDIN(unbindatcmd)
{
	const char* atcmd;
	int i =  0;

	atcmd = script_getstr(st, 2);

	if( *atcmd == atcommand->at_symbol || *atcmd == atcommand->char_symbol )
		atcmd++;

	if( atcommand->binding_count == 0 ) {
		script_pushint(st, 0);
		return true;
	}

	ARR_FIND(0, atcommand->binding_count, i, strcmp(atcommand->binding[i]->command, atcmd) == 0);
	if( i < atcommand->binding_count ) {
		int cursor = 0;
		aFree(atcommand->binding[i]->at_groups);
		aFree(atcommand->binding[i]->char_groups);
		aFree(atcommand->binding[i]);
		atcommand->binding[i] = NULL;
		/* compact the list now that we freed a slot somewhere */
		for( i = 0, cursor = 0; i < atcommand->binding_count; i++ ) {
			if( atcommand->binding[i] == NULL )
				continue;

			if( cursor != i ) {
				memmove(&atcommand->binding[cursor], &atcommand->binding[i], sizeof(struct atcmd_binding_data*));
			}

			cursor++;
		}

		if( (atcommand->binding_count = cursor) == 0 )
			aFree(atcommand->binding);

		script_pushint(st, 1);
	} else
		script_pushint(st, 0);/* not found */

	return true;
}

static BUILDIN(useatcmd)
{
	struct map_session_data *sd, *dummy_sd = NULL;
	int fd;
	const char* cmd;

	cmd = script_getstr(st,2);

	if (st->rid) {
		sd = script->rid2sd(st);
		if (sd == NULL)
			return true;
		fd = sd->fd;
	} else {
		// Use a dummy character.
		sd = dummy_sd = pc->get_dummy_sd();
		fd = 0;

		if( st->oid ) {
			struct block_list* bl = map->id2bl(st->oid);
			memcpy(&sd->bl, bl, sizeof(struct block_list));
			if (bl->type == BL_NPC)
				safestrncpy(sd->status.name, BL_UCAST(BL_NPC, bl)->name, NAME_LENGTH);
		}
	}

	// compatibility with previous implementation (deprecated!)
	if( cmd[0] != atcommand->at_symbol ) {
		cmd += strlen(sd->status.name);
		while( *cmd != atcommand->at_symbol && *cmd != 0 )
			cmd++;
	}

	atcommand->exec(fd, sd, cmd, true);
	if (dummy_sd) aFree(dummy_sd);
	return true;
}

static BUILDIN(has_permission)
{
	struct map_session_data *sd;
	enum e_pc_permission perm;

	if (script_hasdata(st, 3)) {
		sd = map->id2sd(script_getnum(st, 3));
	} else {
		sd = script->rid2sd(st);
	}

	if (sd == NULL) {
		script_pushint(st, 0);
		return false;
	}

	if (script_isstringtype(st, 2)) {
		// to check for plugin permissions
		int i = 0, j = -1;
		const char *name = script_getstr(st, 2);
		for (; i < pcg->permission_count; ++i) {
			if (strcmp(pcg->permissions[i].name, name) == 0) {
				j = i;
				break;
			}
		}
		if (j < 0) {
			ShowError("script:has_permission: unknown permission: %s\n", name);
			script_pushint(st, 0);
			return false;
		}
		script_pushint(st, pc_has_permission(sd, pcg->permissions[j].permission));
		return true;
	}

	// to ckeck for built-in permission
	perm = script_getnum(st, 2);
	script_pushint(st, pc_has_permission(sd, perm));
	return true;
}

static BUILDIN(can_use_command)
{
	struct map_session_data *sd;
	const char *cmd = script_getstr(st, 2);

	if (script_hasdata(st, 3)) {
		sd = map->id2sd(script_getnum(st, 3));
	} else {
		sd = script->rid2sd(st);
	}

	if (sd == NULL) {
		script_pushint(st, 0);
		return false;
	}

	script_pushint(st, pc->can_use_command(sd, cmd));
	return true;
}

/* getrandgroupitem <container_item_id>,<quantity> */
static BUILDIN(getrandgroupitem)
{
	struct item_data *data = NULL;
	struct map_session_data *sd = NULL;
	int nameid = script_getnum(st, 2);
	int count = script_getnum(st, 3);

	if( !(data = itemdb->exists(nameid)) ) {
		ShowWarning("buildin_getrandgroupitem: unknown item id %d\n",nameid);
		script_pushint(st, 1);
	} else if ( count <= 0 ) {
		ShowError("buildin_getrandgroupitem: qty is <= 0!\n");
		script_pushint(st, 1);
	} else if ( !data->group ) {
		ShowWarning("buildin_getrandgroupitem: item '%s' (%d) isn't a group!\n",data->name,nameid);
		script_pushint(st, 1);
	} else if( !( sd = script->rid2sd(st) ) ) {
		ShowWarning("buildin_getrandgroupitem: no player attached!! (item %s (%d))\n",data->name,nameid);
		script_pushint(st, 1);
	} else {
		int i, get_count, flag;
		struct item it;

		memset(&it,0,sizeof(it));

		nameid = itemdb->group_item(data->group);

		it.nameid = nameid;
		it.identify = itemdb->isidentified(nameid);

		if (!itemdb->isstackable(nameid))
			get_count = 1;
		else
			get_count = count;

		for (i = 0; i < count; i += get_count) {
			// if not pet egg
			if (!pet->create_egg(sd, nameid)) {
				if ((flag = pc->additem(sd, &it, get_count, LOG_TYPE_SCRIPT))) {
					clif->additem(sd, 0, 0, flag);
					if( pc->candrop(sd,&it) )
						map->addflooritem(&sd->bl, &it, get_count, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0, false);
				}
			}
		}

		script_pushint(st, 0);
	}

	return true;
}

/* cleanmap <map_name>;
 * cleanarea <map_name>, <x0>, <y0>, <x1>, <y1>; */
static int script_cleanfloor_sub(struct block_list *bl, va_list ap)
{
	map->clearflooritem(bl);

	return 0;
}

static BUILDIN(cleanmap)
{
	const char *mapname = script_getstr(st, 2);
	int16 m = map->mapname2mapid(mapname);

	if ( m == -1 )
		return false;

	if ((script_lastdata(st) - 2) < 4) {
		map->foreachinmap(script->cleanfloor_sub, m, BL_ITEM);
	} else {
		int16 x0 = script_getnum(st, 3);
		int16 y0 = script_getnum(st, 4);
		int16 x1 = script_getnum(st, 5);
		int16 y1 = script_getnum(st, 6);
		if (x0 > 0 && y0 > 0 && x1 > 0 && y1 > 0) {
			map->foreachinarea(script->cleanfloor_sub, m, x0, y0, x1, y1, BL_ITEM);
		} else {
			ShowError("cleanarea: invalid coordinate defined!\n");
			return false;
		}
	}

	return true;
}

/* Cast a skill on the attached player.
 * npcskill <skill id>, <skill lvl>, <stat point>, <NPC level>;
 * npcskill "<skill name>", <skill lvl>, <stat point>, <NPC level>; */
static BUILDIN(npcskill)
{
	struct npc_data *nd;
	uint16 skill_id             = script_isstringtype(st, 2) ? skill->name2id(script_getstr(st, 2)) : script_getnum(st, 2);
	unsigned short skill_level  = script_getnum(st, 3);
	unsigned int stat_point     = script_getnum(st, 4);
	unsigned int npc_level      = script_getnum(st, 5);
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	nd = map->id2nd(sd->npc_id);

	if (stat_point > battle_config.max_third_parameter) {
		ShowError("npcskill: stat point exceeded maximum of %d.\n",battle_config.max_third_parameter );
		return false;
	}
	if (npc_level > MAX_LEVEL) {
		ShowError("npcskill: level exceeded maximum of %d.\n", MAX_LEVEL);
		return false;
	}
	if (nd == NULL) {
		return false;
	}

	nd->level = npc_level;
	nd->stat_point = stat_point;

	if (!nd->status.hp) {
		status_calc_npc(nd, SCO_FIRST);
	} else {
		status_calc_npc(nd, SCO_NONE);
	}

	if (skill->get_inf(skill_id)&INF_GROUND_SKILL) {
		unit->skilluse_pos(&nd->bl, sd->bl.x, sd->bl.y, skill_id, skill_level);
	} else {
		unit->skilluse_id(&nd->bl, sd->bl.id, skill_id, skill_level);
	}

	return true;
}

/* Turns a player into a monster and grants SC attribute effect. [malufett/Hercules]
 * montransform <monster name/id>, <duration>, <sc type>, <val1>, <val2>, <val3>, <val4>; */
static BUILDIN(montransform)
{
	int tick;
	enum sc_type type;
	struct block_list* bl;
	int mob_id, val1, val2, val3, val4;
	val1 = val2 = val3 = val4 = 0;

	if( (bl = map->id2bl(st->rid)) == NULL )
		return true;

	if( script_isstringtype(st, 2) ) {
		mob_id = mob->db_searchname(script_getstr(st, 2));
	} else {
		mob_id = mob->db_checkid(script_getnum(st, 2));
	}

	if( mob_id == 0 ) {
		if( script_isstringtype(st, 2) )
			ShowWarning("buildin_montransform: Attempted to use non-existing monster '%s'.\n", script_getstr(st, 2));
		else
			ShowWarning("buildin_montransform: Attempted to use non-existing monster of ID '%d'.\n", script_getnum(st, 2));
		return false;
	}

	tick = script_getnum(st, 3);

	if (script_hasdata(st, 4))
		type = (sc_type)script_getnum(st, 4);
	else
		type = SC_NONE;

	if (script_hasdata(st, 4)) {
		if( !(type > SC_NONE && type < SC_MAX) ) {
			ShowWarning("buildin_montransform: Unsupported status change id %d\n", type);
			return false;
		}
	}

	if (script_hasdata(st, 5))
		val1 = script_getnum(st, 5);

	if (script_hasdata(st, 6))
		val2 = script_getnum(st, 6);

	if (script_hasdata(st, 7))
		val3 = script_getnum(st, 7);

	if (script_hasdata(st, 8))
		val4 = script_getnum(st, 8);

	if (tick != 0) {
		struct map_session_data *sd = script->id2sd(st, bl->id);

		if (sd == NULL)
			return true;

		if( battle_config.mon_trans_disable_in_gvg && map_flag_gvg2(sd->bl.m) ) {
			clif->message(sd->fd, msg_sd(sd,1488)); // Transforming into monster is not allowed in Guild Wars.
			return true;
		}

		if( sd->disguise != -1 ) {
			clif->message(sd->fd, msg_sd(sd,1486)); // Cannot transform into monster while in disguise.
			return true;
		}

		status_change_end(bl, SC_MONSTER_TRANSFORM, INVALID_TIMER); // Clear previous
		sc_start2(NULL, bl, SC_MONSTER_TRANSFORM, 100, mob_id, type, tick);

		if (script_hasdata(st, 4))
			sc_start4(NULL, bl, type, 100, val1, val2, val3, val4, tick);
	}

	return true;
}

/**
 * Returns the queue with he given index, if it exists.
 *
 * @param idx The queue index.
 *
 * @return The queue, or NULL if it doesn't exist.
 */
static struct script_queue *script_hqueue_get(int idx)
{
	if (idx < 0 || idx >= VECTOR_LENGTH(script->hq) || !VECTOR_INDEX(script->hq, idx).valid)
		return NULL;
	return &VECTOR_INDEX(script->hq, idx);
}

/**
 * Creates a new queue.
 *
 * @return The index of the created queue.
 */
static int script_hqueue_create(void)
{
	struct script_queue *queue = NULL;
	int i;

	ARR_FIND(0, VECTOR_LENGTH(script->hq), i, !VECTOR_INDEX(script->hq, i).valid);

	if (i == VECTOR_LENGTH(script->hq)) {
		VECTOR_ENSURE(script->hq, 1, 1);
		VECTOR_PUSHZEROED(script->hq);
	}
	queue = &VECTOR_INDEX(script->hq, i);

	memset(&VECTOR_INDEX(script->hq, i), 0, sizeof(VECTOR_INDEX(script->hq, i)));

	queue->id = i;
	queue->valid = true;
	return i;
}

/**
 * Script command queue: Creates a queue and returns its id.
 *
 * @code{.herc}
 *    .@queue_id = queue();
 * @endcode
 */
static BUILDIN(queue)
{
	script_pushint(st,script->queue_create());
	return true;
}

/**
 * Script command queuesize: Returns the length of the given queue.
 *
 * Returns 0 on error.
 *
 * \code{.herc}
 *    .@size = queuesize(<queue id>);
 * \endcode
 */
static BUILDIN(queuesize)
{
	int idx = script_getnum(st, 2);

	if (idx < 0 || idx >= VECTOR_LENGTH(script->hq) || !VECTOR_INDEX(script->hq, idx).valid) {
		ShowWarning("buildin_queuesize: unknown queue id %d\n",idx);
		script_pushint(st, 0);
		return false;
	}

	script_pushint(st, VECTOR_LENGTH(VECTOR_INDEX(script->hq, idx).entries));
	return true;
}

/**
 * Adds an entry to the given queue.
 *
 * @param idx The queue index.
 * @param var The entry to add.
 * @retval false if the queue is invalid or the entry is already in the queue.
 * @retval true in case of success.
 */
static bool script_hqueue_add(int idx, int var)
{
	int i;
	struct map_session_data *sd = NULL;
	struct script_queue *queue = NULL;

	if (idx < 0 || idx >= VECTOR_LENGTH(script->hq) || !VECTOR_INDEX(script->hq, idx).valid) {
		ShowWarning("script_hqueue_add: unknown queue id %d\n",idx);
		return false;
	}

	queue = &VECTOR_INDEX(script->hq, idx);

	ARR_FIND(0, VECTOR_LENGTH(queue->entries), i, VECTOR_INDEX(queue->entries, i) == var);
	if (i != VECTOR_LENGTH(queue->entries)) {
		return false; // Entry already exists
	}

	VECTOR_ENSURE(queue->entries, 1, 1);
	VECTOR_PUSH(queue->entries, var);

	if (var >= START_ACCOUNT_NUM && (sd = map->id2sd(var)) != NULL) {
		VECTOR_ENSURE(sd->script_queues, 1, 1);
		VECTOR_PUSH(sd->script_queues, idx);
	}
	return true;
}

/**
 * Script command queueadd: Adds a new entry to the given queue.
 *
 * Returns 0 (false) if already in queue or in case of error, 1 (true)
 * otherwise.
 *
 * @code{.herc}
 *    .@size = queuesize(.@queue_id);
 * @endcode
 */
static BUILDIN(queueadd)
{
	int idx = script_getnum(st, 2);
	int var = script_getnum(st, 3);

	if (script->queue_add(idx, var))
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return true;
}

/**
 * Removes an entry from the given queue.
 *
 * @param idx The queue index.
 * @param var The entry to remove.
 * @retval true if the entry was removed.
 * @retval false if the entry wasn't in queue.
 */
static bool script_hqueue_remove(int idx, int var)
{
	int i;
	struct map_session_data *sd = NULL;
	struct script_queue *queue = NULL;

	if (idx < 0 || idx >= VECTOR_LENGTH(script->hq) || !VECTOR_INDEX(script->hq, idx).valid) {
		ShowWarning("script_hqueue_remove: unknown queue id %d (used with var %d)\n",idx,var);
		return false;
	}

	queue = &VECTOR_INDEX(script->hq, idx);

	ARR_FIND(0, VECTOR_LENGTH(queue->entries), i, VECTOR_INDEX(queue->entries, i) == var);
	if (i == VECTOR_LENGTH(queue->entries))
		return false;

	VECTOR_ERASE(queue->entries, i);

	if (var >= START_ACCOUNT_NUM && (sd = map->id2sd(var)) != NULL) {
		ARR_FIND(0, VECTOR_LENGTH(sd->script_queues), i, VECTOR_INDEX(sd->script_queues, i) == queue->id);

		if (i != VECTOR_LENGTH(sd->script_queues))
			VECTOR_ERASE(sd->script_queues, i);
	}
	return true;
}

/**
 * Script command queueremove: Removes an entry from a queue.
 *
 * Returns 1 (true) on success, 0 (false) if the item wasn't in queue.
 *
 * @code{.herc}
 *    queueremove(.@queue_id, .@value);
 * @endcode
 */
static BUILDIN(queueremove)
{
	int idx = script_getnum(st, 2);
	int var = script_getnum(st, 3);

	if (script->queue_remove(idx,var))
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return true;
}

/**
 * Script command queueopt: Modifies the options of a queue.
 *
 * When the <event label> isn't provided, the option is removed.
 *
 * Returns 1 (true) on success, 0 (false) on failure.
 *
 * The optionType is one of:
 * - QUEUEOPT_DEATH
 * - QUEUEOPT_LOGOUT
 * - QUEUEOPT_MAPCHANGE
 *
 * When the QUEUEOPT_MAPCHANGE event is triggered, it sets a temporary
 * character variable @Queue_Destination_Map$ with the destination map name.
 *
 * @code{.herc}
 *    queueopt(.@queue_id, optionType{, <event label>});
 * @endcode
 */
static BUILDIN(queueopt)
{
	int idx = script_getnum(st, 2);
	int var = script_getnum(st, 3);
	struct script_queue *queue = NULL;

	if (idx < 0 || idx >= VECTOR_LENGTH(script->hq) || !VECTOR_INDEX(script->hq, idx).valid) {
		ShowWarning("buildin_queueopt: unknown queue id %d\n",idx);
		script_pushint(st, 0);
		return false;
	}

	queue = &VECTOR_INDEX(script->hq, idx);

	switch (var) {
		case SQO_ONDEATH:
			if (script_hasdata(st, 4))
				safestrncpy(queue->event_death, script_getstr(st, 4), EVENT_NAME_LENGTH);
			else
				queue->event_death[0] = '\0';
			break;
		case SQO_ONLOGOUT:
			if (script_hasdata(st, 4))
				safestrncpy(queue->event_logout, script_getstr(st, 4), EVENT_NAME_LENGTH);
			else
				queue->event_logout[0] = '\0';
			break;
		case SQO_ONMAPCHANGE:
			if (script_hasdata(st, 4))
				safestrncpy(queue->event_mapchange, script_getstr(st, 4), EVENT_NAME_LENGTH);
			else
				queue->event_mapchange[0] = '\0';
			break;
		default:
			ShowWarning("buildin_queueopt: unsupported optionType %d\n",var);
			script_pushint(st, 0);
			return false;
	}
	script_pushint(st, 1);
	return true;
}

/**
 * Deletes a queue.
 *
 * @param idx The queue index.
 *
 * @retval true if the queue was correctly deleted.
 * @retval false if the queue didn't exist.
 */
static bool script_hqueue_del(int idx)
{
	if (!script->queue_clear(idx))
		return false;

	VECTOR_INDEX(script->hq, idx).valid = false;

	return true;
}

/**
 * Script command queuedel: Deletes a queue.
 *
 * Returns 1 (true) on success, 0 (false) if the queue doesn't exist.
 *
 * @code{.herc}
 *    queuedel(.@queue_id);
 * @endcode
 */
static BUILDIN(queuedel)
{
	int idx = script_getnum(st, 2);

	if (script->queue_del(idx))
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return true;
}

/**
 * Clears a queue.
 *
 * @param idx The queue index.
 *
 * @retval true if the queue was correctly cleared.
 * @retval false if the queue didn't exist.
 */
static bool script_hqueue_clear(int idx)
{
	struct script_queue *queue = NULL;

	if (idx < 0 || idx >= VECTOR_LENGTH(script->hq) || !VECTOR_INDEX(script->hq, idx).valid) {
		ShowWarning("script_hqueue_clear: unknown queue id %d\n",idx);
		return false;
	}

	queue = &VECTOR_INDEX(script->hq, idx);

	while (VECTOR_LENGTH(queue->entries) > 0) {
		int entry = VECTOR_POP(queue->entries);
		struct map_session_data *sd = NULL;

		if (entry >= START_ACCOUNT_NUM && (sd = map->id2sd(entry)) != NULL) {
			int i;
			ARR_FIND(0, VECTOR_LENGTH(sd->script_queues), i, VECTOR_INDEX(sd->script_queues, i) == queue->id);

			if (i != VECTOR_LENGTH(sd->script_queues))
				VECTOR_ERASE(sd->script_queues, i);
		}
	}
	VECTOR_CLEAR(queue->entries);

	return true;
}

/**
 * Script command queueiterator: Creates a new queue iterator.
 *
 * Returns the iterator's id or -1 in case of failure.
 *
 * @code{.herc}
 *    .@id = queueiterator(.@queue_id);
 * @endcode
 */
static BUILDIN(queueiterator)
{
	int qid = script_getnum(st, 2);
	struct script_queue *queue = NULL;
	struct script_queue_iterator *iter = NULL;
	int i;

	if (qid < 0 || qid >= VECTOR_LENGTH(script->hq) || !VECTOR_INDEX(script->hq, qid).valid || !(queue = script->queue(qid))) {
		ShowWarning("queueiterator: invalid queue id %d\n",qid);
		script_pushint(st, -1);
		return false;
	}

	ARR_FIND(0, VECTOR_LENGTH(script->hqi), i, !VECTOR_INDEX(script->hqi, i).valid);

	if (i == VECTOR_LENGTH(script->hqi)) {
		VECTOR_ENSURE(script->hqi, 1, 1);
		VECTOR_PUSHZEROED(script->hqi);
	}

	iter = &VECTOR_INDEX(script->hqi, i);

	VECTOR_ENSURE(iter->entries, VECTOR_LENGTH(queue->entries), 1);
	VECTOR_PUSHARRAY(iter->entries, VECTOR_DATA(queue->entries), VECTOR_LENGTH(queue->entries));

	iter->pos = 0;
	iter->valid = true;

	script_pushint(st, i);
	return true;
}

/**
 * Script command qiget: returns the next/first member in the iterator.
 *
 * Returns 0 if there's no next item.
 *
 * @code{.herc}
 *    for (.@i = qiget(.@iter); qicheck(.@iter); .@i = qiget(.@iter)) {
 *        // ...
 *    }
 * @endcode
 */
static BUILDIN(qiget)
{
	int idx = script_getnum(st, 2);
	struct script_queue_iterator *it = NULL;

	if (idx < 0 || idx >= VECTOR_LENGTH(script->hqi) || !VECTOR_INDEX(script->hqi, idx).valid) {
		ShowWarning("buildin_qiget: unknown queue iterator id %d\n",idx);
		script_pushint(st, 0);
		return false;
	}

	it = &VECTOR_INDEX(script->hqi, idx);

	if (it->pos >= VECTOR_LENGTH(it->entries)) {
		if (it->pos == VECTOR_LENGTH(it->entries))
			++it->pos; // Move beyond the last position to invalidate qicheck
		script_pushint(st, 0);
		return true;
	}
	script_pushint(st, VECTOR_INDEX(it->entries, it->pos++));
	return true;
}

/**
 * Script command qicheck: Checks if the current member in the given iterator (from the last call to qiget) exists.
 *
 * Returns 1 if it exists, 0 otherwise.
 *
 * @code{.herc}
 *    for (.@i = qiget(.@iter); qicheck(.@iter); .@i = qiget(.@iter)) {
 *        // ...
 *    }
 * @endcode
 */
static BUILDIN(qicheck)
{
	int idx = script_getnum(st, 2);
	struct script_queue_iterator *it = NULL;

	if (idx < 0 || idx >= VECTOR_LENGTH(script->hqi) || !VECTOR_INDEX(script->hqi, idx).valid) {
		ShowWarning("buildin_qicheck: unknown queue iterator id %d\n",idx);
		script_pushint(st, 0);
		return false;
	}

	it = &VECTOR_INDEX(script->hqi, idx);

	if (it->pos <= 0 || it->pos > VECTOR_LENGTH(it->entries)) {
		script_pushint(st, 0);
	} else {
		script_pushint(st, 1);
	}

	return true;
}

/**
 * Script command qiclear: Destroys a queue iterator.
 *
 * Returns true (1) on success, false (0) on failure.
 *
 * @code{.herc}
 *    qiclear(.@iter);
 * @endcode
 */
static BUILDIN(qiclear)
{
	int idx = script_getnum(st, 2);
	struct script_queue_iterator *it = NULL;

	if (idx < 0 || idx >= VECTOR_LENGTH(script->hqi) || !VECTOR_INDEX(script->hqi, idx).valid) {
		ShowWarning("buildin_qiclear: unknown queue iterator id %d\n",idx);
		script_pushint(st, 0);
		return false;
	}

	it = &VECTOR_INDEX(script->hqi, idx);

	VECTOR_CLEAR(it->entries);
	it->pos = 0;
	it->valid = false;

	script_pushint(st, 1);
	return true;
}

/**
 * packageitem({<optional container_item_id>})
 * when no item id is provided it tries to assume it comes from the current item id being processed (if any)
 **/
static BUILDIN(packageitem)
{
	struct item_data *data = NULL;
	struct map_session_data *sd = NULL;
	int nameid;

	if( script_hasdata(st, 2) )
		nameid = script_getnum(st, 2);
	else if ( script->current_item_id )
		nameid = script->current_item_id;
	else {
		ShowWarning("buildin_packageitem: no item id provided and no item attached\n");
		script_pushint(st, 1);
		return true;
	}

	if( !(data = itemdb->exists(nameid)) ) {
		ShowWarning("buildin_packageitem: unknown item id %d\n",nameid);
		script_pushint(st, 1);
	} else if ( !data->package ) {
		ShowWarning("buildin_packageitem: item '%s' (%d) isn't a package!\n",data->name,nameid);
		script_pushint(st, 1);
	} else if( !( sd = script->rid2sd(st) ) ) {
		ShowWarning("buildin_packageitem: no player attached!! (item %s (%d))\n",data->name,nameid);
		script_pushint(st, 1);
	} else {
		itemdb->package_item(sd,data->package);
		script_pushint(st, 0);
	}

	return true;
}
/* New Battlegrounds Stuff */
/* bg_team_create(map_name,respawn_x,respawn_y) */
/* returns created team id or -1 when fails */
static BUILDIN(bg_create_team)
{
	const char *map_name, *ev = "", *dev = "";//ev and dev will be dropped.
	int x, y, map_index = 0, bg_id;

	map_name = script_getstr(st,2);
	if( strcmp(map_name,"-") != 0 ) {
		map_index = script->mapindexname2id(st,map_name);
		if( map_index == 0 ) { // Invalid Map
			script_pushint(st, -1);
			return true;
		}
	}

	x = script_getnum(st,3);
	y = script_getnum(st,4);

	if( (bg_id = bg->create(map_index, x, y, ev, dev)) == 0 ) { // Creation failed
		script_pushint(st,-1);
	} else
		script_pushint(st,bg_id);

	return true;

}
/* bg_join_team(team_id{,optional account id}) */
/* when account id is not present it tries to autodetect from the attached player (if any) */
/* returns 0 when successful, 1 otherwise */
static BUILDIN(bg_join_team)
{
	struct map_session_data *sd;
	int team_id = script_getnum(st, 2);

	if (script_hasdata(st, 3))
		sd = script->id2sd(st, script_getnum(st, 3));
	else
		sd = script->rid2sd(st);

	if (sd == NULL)
		script_pushint(st, -1);
	else
		script_pushint(st,bg->team_join(team_id, sd)?0:1);

	return true;
}
/*==============[Mhalicot]==================
 * countbound {<type>};
 * Creates an array of bounded item IDs
 * Returns amount of items found
 * Type:
 * 1 - Account Bound
 * 2 - Guild Bound
 * 3 - Party Bound
 * 4 - Character Bound
 *------------------------------------------*/
static BUILDIN(countbound)
{
	int type, j=0, k=0;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	type = script_hasdata(st,2)?script_getnum(st,2):0;

	for (int i = 0; i < sd->status.inventorySize; i++) {
		if(sd->status.inventory[i].nameid > 0 && (
			(!type && sd->status.inventory[i].bound > 0) ||
			(type && sd->status.inventory[i].bound == type)
		)) {
			pc->setreg(sd,reference_uid(script->add_variable("@bound_items"), k),sd->status.inventory[i].nameid);
			k++;
			j += sd->status.inventory[i].amount;
		}
	}

	script_pushint(st,j);
	return true;
}

/*==========================================
 * checkbound(<item_id>{,<bound_type>{,<refine>{,<attribute>{,<card_1>{,<card_2>{,<card_3>{,<card_4>}}}}}}});
 * Checks to see if specified item is in inventory.
 * Returns the bound type of item found.
 * Type:
 * 1 - Account Bound
 * 2 - Guild Bound
 * 3 - Party Bound
 * 4 - Character Bound
 *------------------------------------------*/
static BUILDIN(checkbound)
{
	int i, nameid = script_getnum(st,2);
	int bound_type = 0;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	if( !(itemdb->exists(nameid)) ){
		ShowError("script_checkbound: Invalid item ID = %d\n", nameid);
		return false;
	}

	if (script_hasdata(st,3))
		bound_type = script_getnum(st,3);

	if( bound_type <= -1 || bound_type > IBT_MAX ){
		ShowError("script_checkbound: Not a valid bind type! Type=%d\n", bound_type);
	}

	ARR_FIND(0, sd->status.inventorySize, i, (sd->status.inventory[i].nameid == nameid &&
			( sd->status.inventory[i].refine == (script_hasdata(st,4)? script_getnum(st,4) : sd->status.inventory[i].refine) ) &&
			( sd->status.inventory[i].attribute == (script_hasdata(st,5)? script_getnum(st,5) : sd->status.inventory[i].attribute) ) &&
			( sd->status.inventory[i].card[0] == (script_hasdata(st,6)? script_getnum(st,6) : sd->status.inventory[i].card[0]) ) &&
			( sd->status.inventory[i].card[1] == (script_hasdata(st,7)? script_getnum(st,7) : sd->status.inventory[i].card[1]) ) &&
			( sd->status.inventory[i].card[2] == (script_hasdata(st,8)? script_getnum(st,8) : sd->status.inventory[i].card[2]) ) &&
			( sd->status.inventory[i].card[3] == (script_hasdata(st,9)? script_getnum(st,9) : sd->status.inventory[i].card[3]) ) &&
			((sd->status.inventory[i].bound > 0 && !bound_type) || sd->status.inventory[i].bound == bound_type)));

	if (i < sd->status.inventorySize) {
		script_pushint(st, sd->status.inventory[i].bound);
		return true;
	} else {
		script_pushint(st,0);
	}

	return true;
}

/* bg_match_over( arena_name {, optional canceled } ) */
/* returns 0 when successful, 1 otherwise */
static BUILDIN(bg_match_over)
{
	bool canceled = script_hasdata(st,3) ? true : false;
	struct bg_arena *arena = bg->name2arena(script_getstr(st, 2));

	if( arena ) {
		bg->match_over(arena,canceled);
		script_pushint(st, 0);
	} else
		script_pushint(st, 1);

	return true;
}

static BUILDIN(instance_mapname)
{
	const char *map_name;
	int m;
	short instance_id = -1;

	map_name = script_getstr(st,2);

	if( script_hasdata(st,3) )
		instance_id = script_getnum(st,3);
	else
		instance_id = st->instance_id;

	// Check that instance mapname is a valid map
	if( instance_id == -1 || (m = instance->mapname2imap(map_name,instance_id)) == -1 )
		script_pushconststr(st, "");
	else
		script_pushconststr(st, map->list[m].name);

	return true;
}

/* modify an instances' reload-spawn point */
/* instance_set_respawn <map_name>,<x>,<y>{,<instance_id>} */
/* returns 1 when successful, 0 otherwise. */
static BUILDIN(instance_set_respawn)
{
	const char *map_name;
	short instance_id = -1;
	short mid;
	short x,y;

	map_name = script_getstr(st,2);
	x = script_getnum(st, 3);
	y = script_getnum(st, 4);

	if( script_hasdata(st, 5) )
		instance_id = script_getnum(st, 5);
	else
		instance_id = st->instance_id;

	if( instance_id == -1 || !instance->valid(instance_id) )
		script_pushint(st, 0);
	else if( (mid = map->mapname2mapid(map_name)) == -1 ) {
		ShowError("buildin_instance_set_respawn: unknown map '%s'\n",map_name);
		script_pushint(st, 0);
	} else {
		int i;

		for(i = 0; i < instance->list[instance_id].num_map; i++) {
			if( map->list[instance->list[instance_id].map[i]].m == mid ) {
				instance->list[instance_id].respawn.map = map_id2index(mid);
				instance->list[instance_id].respawn.x = x;
				instance->list[instance_id].respawn.y = y;
				break;
			}
		}

		if( i != instance->list[instance_id].num_map )
			script_pushint(st, 1);
		else {
			ShowError("buildin_instance_set_respawn: map '%s' not part of instance '%s'\n",map_name,instance->list[instance_id].name);
			script_pushint(st, 0);
		}
	}
	return true;
}

/**
 * @call openshop({NPC Name});
 *
 * @return 1 on success, 0 otherwise.
 **/
static BUILDIN(openshop)
{
	struct npc_data *nd;
	struct map_session_data *sd;

	if (script_hasdata(st, 2)) {
		const char *name = script_getstr(st, 2);
		if (!(nd = npc->name2id(name)) || nd->subtype != SCRIPT) {
			ShowWarning("buildin_openshop(\"%s\"): trying to run without a proper NPC!\n",name);
			return false;
		}
	} else if (!(nd = map->id2nd(st->oid))) {
		ShowWarning("buildin_openshop: trying to run without a proper NPC!\n");
		return false;
	}
	if (!( sd = script->rid2sd(st))) {
		ShowWarning("buildin_openshop: trying to run without a player attached!\n");
		return false;
	} else if (!nd->u.scr.shop || !nd->u.scr.shop->items) {
		ShowWarning("buildin_openshop: trying to open without any items!\n");
		return false;
	}

	if (!npc->trader_open(sd,nd))
		script_pushint(st, 0);
	else
		script_pushint(st, 1);

	return true;
}

static bool script_sellitemcurrency_add(struct npc_data *nd, struct script_state* st, int argIndex)
{
	nullpo_retr(false, nd);
	nullpo_retr(false, st);

	if (!script_hasdata(st, argIndex + 1))
		return false;

	int id = script_getnum(st, argIndex);
	struct item_data *it;
	if (!(it = itemdb->exists(id))) {
		ShowWarning("buildin_sellitemcurrency: unknown item id '%d'!\n", id);
		return false;
	}
	int qty = 0;
	if ((qty = script_getnum(st, argIndex + 1)) <= 0) {
		ShowError("buildin_sellitemcurrency: invalid 'qty'!\n");
		return false;
	}
	int refine_level = -1;
	if (script_hasdata(st, argIndex + 2)) {
		refine_level = script_getnum(st, argIndex + 2);
	}
	int items = nd->u.scr.shop->items;
	if (nd->u.scr.shop == NULL || items == 0) {
		ShowWarning("buildin_sellitemcurrency: shop not have items!\n");
		return false;
	}
	if (nd->u.scr.shop->shop_last_index >= items || nd->u.scr.shop->shop_last_index < 0) {
		ShowWarning("buildin_sellitemcurrency: wrong selected shop index!\n");
		return false;
	}

	struct npc_item_list *item_list = &nd->u.scr.shop->item[nd->u.scr.shop->shop_last_index];
	int index = item_list->value2;
	if (item_list->currency == NULL) {
		CREATE(item_list->currency, struct npc_barter_currency, 1);
		item_list->value2 ++;
	} else {
		RECREATE(item_list->currency, struct npc_barter_currency, ++item_list->value2);
	}
	struct npc_barter_currency *currency = &item_list->currency[index];
	currency->nameid = id;
	currency->refine = refine_level;
	currency->amount = qty;
	return true;
}

/**
 * @call sellitemcurrency <Item_ID>,qty{,refine}};
 *
 * adds <Item_ID> to last item in expanded barter shop
 **/
static BUILDIN(sellitemcurrency)
{
	struct npc_data *nd;
	if ((nd = map->id2nd(st->oid)) == NULL) {
		ShowWarning("buildin_sellitemcurrency: trying to run without a proper NPC!\n");
		return false;
	}
	if (nd->u.scr.shop == NULL || nd->u.scr.shop->type != NST_EXPANDED_BARTER) {
		ShowWarning("buildin_sellitemcurrency: this command can be used only with expanded barter shops!\n");
		return false;
	}

	script->sellitemcurrency_add(nd, st, 2);
	return true;
}

/**
 * @call endsellitem;
 *
 * complete sell item in expanded barter shop (NST_EXPANDED_BARTER)
 **/
static BUILDIN(endsellitem)
{
	struct npc_data *nd;
	if ((nd = map->id2nd(st->oid)) == NULL) {
		ShowWarning("buildin_endsellitem: trying to run without a proper NPC!\n");
		return false;
	}
	if (nd->u.scr.shop == NULL || nd->u.scr.shop->type != NST_EXPANDED_BARTER) {
		ShowWarning("buildin_endsellitem: this command can be used only with expanded barter shops!\n");
		return false;
	}

	int newIndex = nd->u.scr.shop->shop_last_index;
	const struct npc_item_list *const newItem = &nd->u.scr.shop->item[newIndex];
	int i = 0;
	for (i = 0; i < nd->u.scr.shop->items - 1; i++) {
		const struct npc_item_list *const item = &nd->u.scr.shop->item[i];
		if (item->nameid != newItem->nameid || item->value != newItem->value)
			continue;
		if (item->value2 != newItem->value2)
			continue;
		bool found = true;
		for (int k = 0; k < item->value2; k ++) {
			struct npc_barter_currency *currency = &item->currency[k];
			struct npc_barter_currency *newCurrency = &newItem->currency[k];
			if (currency->nameid != newCurrency->nameid ||
			    currency->amount != newCurrency->amount ||
			    currency->refine != newCurrency->refine) {
				found = false;
				break;
			}
		}
		if (!found)
			continue;
		break;
	}

	if (i != nd->u.scr.shop->items - 1) {
		if (nd->u.scr.shop->item[i].qty != -1) {
			nd->u.scr.shop->item[i].qty += nd->u.scr.shop->item[newIndex].qty;
			npc->expanded_barter_tosql(nd, i);
		}
		nd->u.scr.shop->shop_last_index --;
		nd->u.scr.shop->items--;
		if (nd->u.scr.shop->item[newIndex].currency != NULL) {
			aFree(nd->u.scr.shop->item[newIndex].currency);
			nd->u.scr.shop->item[newIndex].currency = NULL;
		}
	}

	return true;
}

/**
 * @call sellitem <Item_ID>,{,price{,qty}};
 *
 * adds <Item_ID> (or modifies if present) to shop
 * if price not provided (or -1) uses the item's value_sell
 **/
static BUILDIN(sellitem)
{
	struct npc_data *nd;
	struct item_data *it;
	int i = 0, id = script_getnum(st,2);
	int value = 0;
	int value2 = 0;
	int qty = 0;

	if( !(nd = map->id2nd(st->oid)) ) {
		ShowWarning("buildin_sellitem: trying to run without a proper NPC!\n");
		return false;
	} else if ( !(it = itemdb->exists(id)) ) {
		ShowWarning("buildin_sellitem: unknown item id '%d'!\n",id);
		return false;
	}

	const bool have_shop = (nd->u.scr.shop != NULL);
	if (!have_shop) {
		npc->trader_update(nd->src_id ? nd->src_id : nd->bl.id);
	}

	if (nd->u.scr.shop->type != NST_BARTER) {
		value = script_hasdata(st, 3) ? script_getnum(st, 3) : it->value_buy;
		if (value == -1)
			value = it->value_buy;
	}

	if (nd->u.scr.shop->type == NST_BARTER) {
		if (!script_hasdata(st, 5)) {
			ShowError("buildin_sellitem: invalid number of parameters for barter-type shop!\n");
			return false;
		}
		value = script_getnum(st, 4);
		value2 = script_getnum(st, 5);
	} else if (nd->u.scr.shop->type == NST_EXPANDED_BARTER) {
		if (!script_hasdata(st, 4)) {
			ShowError("buildin_sellitem: invalid number of parameters for expanded barter type shop!\n");
			return false;
		}
		if ((qty = script_getnum(st, 4)) <= 0 && qty != -1) {
			ShowError("buildin_sellitem: invalid 'qty' for expanded barter type shop!\n");
			return false;
		}
	}

	if (have_shop) {
		if (nd->u.scr.shop->type == NST_BARTER) {
			for (i = 0; i < nd->u.scr.shop->items; i++) {
				const struct npc_item_list *const item = &nd->u.scr.shop->item[i];
				if (item->nameid == id && item->value == value && item->value2 == value2) {
					break;
				}
			}
		} else if (nd->u.scr.shop->type == NST_EXPANDED_BARTER) {
			for (i = 0; i < nd->u.scr.shop->items; i++) {
				const struct npc_item_list *const item = &nd->u.scr.shop->item[i];
				if (item->nameid != id || item->value != value)
					continue;
				if (item->value2 != (script_lastdata(st) - 4) / 3)
					continue;
				bool found = true;
				for (int k = 0; k < item->value2; k ++) {
					const int scriptOffset = k * 3 + 5;
					struct npc_barter_currency *currency = &item->currency[k];
					if (currency->nameid != script_getnum(st, scriptOffset) ||
					    currency->amount != script_getnum(st, scriptOffset + 1) ||
					    currency->refine != script_getnum(st, scriptOffset + 2)) {
						found = false;
						break;
					}
				}
				if (!found)
					continue;
				break;
			}
		} else {
			for (i = 0; i < nd->u.scr.shop->items; i++) {
				if (nd->u.scr.shop->item[i].nameid == id) {
					break;
				}
			}
		}
	}

	if( nd->u.scr.shop->type == NST_MARKET ) {
		if( !script_hasdata(st,4) || ( qty = script_getnum(st, 4) ) <= 0 ) {
			ShowError("buildin_sellitem: invalid 'qty' for market-type shop!\n");
			return false;
		}
	}

	if( ( nd->u.scr.shop->type == NST_ZENY || nd->u.scr.shop->type == NST_MARKET )  && value*0.75 < it->value_sell*1.24 ) {
		ShowWarning("buildin_sellitem: Item %s [%d] discounted buying price (%d->%d) is less than overcharged selling price (%d->%d) in NPC %s (%s)\n",
					it->name, id, value, (int)(value*0.75), it->value_sell, (int)(it->value_sell*1.24), nd->exname, nd->path);
	}

	if (nd->u.scr.shop->type == NST_BARTER) {
		qty = script_getnum(st, 3);
		if (qty < -1 || value <= 0 || value2 <= 0) {
			ShowError("buildin_sellitem: invalid parameters for barter-type shop!\n");
			return false;
		}
	}

	bool foundInShop = (i != nd->u.scr.shop->items);
	if (foundInShop) {
		nd->u.scr.shop->item[i].value = value;
		nd->u.scr.shop->item[i].qty   = qty;
		if (nd->u.scr.shop->type == NST_MARKET) /* has been manually updated, make it reflect on sql */
			npc->market_tosql(nd, i);
		else if (nd->u.scr.shop->type == NST_BARTER) /* has been manually updated, make it reflect on sql */
			npc->barter_tosql(nd, i);
	} else {
		for (i = 0; i < nd->u.scr.shop->items; i++) {
			if (nd->u.scr.shop->item[i].nameid == 0)
				break;
		}

		if (i == nd->u.scr.shop->items) {
			if (nd->u.scr.shop->items == USHRT_MAX) {
				ShowWarning("buildin_sellitem: Can't add %s (%s/%s), shop list is full!\n", it->name, nd->exname, nd->path);
				return false;
			}
			i = nd->u.scr.shop->items;
			RECREATE(nd->u.scr.shop->item, struct npc_item_list, ++nd->u.scr.shop->items);
		}

		nd->u.scr.shop->item[i].nameid = it->nameid;
		nd->u.scr.shop->item[i].value  = value;
		nd->u.scr.shop->item[i].value2 = value2;
		nd->u.scr.shop->item[i].qty    = qty;
		nd->u.scr.shop->item[i].currency = NULL;
	}
	nd->u.scr.shop->shop_last_index = i;

	if (!foundInShop) {
		for (int k = 5; k <= script_lastdata(st); k += 3) {
			script->sellitemcurrency_add(nd, st, k);
		}
	}

	if (foundInShop) {
		if (nd->u.scr.shop->type == NST_EXPANDED_BARTER) {  /* has been manually updated, make it reflect on sql */
			npc->expanded_barter_tosql(nd, i);
		}
	}
	return true;
}

/**
 * @call startsellitem <Item_ID>,{,price{,qty}};
 *
 * Starts adding item into expanded barter shop (NST_EXPANDED_BARTER)
 **/
static BUILDIN(startsellitem)
{
	struct npc_data *nd;
	struct item_data *it;
	int i = 0, id = script_getnum(st,2);
	int value2 = 0;
	int qty = 0;

	if (!(nd = map->id2nd(st->oid))) {
		ShowWarning("buildin_startsellitem: trying to run without a proper NPC!\n");
		return false;
	} else if (!(it = itemdb->exists(id))) {
		ShowWarning("buildin_startsellitem: unknown item id '%d'!\n", id);
		return false;
	}

	const bool have_shop = (nd->u.scr.shop != NULL);
	if (!have_shop) {
		npc->trader_update(nd->src_id ? nd->src_id : nd->bl.id);
	}

	if (nd->u.scr.shop->type != NST_EXPANDED_BARTER) {
		ShowWarning("script_startsellitem: can works only for NST_EXPANDED_BARTER shops");
		return false;
	}

	int value = script_hasdata(st, 3) ? script_getnum(st, 3) : it->value_buy;
	if (value == -1)
		value = it->value_buy;

	if ((qty = script_getnum(st, 4)) <= 0 && qty != -1) {
		ShowError("buildin_startsellitem: invalid 'qty' for expanded barter type shop!\n");
		return false;
	}

	for (i = 0; i < nd->u.scr.shop->items; i++) {
		if (nd->u.scr.shop->item[i].nameid == 0)
			break;
	}

	if (i == nd->u.scr.shop->items) {
		if (nd->u.scr.shop->items == USHRT_MAX) {
			ShowWarning("buildin_startsellitem: Can't add %s (%s/%s), shop list is full!\n", it->name, nd->exname, nd->path);
			return false;
		}
		i = nd->u.scr.shop->items;
		RECREATE(nd->u.scr.shop->item, struct npc_item_list, ++nd->u.scr.shop->items);
	}

	nd->u.scr.shop->item[i].nameid = it->nameid;
	nd->u.scr.shop->item[i].value  = value;
	nd->u.scr.shop->item[i].value2 = value2;
	nd->u.scr.shop->item[i].qty    = qty;
	nd->u.scr.shop->item[i].currency = NULL;
	nd->u.scr.shop->shop_last_index = i;
	return true;
}

/**
 * @call stopselling <Item_ID>;
 *
 * removes <Item_ID> from the current npc shop
 *
 * @return 1 on success, 0 otherwise
 **/
static BUILDIN(stopselling)
{
	struct npc_data *nd;
	int i, id = script_getnum(st, 2);

	if (!(nd = map->id2nd(st->oid)) || !nd->u.scr.shop) {
		ShowWarning("buildin_stopselling: trying to run without a proper NPC!\n");
		return false;
	}

	if (nd->u.scr.shop->type == NST_BARTER) {
		if (!script_hasdata(st, 4)) {
			ShowError("buildin_stopselling: called with wrong number of arguments\n");
			return false;
		}
		const int id2 = script_getnum(st, 3);
		const int amount2 = script_getnum(st, 4);
		for (i = 0; i < nd->u.scr.shop->items; i++) {
			const struct npc_item_list *const item = &nd->u.scr.shop->item[i];
			if (item->nameid == id && item->value == id2 && item->value2 == amount2) {
				break;
			}
		}
	} else if (nd->u.scr.shop->type == NST_EXPANDED_BARTER) {
		if (!script_hasdata(st, 3)) {
			ShowError("buildin_stopselling: called with wrong number of arguments\n");
			return false;
		}
		const int price = script_getnum(st, 3);
		for (i = 0; i < nd->u.scr.shop->items; i++) {
			const struct npc_item_list *const item = &nd->u.scr.shop->item[i];
			if (item->nameid == id && item->value == price) {
				break;
			}
		}
	} else {
		for (i = 0; i < nd->u.scr.shop->items; i++) {
			if (nd->u.scr.shop->item[i].nameid == id) {
				break;
			}
		}
	}

	if (i != nd->u.scr.shop->items) {
		int cursor;

		if (nd->u.scr.shop->type == NST_MARKET)
			npc->market_delfromsql(nd, i);
		else if (nd->u.scr.shop->type == NST_BARTER)
			npc->barter_delfromsql(nd, i);
		else if (nd->u.scr.shop->type == NST_EXPANDED_BARTER)
			npc->expanded_barter_delfromsql(nd, i);

		nd->u.scr.shop->item[i].nameid = 0;
		nd->u.scr.shop->item[i].value  = 0;
		nd->u.scr.shop->item[i].value2 = 0;
		nd->u.scr.shop->item[i].qty    = 0;
		if (nd->u.scr.shop->item[i].currency != NULL) {
			aFree(nd->u.scr.shop->item[i].currency);
			nd->u.scr.shop->item[i].currency = NULL;
		}

		for (i = 0, cursor = 0; i < nd->u.scr.shop->items; i++) {
			if (nd->u.scr.shop->item[i].nameid == 0)
				continue;

			if (cursor != i) {
				nd->u.scr.shop->item[cursor].nameid = nd->u.scr.shop->item[i].nameid;
				nd->u.scr.shop->item[cursor].value  = nd->u.scr.shop->item[i].value;
				nd->u.scr.shop->item[cursor].value2 = nd->u.scr.shop->item[i].value2;
				nd->u.scr.shop->item[cursor].qty    = nd->u.scr.shop->item[i].qty;
				nd->u.scr.shop->item[cursor].currency = nd->u.scr.shop->item[i].currency;
			}

			cursor++;
		}

		nd->u.scr.shop->items--;
		nd->u.scr.shop->item[nd->u.scr.shop->items].currency = NULL;
		script_pushint(st, 1);
	} else {
		script_pushint(st, 0);
	}

	return true;
}

/**
 * @call setcurrency <Val1>{,<Val2>};
 *
 * updates currently-attached player shop currency
 **/
/* setcurrency(<Val1>,{<Val2>}) */
static BUILDIN(setcurrency)
{
	int val1 = script_getnum(st,2),
	val2 = script_hasdata(st, 3) ? script_getnum(st,3) : 0;
	struct npc_data *nd = map->id2nd(st->oid);

	if (!nd) {
		ShowWarning("buildin_setcurrency: trying to run without a proper NPC!\n");
		return false;
	}

	npc->trader_funds[0] = val1;
	npc->trader_funds[1] = val2;

	return true;
}

/**
 * @call tradertype(<type>);
 *
 * defaults to 0, so no need to call when you're doing zeny
 * check enum npc_shop_types for list
 * cleans shop list on use
 **/
static BUILDIN(tradertype)
{
	int type = script_getnum(st, 2);
	struct npc_data *nd;

	if( !(nd = map->id2nd(st->oid)) ) {
		ShowWarning("buildin_tradertype: trying to run without a proper NPC!\n");
		return false;
	} else if ( type < 0 || type > NST_MAX ) {
		ShowWarning("buildin_tradertype: invalid type param %d!\n",type);
		return false;
	}

	if( !nd->u.scr.shop )
		npc->trader_update(nd->src_id?nd->src_id:nd->bl.id);
	else {/* clear list */
		int i;
		for( i = 0; i < nd->u.scr.shop->items; i++ ) {
			nd->u.scr.shop->item[i].nameid = 0;
			nd->u.scr.shop->item[i].value  = 0;
			nd->u.scr.shop->item[i].qty    = 0;
		}
		npc->market_delfromsql(nd, INT_MAX);
		npc->barter_delfromsql(nd, INT_MAX);
		npc->expanded_barter_delfromsql(nd, INT_MAX);
	}

#if PACKETVER < 20131223
	if( type == NST_MARKET ) {
		ShowWarning("buildin_tradertype: NST_MARKET is only available with PACKETVER 20131223 or newer!\n");
		script->reportsrc(st);
	}
#endif
#if PACKETVER_MAIN_NUM < 20190116 && PACKETVER_RE_NUM < 20190116 && PACKETVER_ZERO_NUM < 20181226
	if (type == NST_BARTER) {
		ShowWarning("buildin_tradertype: NST_BARTER is only available with PACKETVER_ZERO_NUM 20181226 or PACKETVER_MAIN_NUM 20190116 or PACKETVER_RE_NUM 20190116 or newer!\n");
		script->reportsrc(st);
	}
#endif
#if PACKETVER_MAIN_NUM < 20191120 && PACKETVER_RE_NUM < 20191106 && PACKETVER_ZERO_NUM < 20191127
	if (type == NST_EXPANDED_BARTER) {
		ShowWarning("buildin_tradertype: NST_EXPANDED_BARTER is only available with PACKETVER_ZERO_NUM 20191127 or PACKETVER_MAIN_NUM 20191120 or PACKETVER_RE_NUM 20191106 or newer!\n");
		script->reportsrc(st);
	}
#endif

	if( nd->u.scr.shop )
		nd->u.scr.shop->type = type;

	return true;
}

/**
 * @call purchaseok();
 *
 * signs the transaction can proceed
 **/
static BUILDIN(purchaseok)
{
	struct npc_data *nd;

	if( !(nd = map->id2nd(st->oid)) || !nd->u.scr.shop ) {
		ShowWarning("buildin_purchaseok: trying to run without a proper NPC!\n");
		return false;
	}

	npc->trader_ok = true;

	return true;
}

/**
 * @call shopcount(<Item_ID>);
 *
 * @return number of available items in the script's attached shop
 **/
static BUILDIN(shopcount)
{
	struct npc_data *nd;
	int id = script_getnum(st, 2);
	unsigned short i;

	if( !(nd = map->id2nd(st->oid)) ) {
		ShowWarning("buildin_shopcount(%d): trying to run without a proper NPC!\n",id);
		return false;
	} else if ( !nd->u.scr.shop || !nd->u.scr.shop->items ) {
		ShowWarning("buildin_shopcount(%d): trying to use without any items!\n",id);
		return false;
	} else if (nd->u.scr.shop->type != NST_MARKET && nd->u.scr.shop->type != NST_BARTER && nd->u.scr.shop->type != NST_EXPANDED_BARTER) {
		ShowWarning("buildin_shopcount(%d): trying to use on a non-NST_MARKET and non-NST_BARTER and non-NST_EXPANDED_BARTER shop!\n",id);
		return false;
	}

	/* lookup */
	for(i = 0; i < nd->u.scr.shop->items; i++) {
		if( nd->u.scr.shop->item[i].nameid == id ) {
			script_pushint(st, nd->u.scr.shop->item[i].qty);
			break;
		}
	}

	/* didn't find it */
	if( i == nd->u.scr.shop->items )
		script_pushint(st, 0);

	return true;
}

/**
 * @call channelmes("#channel", "message");
 *
 * Sends a message through the specified chat channel.
 *
 */
static BUILDIN(channelmes)
{
	struct map_session_data *sd = map->id2sd(st->rid);
	const char *channelname = script_getstr(st, 2);
	struct channel_data *chan = channel->search(channelname, sd);

	if (!chan) {
		script_pushint(st, 0);
		return true;
	}

	channel->send(chan, NULL, script_getstr(st, 3));

	script_pushint(st, 1);
	return true;
}

static BUILDIN(addchannelhandler)
{
	int i;
	struct map_session_data *sd = map->id2sd(st->rid);
	const char *channelname = script_getstr(st, 2);
	const char *eventname = script_getstr(st, 3);
	struct channel_data *chan = channel->search(channelname, sd);

	if (!chan) {
		script_pushint(st, 0);
		return true;
	}

	ARR_FIND(0, MAX_EVENTQUEUE, i, chan->handlers[i][0] == '\0');

	if (i < MAX_EVENTQUEUE) {
		safestrncpy(chan->handlers[i], eventname, EVENT_NAME_LENGTH); //Event enqueued.
		script_pushint(st, 1);
		return true;
	}

	ShowWarning("script:addchannelhandler: too many handlers for channel %s.\n", channelname);
	script_pushint(st, 0);
	return true;
}

static BUILDIN(removechannelhandler)
{
	int i;
	struct map_session_data *sd = map->id2sd(st->rid);
	const char *channelname = script_getstr(st, 2);
	const char *eventname = script_getstr(st, 3);
	struct channel_data *chan = channel->search(channelname, sd);

	if (!chan) {
		script_pushint(st, 0);
		return true;
	}

	for (i = 0; i < MAX_EVENTQUEUE; i++) {
		if (strcmp(chan->handlers[i], eventname) == 0) {
			chan->handlers[i][0] = '\0';
			script_pushint(st, 1);
			return true;
		}
	}

	script_pushint(st, 0);
	return true;
}

/** By Cydh
 * Display script message
 * showscript "<message>"{, <GID>};
 */
static BUILDIN(showscript)
{
	struct block_list *bl = NULL;
	const char *msg = script_getstr(st, 2);
	int id = 0, flag = AREA;

	if (script_hasdata(st, 3)) {
		id = script_getnum(st, 3);
		bl = map->id2bl(id);
	}
	else {
		bl = st->rid ? map->id2bl(st->rid) : map->id2bl(st->oid);
	}

	if (!bl) {
		ShowError("buildin_showscript: Script not attached. (id=%d, rid=%d, oid=%d)\n", id, st->rid, st->oid);
		return false;
	}

	if (script_hasdata(st, 4))
		if (script_getnum(st, 4) == SELF)
			flag = SELF;

	clif->ShowScript(bl, msg, flag);
	return true;
}

static BUILDIN(mergeitem)
{
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	clif->openmergeitem(sd->fd, sd);

	return true;
}

// getcalendartime(<day of month>, <day of week>{, <hour>{, <minute>}});
// Returns the UNIX Timestamp of next ocurrency of given time
static BUILDIN(getcalendartime)
{
	struct tm info = { 0 };
	int day_of_month = script_hasdata(st, 4) ? script_getnum(st, 4) : -1;
	int day_of_week = script_hasdata(st, 5) ? script_getnum(st, 5) : -1;
	int year = date_get_year();
	int month = date_get_month();
	int day = date_get_day();
	int cur_hour = date_get_hour();
	int cur_min = date_get_min();
	int hour = script_getnum(st, 2);
	int minute = script_getnum(st, 3);

	info.tm_sec = 0;
	info.tm_min = minute;
	info.tm_hour = hour;
	info.tm_mday = day;
	info.tm_mon = month - 1;
	info.tm_year = year - 1900;

	if (day_of_month > -1 && day_of_week > -1) {
		ShowError("script:getcalendartime: You must only specify a day_of_week or a day_of_month, not both\n");
		script_pushint(st, -1);
		return false;
	}
	if (day_of_month > -1 && (day_of_month < 1 || day_of_month > 31)) {
		ShowError("script:getcalendartime: Day of Month in invalid range. Must be between 1 and 31.\n");
		script_pushint(st, -1);
		return false;
	}
	if (day_of_week > -1 && (day_of_week < 0 || day_of_week > 6)) {
		ShowError("script:getcalendartime: Day of Week in invalid range. Must be between 0 and 6.\n");
		script_pushint(st, -1);
		return false;
	}
	if (hour > -1 && (hour > 23)) {
		ShowError("script:getcalendartime: Hour in invalid range. Must be between 0 and 23.\n");
		script_pushint(st, -1);
		return false;
	}
	if (minute > -1 && (minute > 59)) {
		ShowError("script:getcalendartime: Minute in invalid range. Must be between 0 and 59.\n");
		script_pushint(st, -1);
		return false;
	}
	if (hour == -1 || minute == -1) {
		ShowError("script:getcalendartime: Minutes and Hours are required\n");
		script_pushint(st, -1);
		return false;
	}

	if (day_of_month > -1) {
		if (day_of_month < day) { // Next Month
			info.tm_mon++;
		} else if (day_of_month == day) { // Today
			if (hour < cur_hour || (hour == cur_hour && minute < cur_min)) { // But past time, next month
				info.tm_mon++;
			}
		}

		// Loops until month has finding a month that has day_of_month
		do {
			time_t t;
			struct tm *lt;
			info.tm_mday = day_of_month;
			t = mktime(&info);
			lt = localtime(&t);
			info = *lt;
		} while (info.tm_mday != day_of_month);
	} else if (day_of_week > -1) {
		int cur_wday = date_get_dayofweek();

		if (day_of_week > cur_wday) { // This week
			info.tm_mday += (day_of_week - cur_wday);
		} else if (day_of_week == cur_wday) { // Today
			if (hour < cur_hour || (hour == cur_hour && minute <= cur_min)) {
				info.tm_mday += 7; // Next week
			}
		} else if (day_of_week < cur_wday) { // Next week
			info.tm_mday += (7 - cur_wday + day_of_week);
		}
	} else if (day_of_week == -1 && day_of_month == -1) { // Next occurence of hour/min
		if (hour < cur_hour || (hour == cur_hour && minute < cur_min)) {
			info.tm_mday++;
		}
	}

	script_pushint(st, mktime(&info));

	return true;
}

enum consolemes_type {
	CONSOLEMES_DEBUG = 0,
	CONSOLEMES_ERROR = 1,
	CONSOLEMES_WARNING = 2,
	CONSOLEMES_INFO = 3,
	CONSOLEMES_STATUS = 4,
	CONSOLEMES_NOTICE = 5,
};

/*==========================================
* consolemes(<type>, "text")
*------------------------------------------*/
static BUILDIN(consolemes)
{
	struct StringBuf buf;
	StrBuf->Init(&buf);
	int type = script_hasdata(st, 2) ? script_getnum(st, 2) : 0;

	if (!script->sprintf_helper(st, 3, &buf)) {
		StrBuf->Destroy(&buf);
		script_pushint(st, 0);
		return false;
	}

	switch (type) {
	default:
	case CONSOLEMES_DEBUG:
		ShowDebug("consolemes: %s\n", StrBuf->Value(&buf));
		break;
	case CONSOLEMES_ERROR:
		ShowError("consolemes: (st->rid: %d) (st->oid: %d) %s\n", st->rid, st->oid, StrBuf->Value(&buf));
		break;
	case CONSOLEMES_WARNING:
		ShowWarning("consolemes: (st->rid: %d) (st->oid: %d) %s\n", st->rid, st->oid, StrBuf->Value(&buf));
		break;
	case CONSOLEMES_INFO:
		ShowInfo("consolemes: %s\n", StrBuf->Value(&buf));
		break;
	case CONSOLEMES_STATUS:
		ShowStatus("consolemes: %s\n", StrBuf->Value(&buf));
		break;
	case CONSOLEMES_NOTICE:
		ShowNotice("consolemes: %s\n", StrBuf->Value(&buf));
		break;
	}

	StrBuf->Destroy(&buf);
	script_pushint(st, 1);
	return true;
}

static BUILDIN(setfavoriteitemidx)
{
	struct map_session_data *sd = script_rid2sd(st);
	int idx = script_getnum(st, 2);
	int value = script_getnum(st, 3);

	if (sd == NULL) {
		ShowError("buildin_setfavoriteitemidx: No player attached.\n");
		return false;
	}

	if (idx < 0 || idx >= sd->status.inventorySize) {
		ShowError("buildin_setfavoriteitemidx: Invalid inventory index %d (min: %d, max: %d).\n", idx, 0, (sd->status.inventorySize - 1));
		return false;
	} else if (sd->inventory_data[idx] == NULL || sd->inventory_data[idx]->nameid <= 0) {
		ShowWarning("buildin_setfavoriteitemidx: Current inventory index %d has no data.\n", idx);
		return false;
	} else if (sd->status.inventory[idx].equip > 0) {
		ShowWarning("buildin_setfavoriteitemidx: Cant change favorite flag of an equipped item.\n");
		return false;
	} else {
		sd->status.inventory[idx].favorite = cap_value(value, 0, 1);
		clif->favorite_item(sd, idx);
	}

	return true;
}

static BUILDIN(autofavoriteitem)
{
	int nameid = script_getnum(st, 2);
	int flag = script_getnum(st, 3);
	struct item_data *item_data;

	if ((item_data = itemdb->exists(nameid)) == NULL) {
		ShowError("buildin_autofavoriteitem: Invalid item '%d'.\n", nameid);
		return false;
	}

	item_data->flag.auto_favorite = cap_value(flag, 0, 1);
	return true;
}

/** place holder for the translation macro **/
static BUILDIN(_)
{
	return true;
}

// declarations that were supposed to be exported from npc_chat.c
BUILDIN(defpattern);
BUILDIN(activatepset);
BUILDIN(deactivatepset);
BUILDIN(deletepset);

enum dressroom_mode {
	DRESSROOM_CLOSE = 0,
	DRESSROOM_OPEN  = 1
};

/**
 * dressroom({<enum dressroom_mode>});
 */
static BUILDIN(dressroom)
{
#if PACKETVER >= 20150513
	struct map_session_data *sd = script->rid2sd(st);
	enum dressroom_mode mode = DRESSROOM_OPEN;

	if (sd == NULL) {
		return false;
	}

	if (script_hasdata(st, 2)) {
		mode = script_getnum(st, 2);
	}

	switch (mode) {
	case DRESSROOM_OPEN:
		clif->dressroom_open(sd, 1);
		break;
	case DRESSROOM_CLOSE:
		clif->dressroom_open(sd, 0);
		break;
	default:
		ShowWarning("script:dressroom: unknown mode (%u).\n", mode);
		script_pushint(st, 0);
		return false;
	}

	script_pushint(st, 1);
	return true;
#else
	ShowError("The dressing room works only with packet version >= 20150513");
	script_pushint(st, 0);
	return false;
#endif
}

static BUILDIN(pcre_match)
{
	const char *input = script_getstr(st, 2);
	const char *regex = script_getstr(st, 3);

	script->op_2str(st, C_RE_EQ, input, regex);
	return true;
}

/**
 * navigateto("<map>"{,<x>,<y>,<flag>,<hide_window>,<monster_id>,<char_id>});
 */
static BUILDIN(navigateto)
{
#if PACKETVER >= 20111010
	struct map_session_data* sd;
	const char *mapname;
	uint16 x = 0;
	uint16 y = 0;
	uint16 monster_id = 0;
	uint8 flag = NAV_KAFRA_AND_AIRSHIP;
	bool hideWindow = true;

	mapname = script_getstr(st, 2);

	if (script_hasdata(st, 3))
		x = script_getnum(st, 3);
	if (script_hasdata(st, 4))
		y = script_getnum(st, 4);
	if (script_hasdata(st, 5))
		flag = (uint8)script_getnum(st, 5);
	if (script_hasdata(st, 6))
		hideWindow = script_getnum(st, 6) ? true : false;
	if (script_hasdata(st, 7))
		monster_id = script_getnum(st, 7);

	if (script_hasdata(st, 8)) {
		sd = map->charid2sd(script_getnum(st, 8));
	} else {
		sd = script->rid2sd(st);
	}

	clif->navigate_to(sd, mapname, x, y, flag, hideWindow, monster_id);

	return true;
#else
	ShowError("Navigation system works only with packet version >= 20111010");
	return false;
#endif
}

static bool rodex_sendmail_sub(struct script_state *st, struct rodex_message *msg)
{
	const char *sender_name, *title, *body;

	if (strcmp(script->getfuncname(st), "rodex_sendmail_acc") == 0 || strcmp(script->getfuncname(st), "rodex_sendmail_acc2") == 0)
		msg->receiver_accountid = script_getnum(st, 2);
	else
		msg->receiver_id = script_getnum(st, 2);

	sender_name = script_getstr(st, 3);
	if (strlen(sender_name) >= NAME_LENGTH) {
		ShowError("script:rodex_sendmail: Sender name must not be bigger than %d!\n", NAME_LENGTH - 1);
		return false;
	}
	safestrncpy(msg->sender_name, sender_name, NAME_LENGTH);

	title = script_getstr(st, 4);
	if (strlen(title) >= RODEX_TITLE_LENGTH) {
		ShowError("script:rodex_sendmail: Mail Title must not be bigger than %d!\n", RODEX_TITLE_LENGTH - 1);
		return false;
	}
	safestrncpy(msg->title, title, RODEX_TITLE_LENGTH);

	body = script_getstr(st, 5);
	if (strlen(body) >= MAIL_BODY_LENGTH) {
		ShowError("script:rodex_sendmail: Mail Message must not be bigger than %d!\n", RODEX_BODY_LENGTH - 1);
		return false;
	}
	safestrncpy(msg->body, body, MAIL_BODY_LENGTH);

	if (script_hasdata(st, 6)) {
		msg->zeny = script_getnum(st, 6);
		if (msg->zeny < 0 || msg->zeny > MAX_ZENY) {
			ShowError("script:rodex_sendmail: Invalid Zeny value %"PRId64"!\n", msg->zeny);
			return false;
		}
	}

	return true;
}

static BUILDIN(rodex_sendmail)
{
	struct rodex_message msg = { 0 };
	int item_count = 0, i = 0, param = 7;

	// Common parameters - sender/message/zeny
	if (rodex_sendmail_sub(st, &msg) == false)
		return false;

	// Item list
	while (i < RODEX_MAX_ITEM && script_hasdata(st, param)) {
		struct item_data *idata;

		if (!script_hasdata(st, param + 1)) {
			ShowError("script:rodex_sendmail: Missing Item %d amount!\n", (i + 1));
			return false;
		}

		++item_count;
		if (data_isstring(script_getdata(st, param)) == false) {
			int itemid = script_getnum(st, param);

			if (itemdb->exists(itemid) == false) {
				ShowError("script:rodex_sendmail: Unknown item ID %d.\n", itemid);
				return false;
			}

			idata = itemdb->search(itemid);
		}
		else {
			ShowError("script:rodex_sendmail: Item %d must be passed as Number.\n", (i + 1));
			return false;
		}

		msg.items[i].item.nameid = idata->nameid;
		msg.items[i].item.amount = script_getnum(st, (param + 1));
		msg.items[i].item.identify = 1;

		++i;
		param += 2;
	}
	msg.items_count = item_count;

	msg.type = MAIL_TYPE_NPC;
	if (msg.zeny > 0)
		msg.type |= MAIL_TYPE_ZENY;
	if (msg.items_count > 0)
		msg.type |= MAIL_TYPE_ITEM;
	msg.send_date = (int)time(NULL);
	msg.expire_date = (int)time(NULL) + RODEX_EXPIRE;

	intif->rodex_sendmail(&msg);

	return true;
}

static BUILDIN(rodex_sendmail2)
{
	struct rodex_message msg = { 0 };
	int item_count = 0, i = 0, param = 7;

	// Common parameters - sender/message/zeny
	if (rodex_sendmail_sub(st, &msg) == false)
		return false;

	// Item list
	while (i < RODEX_MAX_ITEM && script_hasdata(st, param)) {
		struct item_data *idata;
		int j;

		// Tests
		if (!script_hasdata(st, param + 1)) {
			ShowError("script:rodex_sendmail: Missing Item %d amount!\n", (i + 1));
			return false;
		}
		if (!script_hasdata(st, param + 2)) {
			ShowError("script:rodex_sendmail: Missing Item %d refine!\n", (i + 1));
			return false;
		}
		if (!script_hasdata(st, param + 3)) {
			ShowError("script:rodex_sendmail: Missing Item %d attribute!\n", (i + 1));
			return false;
		}
		for (j = 0; j < MAX_SLOTS; ++j) {
			if (!script_hasdata(st, param + 4 + j)) {
				ShowError("script:rodex_sendmail: Missing Item %d card %d!\n", (i + 1), j);
				return false;
			}
		}

		// Set data to message
		++item_count;
		if (data_isstring(script_getdata(st, param)) == false) {
			int itemid = script_getnum(st, param);

			if (itemdb->exists(itemid) == false) {
				ShowError("script:rodex_sendmail: Unknown item ID %d.\n", itemid);
				return false;
			}

			idata = itemdb->search(itemid);
		} else {
			ShowError("script:rodex_sendmail: Item %d must be passed as Number.\n", (i + 1));
			return false;
		}

		msg.items[i].item.nameid = idata->nameid;
		msg.items[i].item.amount = script_getnum(st, (param + 1));
		msg.items[i].item.refine = script_getnum(st, (param + 2));
		msg.items[i].item.attribute = script_getnum(st, (param + 3));
		msg.items[i].item.identify = 1;

		for (j = 0; j < MAX_SLOTS; ++j) {
			msg.items[i].item.card[j] = script_getnum(st, param + 4 + j);
		}

		++i;
		param += 4 + MAX_SLOTS;
	}
	msg.items_count = item_count;

	msg.type = MAIL_TYPE_NPC;
	if (msg.zeny > 0)
		msg.type |= MAIL_TYPE_ZENY;
	if (msg.items_count > 0)
		msg.type |= MAIL_TYPE_ITEM;
	msg.send_date = (int)time(NULL);
	msg.expire_date = (int)time(NULL) + RODEX_EXPIRE;

	intif->rodex_sendmail(&msg);

	return true;
}

/**
 * Clan System: Add a player to a clan
 */
static BUILDIN(clan_join)
{
	struct map_session_data *sd = NULL;
	int clan_id = script_getnum(st, 2);

	if (script_hasdata(st, 3))
		sd = map->id2sd(script_getnum(st, 3));
	else
		sd = map->id2sd(st->rid);

	if (sd == NULL) {
		script_pushint(st, false);
		return false;
	}

	if (clan->join(sd, clan_id))
		script_pushint(st, true);
	else
		script_pushint(st, false);

	return true;
}

/**
 * Clan System: Remove a player from clan
 */
static BUILDIN(clan_leave)
{
	struct map_session_data *sd = NULL;

	if (script_hasdata(st, 2))
		sd = map->id2sd(script_getnum(st, 2));
	else
		sd = map->id2sd(st->rid);

	if (sd == NULL) {
		script_pushint(st, false);
		return false;
	}

	if (clan->leave(sd, false))
		script_pushint(st, true);
	else
		script_pushint(st, false);

	return true;
}

/**
 * Clan System: Show clan emblem next to npc name
 */
static BUILDIN(clan_master)
{
	struct npc_data *nd = map->id2nd(st->oid);
	int clan_id = script_getnum(st, 2);

	if (nd == NULL) {
		script_pushint(st, false);
		return false;
	} else if (clan_id <= 0) {
		script_pushint(st, false);
		ShowError("buildin_clan_master: Received Invalid Clan ID %d\n", clan_id);
		return false;
	} else if (clan->search(clan_id) == NULL) {
		script_pushint(st, false);
		ShowError("buildin_clan_master: Received Id of a nonexistent Clan. Id: %d\n", clan_id);
		return false;
	}

	nd->clan_id = clan_id;
	clif->sc_load(&nd->bl, nd->bl.id, AREA, status->get_sc_icon(SC_CLAN_INFO), 0, clan_id, 0);

	script_pushint(st, true);
	return true;
}

static BUILDIN(airship_respond)
{
	struct map_session_data *sd = map->id2sd(st->rid);
	int32 flag = script_getnum(st, 2);

	if (sd == NULL)
		return false;

	if (flag < P_AIRSHIP_NONE || flag > P_AIRSHIP_ITEM_INVALID) {
		ShowWarning("buildin_airship_respond: invalid flag %d has been given.", flag);
		return false;
	}

	clif->PrivateAirshipResponse(sd, flag);
	return true;
}

/**
 * hateffect(EffectID, Enable_State)
 */
static BUILDIN(hateffect)
{
#if PACKETVER >= 20150422
	struct map_session_data *sd = script_rid2sd(st);
	int effectId, enabled = 0;
	int i;

	if (sd == NULL)
		return false;

	effectId = script_getnum(st, 2);
	enabled = script_getnum(st, 3);

	for (i = 0; i < VECTOR_LENGTH(sd->hatEffectId); ++i) {
		if (VECTOR_INDEX(sd->hatEffectId, i) == effectId) {
			if (enabled == 1) { // Already Enabled
				return true;
			} else { // Remove
				VECTOR_ERASE(sd->hatEffectId, i);
				clif->hat_effect_single(&sd->bl, effectId, enabled);
				return true;
			}
		}
	}

	VECTOR_ENSURE(sd->hatEffectId, 1, 1);
	VECTOR_PUSH(sd->hatEffectId, effectId);

	clif->hat_effect_single(&sd->bl, effectId, enabled);
#endif
	return true;
}

static BUILDIN(openstylist)
{
	struct map_session_data *sd = script_rid2sd(st);

	if (sd == NULL)
		return false;

#if PACKETVER >= 20150128
	clif->open_ui(sd, CZ_STYLIST_UI);
#endif
	return true;
}

static BUILDIN(msgtable)
{
	struct map_session_data *sd = script_rid2sd(st);
	if (sd == NULL)
		return false;

	const enum clif_messages msgId = script_getnum(st, 2);
	if (script_hasdata(st, 3)) {
		clif->msgtable_color(sd, msgId, script_getnum(st, 3));
	} else {
		clif->msgtable(sd, msgId);
	}

	return true;
}

static BUILDIN(msgtable2)
{
	struct map_session_data *sd = script_rid2sd(st);
	if (sd == NULL)
		return false;

	const enum clif_messages msgId = script_getnum(st, 2);
	if (script_isstringtype(st, 3)) {
		const char *value = script_getstr(st, 3);
		if (script_hasdata(st, 4)) {
			clif->msgtable_str_color(sd, msgId, value, script_getnum(st, 4));
		} else {
			clif->msgtable_str(sd, msgId, value);
		}
	} else {
		const int value = script_getnum(st, 3);
		clif->msgtable_num(sd, msgId, value);
	}

	return true;
}

// show/hide camera info
static BUILDIN(camerainfo)
{
	struct map_session_data *sd = script_rid2sd(st);
	if (sd == NULL)
		return false;

	clif->camera_showWindow(sd);
	return true;
}

// allow change some camera parameters
static BUILDIN(changecamera)
{
	struct map_session_data *sd = script_rid2sd(st);
	if (sd == NULL)
		return false;

	enum send_target target = SELF;
	if (script_hasdata(st, 5)) {
		target = script_getnum(st, 5);
	}
	clif->camera_change(sd, (float)script_getnum(st, 2), (float)script_getnum(st, 3), (float)script_getnum(st, 4), target);
	return true;
}

// update preview window to given item
static BUILDIN(itempreview)
{
	struct map_session_data *sd = script_rid2sd(st);
	if (sd == NULL)
		return false;
	clif->item_preview(sd, script_getnum(st, 2));
	return true;
}

// insert or remove card into equipped item
static BUILDIN(enchantitem)
{
	struct map_session_data *sd = script_rid2sd(st);
	if (sd == NULL)
		return false;
	const int pos = script_getnum(st, 2);
	if ((pos < EQI_ACC_L || pos > EQI_HAND_R) && pos != EQI_AMMO) {
		ShowError("Wrong equip position: %d\n", pos);
		script->reportfunc(st);
		script->reportsrc(st);
		script_pushint(st, false);
		return true;
	}
	const int cardId = script_getnum(st, 4);
	struct item_data *it = itemdb->exists(cardId);
	if (it == NULL || it->type != IT_CARD) {
		ShowError("Item id is not card or not exists: %d\n", cardId);
		script->reportfunc(st);
		script->reportsrc(st);
		script_pushint(st, false);
		return true;
	}
	const int n = sd->equip_index[pos];
	if (n < 0) {
		ShowError("Item in equipment slot %d is not equipped\n", pos);
		script->reportfunc(st);
		script->reportsrc(st);
		script_pushint(st, false);
		return true;
	}
	const int cardSlot = script_getnum(st, 3);
	if (cardSlot < 0 || cardSlot >= MAX_SLOTS) {
		ShowError("Wrong card slot %d. Must be in range 0-3.\n", cardSlot);
		script->reportfunc(st);
		script->reportsrc(st);
		script_pushint(st, false);
		return true;
	}
	const bool res = clif->enchant_equipment(sd, pc->equip_pos[pos], cardSlot, cardId);
	if (res) {
		logs->pick_pc(sd, LOG_TYPE_CARD, -1, &sd->status.inventory[n],sd->inventory_data[n]);
		sd->status.inventory[n].card[cardSlot] = cardId;
		logs->pick_pc(sd, LOG_TYPE_CARD,  1, &sd->status.inventory[n],sd->inventory_data[n]);
		status_calc_pc(sd, SCO_NONE);
	}
	script_pushint(st, res);
	return true;
}

// send ack to inventory expand request
static BUILDIN(expandinventoryack)
{
	struct map_session_data *sd = script_rid2sd(st);
	if (sd == NULL)
		return false;
	int itemId = 0;
	if (script_hasdata(st, 3)) {
		itemId = script_getnum(st, 3);
	}
	clif->inventoryExpandAck(sd, script_getnum(st, 2), itemId);
	return true;
}

// send final ack to inventory expand request
static BUILDIN(expandinventoryresult)
{
	struct map_session_data *sd = script_rid2sd(st);
	if (sd == NULL)
		return false;
	clif->inventoryExpandResult(sd, script_getnum(st, 2));
	return true;
}

// adjust player inventory size to given value positive or negative
static BUILDIN(expandinventory)
{
	struct map_session_data *sd = script_rid2sd(st);
	if (sd == NULL)
		return false;
	script_pushint(st, pc->expandInventory(sd, script_getnum(st, 2)));
	return true;
}

// return current player inventory size
static BUILDIN(getinventorysize)
{
	struct map_session_data *sd = script_rid2sd(st);
	if (sd == NULL)
		return false;
	script_pushint(st, sd->status.inventorySize);
	return true;
}

// force close roulette window if it opened
static BUILDIN(closeroulette)
{
	struct map_session_data *sd = script_rid2sd(st);
	if (sd == NULL)
		return false;
	clif->roulette_close(sd);
	return true;
}

static BUILDIN(openrefineryui)
{
	struct map_session_data *sd = script_rid2sd(st);

	if (sd == NULL) {
		script_pushint(st, 0);
		return true;
	}

	if (battle_config.enable_refinery_ui == 0) {
		script_pushint(st, 0);
		return true;
	}

	clif->OpenRefineryUI(sd);
	script_pushint(st, 1);
	return true;
}

/**
 * identify(<item id>)
 * Identifies the first unidentified <item id> item on player's inventory.
 * Returns -2 on error, -1 if no item to identify was found, identified idx otherwise.
 */
static BUILDIN(identify)
{
	struct map_session_data *sd = script_rid2sd(st);

	if (sd == NULL) {
		script_pushint(st, -2);
		return true;
	}

	int itemid = script_getnum(st, 2);
	if (itemdb->exists(itemid) == NULL) {
		ShowError("buildin_identify: Invalid item ID (%d)\n", itemid);
		script_pushint(st, -2);
		return true;
	}

	int idx = -1;
	ARR_FIND(0, sd->status.inventorySize, idx, (sd->status.inventory[idx].nameid == itemid && sd->status.inventory[idx].identify == 0));

	if (idx < 0 || idx >= sd->status.inventorySize) {
		script_pushint(st, -1);
		return true;
	}

	sd->status.inventory[idx].identify = 1;
	clif->item_identified(sd, idx, 0);
	script_pushint(st, idx);

	return true;
}

/**
 * identifyidx(idx)
 * Identifies item at idx.
 * Returns true if item is identified, false otherwise.
 */
static BUILDIN(identifyidx)
{
	struct map_session_data *sd = script_rid2sd(st);

	if (sd == NULL) {
		script_pushint(st, false);
		return true;
	}

	int idx = script_getnum(st, 2);
	if (idx < 0 || idx >= sd->status.inventorySize) {
		ShowError("buildin_identifyidx: Invalid inventory index (%d), expected a value between 0 and %d\n", idx, sd->status.inventorySize);
		script_pushint(st, false);
		return true;
	}

	if (sd->status.inventory[idx].nameid <= 0 || sd->status.inventory[idx].identify != 0) {
		script_pushint(st, false);
		return true;
	}

	sd->status.inventory[idx].identify = 1;
	clif->item_identified(sd, idx, 0);
	script_pushint(st, true);

	return true;
}

static BUILDIN(openlapineddukddakboxui)
{
	struct map_session_data *sd = script_rid2sd(st);
	if (sd == NULL)
		return false;
	const int item_id = script_getnum(st, 2);
	struct item_data *it = itemdb->exists(item_id);
	if (it == NULL) {
		ShowError("buildin_openlapineddukddakboxui: Item %d is not valid\n", item_id);
		script->reportfunc(st);
		script->reportsrc(st);
		script_pushint(st, false);
		return true;
	}
	clif->lapineDdukDdak_open(sd, item_id);
	script_pushint(st, true);
	return true;
}

// Reset 'Feeling' maps.
BUILDIN(resetfeel)
{
	struct map_session_data *sd;

	if (script_hasdata(st, 2))
		sd = script->id2sd(st, script_getnum(st, 2));
	else
		sd = script->rid2sd(st);

	if (sd != NULL)
		pc->resetfeel(sd);

	return true;
}

// Reset hatred target marks.
BUILDIN(resethate)
{
	struct map_session_data *sd;

	if (script_hasdata(st, 2))
		sd = script->id2sd(st, script_getnum(st, 2));
	else
		sd = script->rid2sd(st);

	if (sd != NULL)
		pc->resethate(sd);

	return true;
}

/**
 * Adds a built-in script function.
 *
 * @param buildin Script function data
 * @param force   Whether to override an existing function with the same name
 *                (i.e. a plugin overriding a built-in function)
 * @return Whether the function was successfully added.
 */
static bool script_add_builtin(const struct script_function *buildin, bool override)
{
	int n = 0, offset = 0;
	size_t slen;
	if( !buildin ) {
		return false;
	}
	if( buildin->arg ) {
		// arg must follow the pattern: (v|s|i|r|l)*\?*\*?
		// 'v' - value (either string or int or reference)
		// 's' - string
		// 'i' - int
		// 'r' - reference (of a variable)
		// 'l' - label
		// '?' - one optional parameter
		// '*' - unknown number of optional parameters
		char *p = buildin->arg;
		while( *p == 'v' || *p == 's' || *p == 'i' || *p == 'r' || *p == 'l' ) ++p;
		while( *p == '?' ) ++p;
		if( *p == '*' ) ++p;
		if( *p != 0 ) {
			ShowWarning("add_builtin: ignoring function \"%s\" with invalid arg \"%s\".\n", buildin->name, buildin->arg);
			return false;
		}
	}
	if( !buildin->name || *script->skip_word(buildin->name) != 0 ) {
		ShowWarning("add_builtin: ignoring function with invalid name \"%s\" (must be a word).\n", buildin->name);
		return false;
	}
	if ( !buildin->func ) {
		ShowWarning("add_builtin: ignoring function \"%s\" with invalid source function.\n", buildin->name);
		return false;
	}
	slen = buildin->arg ? strlen(buildin->arg) : 0;
	n = script->add_str(buildin->name);

	if( !override && script->str_data[n].func && script->str_data[n].func != buildin->func ) {
		return false; /* something replaced it, skip. */
	}

	if( override && script->str_data[n].type == C_FUNC ) {
		// Overriding
		offset = script->str_data[n].val;
		if( script->buildin[offset] )
			aFree(script->buildin[offset]);
		script->buildin[offset] = NULL;
	} else {
		// Adding new function
		if( strcmp(buildin->name, "__setr") == 0 ) script->buildin_set_ref = n;
		else if( strcmp(buildin->name, "callsub") == 0 ) script->buildin_callsub_ref = n;
		else if( strcmp(buildin->name, "callfunc") == 0 ) script->buildin_callfunc_ref = n;
		else if( strcmp(buildin->name, "getelementofarray") == 0 ) script->buildin_getelementofarray_ref = n;
		else if( strcmp(buildin->name, "mes") == 0 ) script->buildin_mes_offset = script->buildin_count;
		else if( strcmp(buildin->name, "mesf") == 0 ) script->buildin_mesf_offset = script->buildin_count;
		else if( strcmp(buildin->name, "select") == 0 ) script->buildin_select_offset = script->buildin_count;
		else if( strcmp(buildin->name, "_") == 0 ) script->buildin_lang_macro_offset = script->buildin_count;
		else if( strcmp(buildin->name, "_$") == 0 ) script->buildin_lang_macro_fmtstring_offset = script->buildin_count;

		offset = script->buildin_count;

		script->str_data[n].type = C_FUNC;
		script->str_data[n].val = offset;

		// Note: This is a no-op if script->buildin is already large enough
		//   (it'll only have effect when a plugin adds a new command)
		RECREATE(script->buildin, char *, ++script->buildin_count);
	}

	script->str_data[n].func = buildin->func;
	script->str_data[n].deprecated = (buildin->deprecated ? 1 : 0);

	/* we only store the arguments, its the only thing used out of this */
	if( slen ) {
		CREATE(script->buildin[offset], char, slen + 1);
		safestrncpy(script->buildin[offset], buildin->arg, slen + 1);
	} else {
		script->buildin[offset] = NULL;
	}

	return true;
}

static bool script_hp_add(char *name, char *args, bool (*func)(struct script_state *st), bool isDeprecated)
{
	struct script_function buildin;
	buildin.name = name;
	buildin.arg = args;
	buildin.func = func;
	buildin.deprecated = isDeprecated;
	return script->add_builtin(&buildin, true);
}

static void script_run_use_script(struct map_session_data *sd, struct item_data *data, int oid) __attribute__((nonnull (1)));

/**
 * Run use script for item.
 *
 * @param sd    player session data. Must be correct and checked before.
 * @param n     item index in inventory. Must be correct and checked before.
 * @param oid   npc id. Can be also 0 or fake npc id.
 */
static void script_run_use_script(struct map_session_data *sd, struct item_data *data, int oid)
{
	nullpo_retv(data);
	script->current_item_id = data->nameid;
	script->run(data->script, 0, sd->bl.id, oid);
	script->current_item_id = 0;
}

static void script_run_item_equip_script(struct map_session_data *sd, struct item_data *data, int oid) __attribute__((nonnull (1, 2)));

/**
 * Run item equip script for item.
 *
 * @param sd    player session data. Must be correct and checked before.
 * @param data  equipped item data. Must be correct and checked before.
 * @param oid   npc id. Can be also 0 or fake npc id.
 */
static void script_run_item_equip_script(struct map_session_data *sd, struct item_data *data, int oid)
{
	script->current_item_id = data->nameid;
	script->run(data->equip_script, 0, sd->bl.id, oid);
	script->current_item_id = 0;
}

static void script_run_item_unequip_script(struct map_session_data *sd, struct item_data *data, int oid) __attribute__((nonnull (1, 2)));

/**
 * Run item unequip script for item.
 *
 * @param sd    player session data. Must be correct and checked before.
 * @param data  unequipped item data. Must be correct and checked before.
 * @param oid   npc id. Can be also 0 or fake npc id.
 */
static void script_run_item_unequip_script(struct map_session_data *sd, struct item_data *data, int oid)
{
	script->current_item_id = data->nameid;
	script->run(data->unequip_script, 0, sd->bl.id, oid);
	script->current_item_id = 0;
}

static void script_run_item_rental_start_script(struct map_session_data *sd, struct item_data *data, int oid) __attribute__((nonnull(1, 2)));

/**
 * Run item rental start script
 * @param sd    player session data. Must be correct and checked before.
 * @param data  rental item data. Must be correct and checked before.
 * @param oid   npc id. Can be also 0 or fake npc id.
 **/
static void script_run_item_rental_start_script(struct map_session_data *sd, struct item_data *data, int oid)
{
	script->current_item_id = data->nameid;
	script->run(data->rental_start_script, 0, sd->bl.id, oid);
	script->current_item_id = 0;
}

static void script_run_item_rental_end_script(struct map_session_data *sd, struct item_data *data, int oid) __attribute__((nonnull(1, 2)));

/**
* Run item rental end script
* @param sd    player session data. Must be correct and checked before.
* @param data  rental item data. Must be correct and checked before.
* @param oid   npc id. Can be also 0 or fake npc id.
**/
static void script_run_item_rental_end_script(struct map_session_data *sd, struct item_data *data, int oid)
{
	script->current_item_id = data->nameid;
	script->run(data->rental_end_script, 0, sd->bl.id, oid);
	script->current_item_id = 0;
}

static void script_run_item_lapineddukddak_script(struct map_session_data *sd, struct item_data *data, int oid) __attribute__((nonnull (1, 2)));

/**
 * Run item lapineddukddak script for item.
 *
 * @param sd    player session data. Must be correct and checked before.
 * @param data  unequipped item data. Must be correct and checked before.
 * @param oid   npc id. Can be also 0 or fake npc id.
 */
static void script_run_item_lapineddukddak_script(struct map_session_data *sd, struct item_data *data, int oid)
{
	script->current_item_id = data->nameid;
	script->run(data->lapineddukddak->script, 0, sd->bl.id, oid);
	script->current_item_id = 0;
}

#define BUILDIN_DEF(x,args) { buildin_ ## x , #x , args, false }
#define BUILDIN_DEF2(x,x2,args) { buildin_ ## x , x2 , args, false }
#define BUILDIN_DEF_DEPRECATED(x,args) { buildin_ ## x , #x , args, true }
#define BUILDIN_DEF2_DEPRECATED(x,x2,args) { buildin_ ## x , x2 , args, true }
static void script_parse_builtin(void)
{
	struct script_function BUILDIN[] = {
		/* Commands for internal use by the script engine */
		BUILDIN_DEF(__jump_zero,"il"),
		BUILDIN_DEF(__setr,"rv?"),

		// NPC interaction
		BUILDIN_DEF(mes, "?"),
		BUILDIN_DEF(mesf, "s*"),
		BUILDIN_DEF(next,""),
		BUILDIN_DEF(mesclear,""),
		BUILDIN_DEF(close,""),
		BUILDIN_DEF(close2,""),
		BUILDIN_DEF(menu,"sl*"),
		BUILDIN_DEF(select,"s*"), //for future jA script compatibility
		BUILDIN_DEF2(select, "prompt", "s*"),
		//
		BUILDIN_DEF(goto,"l"),
		BUILDIN_DEF(callsub,"l*"),
		BUILDIN_DEF(callfunc,"s*"),
		BUILDIN_DEF(return,"?"),
		BUILDIN_DEF(getarg,"i?"),
		BUILDIN_DEF(jobchange,"i?"),
		BUILDIN_DEF(jobname,"i"),
		BUILDIN_DEF(input,"r??"),
		BUILDIN_DEF(warp,"sii?"),
		BUILDIN_DEF(areawarp,"siiiisii??"),
		BUILDIN_DEF(warpchar,"siii"), // [LuzZza]
		BUILDIN_DEF(warpparty,"siii???"), // [Fredzilla] [Paradox924X] [Jedzkie] [Dastgir]
		BUILDIN_DEF(warpguild,"siii??"), // [Fredzilla]
		BUILDIN_DEF(setlook,"ii"),
		BUILDIN_DEF(changelook,"ii"), // Simulates but don't Store it
		BUILDIN_DEF2(__setr,"set","rv"),
		BUILDIN_DEF(setarray,"rv*"),
		BUILDIN_DEF(cleararray,"rvi"),
		BUILDIN_DEF(copyarray,"rri"),
		BUILDIN_DEF(getarraysize,"r"),
		BUILDIN_DEF(getarrayindex,"r"),
		BUILDIN_DEF(deletearray,"r?"),
		BUILDIN_DEF(getelementofarray,"ri"),
		BUILDIN_DEF(getitem,"vi?"),
		BUILDIN_DEF(rentitem,"vi"),
		BUILDIN_DEF(getitem2,"viiiiiiii?"),
		BUILDIN_DEF(getnameditem,"vv"),
		BUILDIN_DEF2(grouprandomitem,"groupranditem","i"),
		BUILDIN_DEF(makeitem,"visii"),
		BUILDIN_DEF(makeitem2,"viiiiiiii????"),
		BUILDIN_DEF(delitem,"vi?"),
		BUILDIN_DEF(delitem2,"viiiiiiii?"),
		BUILDIN_DEF(delitemidx, "i??"),
		BUILDIN_DEF2(enableitemuse,"enable_items",""),
		BUILDIN_DEF2(disableitemuse,"disable_items",""),
		BUILDIN_DEF(cutin,"si"),
		BUILDIN_DEF(viewpoint,"iiiii"),
		BUILDIN_DEF(heal,"ii"),
		BUILDIN_DEF(itemheal,"ii"),
		BUILDIN_DEF(percentheal,"ii"),
		BUILDIN_DEF(rand,"i?"),
		BUILDIN_DEF(countitem,"v"),
		BUILDIN_DEF(countitem2,"viiiiiii"),
		BUILDIN_DEF(countnameditem,"v?"),
		BUILDIN_DEF(checkweight,"vi*"),
		BUILDIN_DEF(checkweight2,"rr"),
		BUILDIN_DEF(readparam,"i?"),
		BUILDIN_DEF(setparam,"ii?"),
		BUILDIN_DEF(getcharid,"i?"),
		BUILDIN_DEF(getnpcid, "?"),
		BUILDIN_DEF(getpartyname,"i"),
		BUILDIN_DEF(getpartymember,"i?"),
		BUILDIN_DEF(getpartyleader,"i?"),
		BUILDIN_DEF_DEPRECATED(getguildname,"i"),
		BUILDIN_DEF_DEPRECATED(getguildmaster,"i"),
		BUILDIN_DEF_DEPRECATED(getguildmasterid,"i"),
		BUILDIN_DEF(getguildmember,"i?"),
		BUILDIN_DEF(getguildinfo,"i?"),
		BUILDIN_DEF(getguildonline, "i?"),
		BUILDIN_DEF(strcharinfo,"i??"),
		BUILDIN_DEF(strnpcinfo,"i??"),
		BUILDIN_DEF(charid2rid,"i"),
		BUILDIN_DEF(getequipid,"i"),
		BUILDIN_DEF(getequipname,"i"),
		BUILDIN_DEF(getbrokenid,"i"), // [Valaris]
		BUILDIN_DEF(getbrokencount,""),
		BUILDIN_DEF(repair,"i"), // [Valaris]
		BUILDIN_DEF(repairall,""),
		BUILDIN_DEF(getequipisequiped,"i"),
		BUILDIN_DEF(getequipisenableref,"i"),
		BUILDIN_DEF(getequipisidentify,"i"),
		BUILDIN_DEF(getequiprefinerycnt,"i*"),
		BUILDIN_DEF(getequipweaponlv,"i"),
		BUILDIN_DEF(getequippercentrefinery,"i?"),
		BUILDIN_DEF(successrefitem,"i?"),
		BUILDIN_DEF(failedrefitem,"i"),
		BUILDIN_DEF(downrefitem,"i?"),
		BUILDIN_DEF(statusup,"i"),
		BUILDIN_DEF(statusup2,"ii"),
		BUILDIN_DEF(needed_status_point, "ii"),
		BUILDIN_DEF(bonus,"iv"),
		BUILDIN_DEF2(bonus,"bonus2","ivi"),
		BUILDIN_DEF2(bonus,"bonus3","ivii"),
		BUILDIN_DEF2(bonus,"bonus4","ivvii"),
		BUILDIN_DEF2(bonus,"bonus5","ivviii"),
		BUILDIN_DEF(autobonus,"sii??"),
		BUILDIN_DEF(autobonus2,"sii??"),
		BUILDIN_DEF(autobonus3,"siiv?"),
		BUILDIN_DEF(skill,"vi?"),
		BUILDIN_DEF(addtoskill,"vi?"), // [Valaris]
		BUILDIN_DEF(guildskill,"vi"),
		BUILDIN_DEF(getskilllv,"v"),
		BUILDIN_DEF(getgdskilllv,"iv"),
		BUILDIN_DEF(basicskillcheck,""),
		BUILDIN_DEF(getgmlevel,""),
		BUILDIN_DEF(setgroupid, "i?"),
		BUILDIN_DEF(getgroupid,"?"),
		BUILDIN_DEF(end,""),
		BUILDIN_DEF(checkoption,"i?"),
		BUILDIN_DEF(setoption,"i??"),
		BUILDIN_DEF(setcart,"?"),
		BUILDIN_DEF(checkcart,""),
		BUILDIN_DEF(setfalcon,"?"),
		BUILDIN_DEF(checkfalcon,""),
		BUILDIN_DEF(setmount,"??"),
		BUILDIN_DEF(checkmount,""),
		BUILDIN_DEF(checkwug,""),
		BUILDIN_DEF(savepoint,"sii"),
		BUILDIN_DEF(gettimetick,"i"),
		BUILDIN_DEF(gettime,"i"),
		BUILDIN_DEF(gettimestr, "si?"),
		BUILDIN_DEF(openstorage,""),
		BUILDIN_DEF(guildopenstorage,""),
		BUILDIN_DEF(itemskill,"vi?"),
		BUILDIN_DEF(produce,"i"),
		BUILDIN_DEF(cooking,"i"),
		BUILDIN_DEF(monster,"siisii???"),
		BUILDIN_DEF(getmobdrops,"i"),
		BUILDIN_DEF(areamonster,"siiiisii???"),
		BUILDIN_DEF(killmonster,"ss?"),
		BUILDIN_DEF(killmonsterall,"s?"),
		BUILDIN_DEF(killmonstergid, "i"),
		BUILDIN_DEF(clone,"siisi????"),
		BUILDIN_DEF(doevent,"s"),
		BUILDIN_DEF(donpcevent,"s"),
		BUILDIN_DEF(addtimer,"is?"),
		BUILDIN_DEF(deltimer,"s?"),
		BUILDIN_DEF(addtimercount,"si?"),
		BUILDIN_DEF(gettimer,"i??"),
		BUILDIN_DEF(getunits,"iri?????"),
		BUILDIN_DEF(initnpctimer,"??"),
		BUILDIN_DEF(stopnpctimer,"??"),
		BUILDIN_DEF(startnpctimer,"??"),
		BUILDIN_DEF(setnpctimer,"i?"),
		BUILDIN_DEF(getnpctimer,"i?"),
		BUILDIN_DEF(attachnpctimer,"?"), // attached the player id to the npc timer [Celest]
		BUILDIN_DEF(detachnpctimer,"?"), // detached the player id from the npc timer [Celest]
		BUILDIN_DEF(playerattached,""), // returns id of the current attached player. [Skotlex]
		BUILDIN_DEF(mobattached, ""),
		BUILDIN_DEF(announce,"si?????"),
		BUILDIN_DEF(mapannounce,"ssi?????"),
		BUILDIN_DEF(areaannounce,"siiiisi?????"),
		BUILDIN_DEF(getusers,"i"),
		BUILDIN_DEF(getmapguildusers,"si"),
		BUILDIN_DEF(getmapusers,"s"),
		BUILDIN_DEF(getareausers,"*"),
		BUILDIN_DEF(getareadropitem,"siiiiv"),
		BUILDIN_DEF(enablenpc,"s"),
		BUILDIN_DEF(disablenpc,"s"),
		BUILDIN_DEF(hideoffnpc,"s"),
		BUILDIN_DEF(hideonnpc,"s"),
		BUILDIN_DEF(cloakonnpc,"s?"),
		BUILDIN_DEF(cloakoffnpc,"s?"),
		BUILDIN_DEF(sc_start,"iii???"),
		BUILDIN_DEF2(sc_start,"sc_start2","iiii???"),
		BUILDIN_DEF2(sc_start,"sc_start4","iiiiii???"),
		BUILDIN_DEF(sc_end,"i?"),
		BUILDIN_DEF(getstatus, "i?"),
		BUILDIN_DEF(getscrate,"ii?"),
		BUILDIN_DEF_DEPRECATED(debugmes,"v*"),
		BUILDIN_DEF(consolemes,"iv*"),
		BUILDIN_DEF2(catchpet,"pet","i"),
		BUILDIN_DEF2(birthpet,"bpet",""),
		BUILDIN_DEF(resetlvl,"i"),
		BUILDIN_DEF(resetstatus,""),
		BUILDIN_DEF(resetskill,""),
		BUILDIN_DEF(resetfeel, "?"),
		BUILDIN_DEF(resethate, "?"),
		BUILDIN_DEF(skillpointcount,""),
		BUILDIN_DEF(changebase,"i?"),
		BUILDIN_DEF(changesex,""),
		BUILDIN_DEF(changecharsex,""), // [4144]
		BUILDIN_DEF(waitingroom,"si?????"),
		BUILDIN_DEF(delwaitingroom,"?"),
		BUILDIN_DEF2(waitingroomkickall,"kickwaitingroomall","?"),
		BUILDIN_DEF(enablewaitingroomevent,"?"),
		BUILDIN_DEF(disablewaitingroomevent,"?"),
		BUILDIN_DEF(getwaitingroomstate,"i?"),
		BUILDIN_DEF(warpwaitingpc,"sii?"),
		BUILDIN_DEF(attachrid,"i"),
		BUILDIN_DEF(detachrid,""),
		BUILDIN_DEF(isloggedin,"i?"),
		BUILDIN_DEF(setmapflagnosave,"ssii"),
		BUILDIN_DEF(getmapflag,"si"),
		BUILDIN_DEF(getmapinfo,"i?"),
		BUILDIN_DEF(setmapflag,"si?"),
		BUILDIN_DEF(removemapflag,"si"),
		BUILDIN_DEF(pvpon,"s"),
		BUILDIN_DEF(pvpoff,"s"),
		BUILDIN_DEF(gvgon,"s"),
		BUILDIN_DEF(gvgoff,"s"),
		BUILDIN_DEF(emotion,"i??"),
		BUILDIN_DEF(maprespawnguildid,"sii"),
		BUILDIN_DEF(agitstart,""), // <Agit>
		BUILDIN_DEF(agitend,""),
		BUILDIN_DEF(agitcheck,""),   // <Agitcheck>
		BUILDIN_DEF(flagemblem,"i"), // Flag Emblem
		BUILDIN_DEF(getcastlename,"s"),
		BUILDIN_DEF(getcastledata,"si"),
		BUILDIN_DEF(setcastledata,"sii"),
		BUILDIN_DEF(requestguildinfo,"i?"),
		BUILDIN_DEF(getequipcardcnt,"i"),
		BUILDIN_DEF(successremovecards,"i"),
		BUILDIN_DEF(failedremovecards,"ii"),
		BUILDIN_DEF(marriage,"s"),
		BUILDIN_DEF2(wedding_effect,"wedding",""),
		BUILDIN_DEF(divorce,""),
		BUILDIN_DEF(ispartneron,""),
		BUILDIN_DEF(getpartnerid,""),
		BUILDIN_DEF(getchildid,""),
		BUILDIN_DEF(getmotherid,""),
		BUILDIN_DEF(getfatherid,""),
		BUILDIN_DEF(warppartner,"sii"),
		BUILDIN_DEF(getitemname,"v"),
		BUILDIN_DEF(getitemslots,"i"),
		BUILDIN_DEF(makepet,"i"),
		BUILDIN_DEF(getexp,"ii"),
		BUILDIN_DEF(getinventorylist,""),
		BUILDIN_DEF(getcartinventorylist,""),
		BUILDIN_DEF(getskilllist,""),
		BUILDIN_DEF(clearitem,""),
		BUILDIN_DEF(classchange,"ii?"),
		BUILDIN_DEF_DEPRECATED(misceffect,"i"),
		BUILDIN_DEF(playbgm,"s"),
		BUILDIN_DEF(playbgmall,"s?????"),
		BUILDIN_DEF(soundeffect,"si"),
		BUILDIN_DEF(soundeffectall,"si?????"), // SoundEffectAll [Codemaster]
		BUILDIN_DEF(strmobinfo,"ii"), // display mob data [Valaris]
		BUILDIN_DEF(guardian,"siisi??"), // summon guardians
		BUILDIN_DEF(guardianinfo,"sii"), // display guardian data [Valaris]
		BUILDIN_DEF(petskillbonus,"iiii"), // [Valaris]
		BUILDIN_DEF(petrecovery,"ii"), // [Valaris]
		BUILDIN_DEF(petloot,"i"), // [Valaris]
		BUILDIN_DEF(petskillattack,"viiii"), // [Skotlex]
		BUILDIN_DEF(petskillsupport,"viiii"), // [Skotlex]
		BUILDIN_DEF(skilleffect,"vi"), // skill effect [Celest]
		BUILDIN_DEF(npcskilleffect,"viii"), // npc skill effect [Valaris]
		BUILDIN_DEF(specialeffect,"i???"), // npc skill effect [Valaris]
		BUILDIN_DEF(specialeffectnum,"iii???"), // npc skill effect with num [4144]
		BUILDIN_DEF(removespecialeffect,"i???"),
		BUILDIN_DEF_DEPRECATED(specialeffect2,"i??"), // skill effect on players[Valaris]
		BUILDIN_DEF(nude,""), // nude command [Valaris]
		BUILDIN_DEF(mapwarp,"ssii??"), // Added by RoVeRT
		BUILDIN_DEF(atcommand,"s"), // [MouseJstr]
		BUILDIN_DEF2(atcommand,"charcommand","s"), // [MouseJstr]
		BUILDIN_DEF(movenpc,"sii?"), // [MouseJstr]
		BUILDIN_DEF(message,"vs"), // [MouseJstr]
		BUILDIN_DEF(servicemessage, "si?"),
		BUILDIN_DEF(npctalk,"s??"), // [Valaris][Murilo BiO]
		BUILDIN_DEF(mobcount,"ss"),
		BUILDIN_DEF(getlook,"i"),
		BUILDIN_DEF(getsavepoint,"i"),
		BUILDIN_DEF(npcspeed,"i"), // [Valaris]
		BUILDIN_DEF(npcwalkto,"ii"), // [Valaris]
		BUILDIN_DEF(npcstop,""), // [Valaris]
		BUILDIN_DEF(setnpcdistance,"i"), // [4144]
		BUILDIN_DEF(getnpcdir,"?"), // [4144]
		BUILDIN_DEF(setnpcdir,"*"), // [4144]
		BUILDIN_DEF(getnpcclass,"?"), // [4144]
		BUILDIN_DEF(getmapxy,"rrri?"), //by Lorky [Lupus]
		BUILDIN_DEF(checkoption1,"i?"),
		BUILDIN_DEF(checkoption2,"i?"),
		BUILDIN_DEF(guildgetexp,"i"),
		BUILDIN_DEF(guildchangegm,"is"),
		BUILDIN_DEF(logmes,"s?"), //this command actls as MES but rints info into LOG file either SQL/TXT [Lupus]
		BUILDIN_DEF(summon,"si??"), // summons a slave monster [Celest]
		BUILDIN_DEF(isnight,""), // check whether it is night time [Celest]
		BUILDIN_DEF(isequipped,"i*"), // check whether another item/card has been equipped [Celest]
		BUILDIN_DEF(isequippedcnt,"i*"), // check how many items/cards are being equipped [Celest]
		BUILDIN_DEF(cardscnt,"i*"), // check how many items/cards are being equipped in the same arm [Lupus]
		BUILDIN_DEF(getrefine,""), // returns the refined number of the current item, or an item with index specified [celest]
		BUILDIN_DEF(night,""), // sets the server to night time
		BUILDIN_DEF(day,""), // sets the server to day time
		BUILDIN_DEF(defpattern,"iss"), // Define pattern to listen for [MouseJstr]
		BUILDIN_DEF(activatepset,"i"), // Activate a pattern set [MouseJstr]
		BUILDIN_DEF(deactivatepset,"i"), // Deactive a pattern set [MouseJstr]
		BUILDIN_DEF(deletepset,"i"), // Delete a pattern set [MouseJstr]
		BUILDIN_DEF(dressroom,"?"),
		BUILDIN_DEF(pcre_match,"ss"),
		BUILDIN_DEF(dispbottom,"s?"), //added from jA [Lupus]
		BUILDIN_DEF(getusersname,""),
		BUILDIN_DEF(recovery,"?????"),
		BUILDIN_DEF(getpetinfo,"i"),
		BUILDIN_DEF(gethominfo,"i"),
		BUILDIN_DEF(getmercinfo,"i?"),
		BUILDIN_DEF(checkequipedcard,"i"),
		BUILDIN_DEF(globalmes,"s?"), //end jA addition
		BUILDIN_DEF(unequip,"i"), // unequip command [Spectre]
		BUILDIN_DEF(getstrlen,"s"), //strlen [Valaris]
		BUILDIN_DEF(charisalpha,"si"), //isalpha [Valaris]
		BUILDIN_DEF(charat,"si"),
		BUILDIN_DEF(isstr,"v"),
		BUILDIN_DEF(getdatatype, "?"),
		BUILDIN_DEF(data_to_string, "?"),
		BUILDIN_DEF2(getd, "string_to_data", "?"),
		BUILDIN_DEF(chr,"i"),
		BUILDIN_DEF(ord,"s"),
		BUILDIN_DEF(setchar,"ssi"),
		BUILDIN_DEF(insertchar,"ssi"),
		BUILDIN_DEF(delchar,"si"),
		BUILDIN_DEF(strtoupper,"s"),
		BUILDIN_DEF(strtolower,"s"),
		BUILDIN_DEF(charisupper, "si"),
		BUILDIN_DEF(charislower, "si"),
		BUILDIN_DEF(substr,"sii"),
		BUILDIN_DEF(explode, "rss"),
		BUILDIN_DEF(implode, "r?"),
		BUILDIN_DEF(sprintf,"s*"),  // [Mirei]
		BUILDIN_DEF(sscanf,"ss*"),  // [Mirei]
		BUILDIN_DEF(strpos,"ss?"),
		BUILDIN_DEF(replacestr,"sss??"),
		BUILDIN_DEF(countstr,"ss?"),
		BUILDIN_DEF(setnpcdisplay,"sv??"),
		BUILDIN_DEF(compare,"ss"), // Lordalfa - To bring strstr to scripting Engine.
		BUILDIN_DEF(strcmp,"ss"),
		BUILDIN_DEF(getiteminfo,"vi"), //[Lupus] returns Items Buy / sell Price, etc info
		BUILDIN_DEF(setiteminfo,"iii"), //[Lupus] set Items Buy / sell Price, etc info
		BUILDIN_DEF(getequipcardid,"ii"), //[Lupus] returns CARD ID or other info from CARD slot N of equipped item
		BUILDIN_DEF(getequippedoptioninfo, "i"),
		BUILDIN_DEF(getequipoption, "iii"),
		BUILDIN_DEF(setequipoption, "iiii"),
		BUILDIN_DEF(getequipisenableopt, "i"),
		// List of mathematics commands --->
		BUILDIN_DEF(log10,"i"),
		BUILDIN_DEF(sqrt,"i"), //[zBuffer]
		BUILDIN_DEF_DEPRECATED(pow,"ii"), //[zBuffer]
		BUILDIN_DEF(distance,"iiii"), //[zBuffer]
		// <--- List of mathematics commands
		BUILDIN_DEF(min, "i*"),
		BUILDIN_DEF(max, "i*"),
		BUILDIN_DEF(cap_value, "iii"),
		BUILDIN_DEF(md5,"s"),
		BUILDIN_DEF(swap,"rr"),
		// [zBuffer] List of dynamic var commands --->
		BUILDIN_DEF(getd,"s"),
		BUILDIN_DEF(setd,"sv"),
		// <--- [zBuffer] List of dynamic var commands
		BUILDIN_DEF_DEPRECATED(petstat, "i"), // Deprecated 2019-03-11
		BUILDIN_DEF(callshop,"s?"), // [Skotlex]
		BUILDIN_DEF(npcshopitem,"sii*"), // [Lance]
		BUILDIN_DEF(npcshopadditem,"sii*"),
		BUILDIN_DEF(npcshopdelitem,"si*"),
		BUILDIN_DEF(npcshopattach,"s?"),
		BUILDIN_DEF(equip,"i"),
		BUILDIN_DEF(autoequip,"ii"),
		BUILDIN_DEF(equip2,"iiiiiii"),
		BUILDIN_DEF(setbattleflag,"si"),
		BUILDIN_DEF(getbattleflag,"s"),
		BUILDIN_DEF(setitemscript,"is?"), //Set NEW item bonus script. Lupus
		BUILDIN_DEF(disguise,"i"), //disguise player. Lupus
		BUILDIN_DEF(undisguise,""), //undisguise player. Lupus
		BUILDIN_DEF(getmonsterinfo,"ii"), //Lupus
		BUILDIN_DEF(addmonsterdrop,"vii"),
		BUILDIN_DEF(delmonsterdrop,"vi"),
		BUILDIN_DEF(axtoi,"s"),
		BUILDIN_DEF(query_sql,"s*"),
		BUILDIN_DEF(query_logsql,"s*"),
		BUILDIN_DEF(escape_sql,"v"),
		BUILDIN_DEF(atoi,"s"),
		BUILDIN_DEF(strtol,"si"),
		// [zBuffer] List of player cont commands --->
		BUILDIN_DEF(rid2name,"i"),
		BUILDIN_DEF(pcfollow,"ii"),
		BUILDIN_DEF(pcstopfollow,"i"),
		BUILDIN_DEF_DEPRECATED(pcblockmove,"ii"), // Deprecated 2018-05-04
		BUILDIN_DEF(setpcblock, "ii?"),
		BUILDIN_DEF(checkpcblock, "?"),
		// <--- [zBuffer] List of player cont commands
		// [zBuffer] List of mob control commands --->
		BUILDIN_DEF(getunittype,"i"),
		/* Unit Data */
		BUILDIN_DEF(setunitdata,"iiv??"),
		BUILDIN_DEF(getunitdata,"ii?"),
		BUILDIN_DEF(getunitname,"i"),
		BUILDIN_DEF(setunitname,"is"),
		BUILDIN_DEF(getunittitle,"i"),
		BUILDIN_DEF(setunittitle,"is"),
		BUILDIN_DEF(unitwalk,"ii?"),
		BUILDIN_DEF(unitiswalking, "?"),
		BUILDIN_DEF(unitkill,"i"),
		BUILDIN_DEF(unitwarp,"isii"),
		BUILDIN_DEF(unitattack,"iv?"),
		BUILDIN_DEF(unitstop,"i"),
		BUILDIN_DEF(unittalk,"is???"),
		BUILDIN_DEF(unitemote,"ii"),
		BUILDIN_DEF(unitskilluseid,"ivi?"), // originally by Qamera [Celest]
		BUILDIN_DEF(unitskillusepos,"iviii"), // [Celest]
		// <--- [zBuffer] List of mob control commands
		BUILDIN_DEF(sleep,"i"),
		BUILDIN_DEF(sleep2,"i"),
		BUILDIN_DEF(awake,"s"),
		BUILDIN_DEF(getvariableofnpc,"rs"),
		BUILDIN_DEF(getvariableofpc,"ri?"),
		BUILDIN_DEF(warpportal,"iisii"),
		BUILDIN_DEF2(homunculus_evolution,"homevolution",""), //[orn]
		BUILDIN_DEF2(homunculus_mutate,"hommutate","?"),
		BUILDIN_DEF2(homunculus_morphembryo,"morphembryo",""),
		BUILDIN_DEF2(homunculus_checkcall,"checkhomcall",""),
		BUILDIN_DEF2(homunculus_shuffle,"homshuffle",""), //[Zephyrus]
		BUILDIN_DEF(eaclass,"?"), //[Skotlex]
		BUILDIN_DEF(roclass,"i?"), //[Skotlex]
		BUILDIN_DEF(checkvending,"?"),
		BUILDIN_DEF(checkchatting,"?"),
		BUILDIN_DEF(checkidle,"?"),
		BUILDIN_DEF(openmail,""),
		BUILDIN_DEF(openauction,""),
		BUILDIN_DEF(checkcell,"siii"),
		BUILDIN_DEF(setcell,"siiiiii"),
		BUILDIN_DEF(setwall,"siiiiis"),
		BUILDIN_DEF(delwall,"s"),
		BUILDIN_DEF(searchitem,"rs"),
		BUILDIN_DEF(mercenary_create,"ii"),
		BUILDIN_DEF(mercenary_heal,"ii"),
		BUILDIN_DEF(mercenary_sc_start,"iii"),
		BUILDIN_DEF(mercenary_get_calls,"i"),
		BUILDIN_DEF(mercenary_get_faith,"i"),
		BUILDIN_DEF(mercenary_set_calls,"ii"),
		BUILDIN_DEF(mercenary_set_faith,"ii"),
		BUILDIN_DEF(readbook,"ii"),
		BUILDIN_DEF(setfont,"i"),
		BUILDIN_DEF(getfont, ""),
		BUILDIN_DEF(areamobuseskill,"siiiiviiiii"),
		BUILDIN_DEF(progressbar,"si"),
		BUILDIN_DEF(progressbar_unit,"si?"),
		BUILDIN_DEF(pushpc,"ii"),
		BUILDIN_DEF(buyingstore,"i"),
		BUILDIN_DEF(searchstores,"ii"),
		BUILDIN_DEF(showdigit,"i?"),
		BUILDIN_DEF(msgtable, "i?"),
		BUILDIN_DEF(msgtable2, "iv?"),
		// WoE SE
		BUILDIN_DEF(agitstart2,""),
		BUILDIN_DEF(agitend2,""),
		BUILDIN_DEF(agitcheck2,""),
		// Achievements [Smokexyz/Hercules]
		BUILDIN_DEF(achievement_progress, "iiii?"),
		BUILDIN_DEF(achievement_iscompleted, "i?"),
		// BattleGround
		BUILDIN_DEF(waitingroom2bg,"siiss?"),
		BUILDIN_DEF(waitingroom2bg_single,"isiis"),
		BUILDIN_DEF(bg_team_setxy,"iii"),
		BUILDIN_DEF(bg_warp,"isii"),
		BUILDIN_DEF(bg_monster,"isiisi?"),
		BUILDIN_DEF(bg_monster_set_team,"ii"),
		BUILDIN_DEF(bg_leave,""),
		BUILDIN_DEF(bg_destroy,"i"),
		BUILDIN_DEF(areapercentheal,"siiiiii"),
		BUILDIN_DEF(bg_get_data,"ii"),
		BUILDIN_DEF(bg_getareausers,"isiiii"),
		BUILDIN_DEF(bg_updatescore,"sii"),

		// Instancing
		BUILDIN_DEF(instance_create,"si?"),
		BUILDIN_DEF(instance_destroy,"?"),
		BUILDIN_DEF(instance_attachmap,"si??"),
		BUILDIN_DEF(instance_detachmap,"s?"),
		BUILDIN_DEF(instance_attach,"i"),
		BUILDIN_DEF(instance_id,""),
		BUILDIN_DEF(instance_set_timeout,"ii?"),
		BUILDIN_DEF(instance_init,"i"),
		BUILDIN_DEF(instance_announce,"isi?????"),
		BUILDIN_DEF(instance_npcname,"s?"),
		BUILDIN_DEF(has_instance,"s?"),
		BUILDIN_DEF(instance_warpall,"sii?"),
		BUILDIN_DEF(instance_check_party,"i???"),
		BUILDIN_DEF(instance_check_guild,"i???"),
		BUILDIN_DEF(instance_mapname,"s?"),
		BUILDIN_DEF(instance_set_respawn,"sii?"),
		BUILDIN_DEF2(has_instance,"has_instance2","s"),

		/**
		 * 3rd-related
		 **/
		BUILDIN_DEF(makerune,"i"),
		BUILDIN_DEF(hascashmount,""),//[Ind]
		BUILDIN_DEF(setcashmount,""),//[Ind]
		/**
		 * rAthena and beyond!
		 **/
		BUILDIN_DEF(getargcount,""),
		BUILDIN_DEF(getcharip,"?"),
		BUILDIN_DEF(is_function,"s"),
		BUILDIN_DEF(freeloop,"i"),
		BUILDIN_DEF(getrandgroupitem,"ii"),
		BUILDIN_DEF(cleanmap,"s"),
		BUILDIN_DEF2(cleanmap,"cleanarea","siiii"),
		BUILDIN_DEF(npcskill,"viii"),
		BUILDIN_DEF(itemeffect,"v"),
		BUILDIN_DEF2(itemeffect,"consumeitem","v"), /* alias of itemeffect */
		BUILDIN_DEF(delequip,"i"),
		/**
		 * @commands (script based)
		 **/
		BUILDIN_DEF(bindatcmd, "ss???"),
		BUILDIN_DEF(unbindatcmd, "s"),
		BUILDIN_DEF_DEPRECATED(useatcmd, "s"),
		BUILDIN_DEF(has_permission, "v?"),
		BUILDIN_DEF(can_use_command, "s?"),
		BUILDIN_DEF(add_group_command, "siii"),

		/**
		 * Item bound [Xantara] [Akinari] [Mhalicot/Hercules]
		 **/
		BUILDIN_DEF2(getitem,"getitembound","vii?"),
		BUILDIN_DEF2(getitem2,"getitembound2","viiiiiiiii?"),
		BUILDIN_DEF(countbound, "?"),
		BUILDIN_DEF(checkbound, "i???????"),

		//Quest Log System [Inkfish]
		BUILDIN_DEF(questinfo, "i?"),
		BUILDIN_DEF(setquestinfo, "i???"),
		BUILDIN_DEF(setquest, "i?"),
		BUILDIN_DEF(erasequest, "i?"),
		BUILDIN_DEF(completequest, "i?"),
		BUILDIN_DEF(questprogress, "i?"),
		BUILDIN_DEF(questactive, "i"),
		BUILDIN_DEF(changequest, "ii"),
		BUILDIN_DEF(showevent, "i?"),

		/**
		 * script_queue [Ind/Hercules]
		 **/
		BUILDIN_DEF(queue,""),
		BUILDIN_DEF(queuesize,"i"),
		BUILDIN_DEF(queueadd,"ii"),
		BUILDIN_DEF(queueremove,"ii"),
		BUILDIN_DEF(queueopt,"ii?"),
		BUILDIN_DEF(queuedel,"i"),
		BUILDIN_DEF(queueiterator,"i"),
		BUILDIN_DEF(qicheck,"i"),
		BUILDIN_DEF(qiget,"i"),
		BUILDIN_DEF(qiclear,"i"),

		BUILDIN_DEF(packageitem,"?"),

		BUILDIN_DEF(sit, "?"),
		BUILDIN_DEF(stand, "?"),
		BUILDIN_DEF(issit, "?"),

		BUILDIN_DEF(montransform, "vi?????"), // Monster Transform [malufett/Hercules]

		/* New BG Commands [Hercules] */
		BUILDIN_DEF(bg_create_team,"sii"),
		BUILDIN_DEF(bg_join_team,"i?"),
		BUILDIN_DEF(bg_match_over,"s?"),

		/* New Shop Support */
		BUILDIN_DEF(openshop,"?"),
		BUILDIN_DEF(sellitem, "i???*"),
		BUILDIN_DEF(sellitemcurrency, "ii?"),
		BUILDIN_DEF(startsellitem, "iii"),
		BUILDIN_DEF(endsellitem, ""),
		BUILDIN_DEF(stopselling,"i??"),
		BUILDIN_DEF(setcurrency,"i?"),
		BUILDIN_DEF(tradertype,"i"),
		BUILDIN_DEF(purchaseok,""),
		BUILDIN_DEF(shopcount, "i"),

		/* Navigation */
		BUILDIN_DEF(navigateto, "s??????"),

		/* Clan System */
		BUILDIN_DEF(clan_join,"i?"),
		BUILDIN_DEF(clan_leave,"?"),
		BUILDIN_DEF(clan_master,"i"),

		BUILDIN_DEF(channelmes, "ss"),
		BUILDIN_DEF(addchannelhandler, "ss"),
		BUILDIN_DEF(removechannelhandler, "ss"),
		BUILDIN_DEF(showscript, "s??"),
		BUILDIN_DEF(mergeitem,""),
		BUILDIN_DEF(getcalendartime, "ii??"),

		// -- RoDEX
		BUILDIN_DEF(rodex_sendmail, "isss???????????"),
		BUILDIN_DEF2(rodex_sendmail, "rodex_sendmail_acc", "isss???????????"),
		BUILDIN_DEF(rodex_sendmail2, "isss?????????????????????????????????????????"),
		BUILDIN_DEF2(rodex_sendmail2, "rodex_sendmail_acc2", "isss?????????????????????????????????????????"),
		BUILDIN_DEF(airship_respond, "i"),
		BUILDIN_DEF(openstylist,""),
		BUILDIN_DEF(_,"s"),
		BUILDIN_DEF2(_, "_$", "s"),

		// -- HatEffect
		BUILDIN_DEF(hateffect, "ii"),

		// camera
		BUILDIN_DEF(camerainfo, ""),
		BUILDIN_DEF(changecamera, "iii?"),

		BUILDIN_DEF(itempreview, "i"),
		BUILDIN_DEF(enchantitem, "iii"),
		BUILDIN_DEF(expandinventoryack, "i?"),
		BUILDIN_DEF(expandinventoryresult, "i"),
		BUILDIN_DEF(expandinventory, "i"),
		BUILDIN_DEF(getinventorysize, ""),

		BUILDIN_DEF(closeroulette, ""),
		BUILDIN_DEF(openrefineryui, ""),
		BUILDIN_DEF(setfavoriteitemidx, "ii"),
		BUILDIN_DEF(autofavoriteitem, "ii"),

		BUILDIN_DEF(identify, "i"),
		BUILDIN_DEF(identifyidx, "i"),
		BUILDIN_DEF(openlapineddukddakboxui, "i"),
	};
	int i, len = ARRAYLENGTH(BUILDIN);
	RECREATE(script->buildin, char *, script->buildin_count + len); // Pre-alloc to speed up
	memset(script->buildin + script->buildin_count, '\0', sizeof(char *) * len);
	for( i = 0; i < len; i++ ) {
		script->add_builtin(&BUILDIN[i], false);
	}
}
#undef BUILDIN_DEF
#undef BUILDIN_DEF2

static void script_label_add(int key, int pos)
{
	int idx = script->label_count;

	if( script->labels_size == script->label_count ) {
		script->labels_size += 1024;
		RECREATE(script->labels, struct script_label_entry, script->labels_size);
	}

	script->labels[idx].key = key;
	script->labels[idx].pos = pos;
	script->label_count++;
}

/**
 * Sets source-end constants for scripts to play with
 **/
static void script_hardcoded_constants(void)
{
	script->constdb_comment("Boolean");
	script->set_constant("true", 1, false, false);
	script->set_constant("false", 0, false, false);

	script->constdb_comment("Server defines");
	script->set_constant("PACKETVER",PACKETVER,false, false);
	script->set_constant("MAX_LEVEL",MAX_LEVEL,false, false);
	script->set_constant("MAX_STORAGE",MAX_STORAGE,false, false);
	script->set_constant("MAX_GUILD_STORAGE",MAX_GUILD_STORAGE,false, false);
	script->set_constant("MAX_CART", MAX_CART, false, false);
	script->set_constant("MAX_INVENTORY",MAX_INVENTORY,false, false);
	script->set_constant("FIXED_INVENTORY_SIZE", FIXED_INVENTORY_SIZE, false, false);
	script->set_constant("MAX_ZENY",MAX_ZENY,false, false);
	script->set_constant("MAX_BANK_ZENY", MAX_BANK_ZENY, false, false);
	script->set_constant("MAX_BG_MEMBERS",MAX_BG_MEMBERS,false, false);
	script->set_constant("MAX_CHAT_USERS",MAX_CHAT_USERS,false, false);
	script->set_constant("MAX_REFINE",MAX_REFINE,false, false);
	script->set_constant("MAX_ITEM_ID",MAX_ITEM_ID,false, false);
	script->set_constant("MAX_MENU_OPTIONS", MAX_MENU_OPTIONS, false, false);
	script->set_constant("MAX_MENU_LENGTH", MAX_MENU_LENGTH, false, false);
	script->set_constant("MOB_CLONE_START", MOB_CLONE_START, false, false);
	script->set_constant("MOB_CLONE_END", MOB_CLONE_END, false, false);
	script->set_constant("MAX_NPC_PER_MAP", MAX_NPC_PER_MAP, false, false);

	script->constdb_comment("status options");
	script->set_constant("Option_Nothing",OPTION_NOTHING,false, false);
	script->set_constant("Option_Sight",OPTION_SIGHT,false, false);
	script->set_constant("Option_Hide",OPTION_HIDE,false, false);
	script->set_constant("Option_Cloak",OPTION_CLOAK,false, false);
	script->set_constant("Option_Falcon",OPTION_FALCON,false, false);
	script->set_constant("Option_Riding",OPTION_RIDING,false, false);
	script->set_constant("Option_Invisible",OPTION_INVISIBLE,false, false);
	script->set_constant("Option_Orcish",OPTION_ORCISH,false, false);
	script->set_constant("Option_Wedding",OPTION_WEDDING,false, false);
	script->set_constant("Option_Chasewalk",OPTION_CHASEWALK,false, false);
	script->set_constant("Option_Flying",OPTION_FLYING,false, false);
	script->set_constant("Option_Xmas",OPTION_XMAS,false, false);
	script->set_constant("Option_Transform",OPTION_TRANSFORM,false, false);
	script->set_constant("Option_Summer",OPTION_SUMMER,false, false);
	script->set_constant("Option_Dragon1",OPTION_DRAGON1,false, false);
	script->set_constant("Option_Wug",OPTION_WUG,false, false);
	script->set_constant("Option_Wugrider",OPTION_WUGRIDER,false, false);
	script->set_constant("Option_Madogear",OPTION_MADOGEAR,false, false);
	script->set_constant("Option_Dragon2",OPTION_DRAGON2,false, false);
	script->set_constant("Option_Dragon3",OPTION_DRAGON3,false, false);
	script->set_constant("Option_Dragon4",OPTION_DRAGON4,false, false);
	script->set_constant("Option_Dragon5",OPTION_DRAGON5,false, false);
	script->set_constant("Option_Hanbok",OPTION_HANBOK,false, false);
	script->set_constant("Option_Oktoberfest",OPTION_OKTOBERFEST,false, false);
	script->set_constant("Option_Summer2", OPTION_SUMMER2, false, false);

	script->constdb_comment("status option compounds");
	script->set_constant("Option_Dragon",OPTION_DRAGON,false, false);
	script->set_constant("Option_Costume",OPTION_COSTUME,false, false);

	script->constdb_comment("send_target");
	script->set_constant("ALL_CLIENT",ALL_CLIENT,false, false);
	script->set_constant("ALL_SAMEMAP",ALL_SAMEMAP,false, false);
	script->set_constant("AREA",AREA,false, false);
	script->set_constant("AREA_WOS",AREA_WOS,false, false);
	script->set_constant("AREA_WOC",AREA_WOC,false, false);
	script->set_constant("AREA_WOSC",AREA_WOSC,false, false);
	script->set_constant("AREA_CHAT_WOC",AREA_CHAT_WOC,false, false);
	script->set_constant("CHAT",CHAT,false, false);
	script->set_constant("CHAT_WOS",CHAT_WOS,false, false);
	script->set_constant("PARTY",PARTY,false, false);
	script->set_constant("PARTY_WOS",PARTY_WOS,false, false);
	script->set_constant("PARTY_SAMEMAP",PARTY_SAMEMAP,false, false);
	script->set_constant("PARTY_SAMEMAP_WOS",PARTY_SAMEMAP_WOS,false, false);
	script->set_constant("PARTY_AREA",PARTY_AREA,false, false);
	script->set_constant("PARTY_AREA_WOS",PARTY_AREA_WOS,false, false);
	script->set_constant("GUILD",GUILD,false, false);
	script->set_constant("GUILD_WOS",GUILD_WOS,false, false);
	script->set_constant("GUILD_SAMEMAP",GUILD_SAMEMAP,false, false);
	script->set_constant("GUILD_SAMEMAP_WOS",GUILD_SAMEMAP_WOS,false, false);
	script->set_constant("GUILD_AREA",GUILD_AREA,false, false);
	script->set_constant("GUILD_AREA_WOS",GUILD_AREA_WOS,false, false);
	script->set_constant("GUILD_NOBG",GUILD_NOBG,false, false);
	script->set_constant("DUEL",DUEL,false, false);
	script->set_constant("DUEL_WOS",DUEL_WOS,false, false);
	script->set_constant("SELF",SELF,false, false);
	script->set_constant("BG",BG,false, false);
	script->set_constant("BG_WOS",BG_WOS,false, false);
	script->set_constant("BG_SAMEMAP",BG_SAMEMAP,false, false);
	script->set_constant("BG_SAMEMAP_WOS",BG_SAMEMAP_WOS,false, false);
	script->set_constant("BG_AREA",BG_AREA,false, false);
	script->set_constant("BG_AREA_WOS",BG_AREA_WOS,false, false);
	script->set_constant("BG_QUEUE",BG_QUEUE,false, false);

	script->constdb_comment("LOOK_ constants, use in setlook/changelook script commands");
	script->set_constant("LOOK_BASE", LOOK_BASE, false, false);
	script->set_constant("LOOK_HAIR", LOOK_HAIR, false, false);
	script->set_constant("LOOK_WEAPON", LOOK_WEAPON, false, false);
	script->set_constant("LOOK_HEAD_BOTTOM", LOOK_HEAD_BOTTOM, false, false);
	script->set_constant("LOOK_HEAD_TOP", LOOK_HEAD_TOP, false, false);
	script->set_constant("LOOK_HEAD_MID", LOOK_HEAD_MID, false, false);
	script->set_constant("LOOK_HAIR_COLOR", LOOK_HAIR_COLOR, false, false);
	script->set_constant("LOOK_CLOTHES_COLOR", LOOK_CLOTHES_COLOR, false, false);
	script->set_constant("LOOK_SHIELD", LOOK_SHIELD, false, false);
	script->set_constant("LOOK_SHOES", LOOK_SHOES, false, false);
	script->set_constant("LOOK_BODY", LOOK_BODY, false, false);
	script->set_constant("LOOK_FLOOR", LOOK_FLOOR, false, false);
	script->set_constant("LOOK_ROBE", LOOK_ROBE, false, false);
	script->set_constant("LOOK_BODY2", LOOK_BODY2, false, false);

	script->constdb_comment("Equip Position in Bits,  use with *getiteminfo type 5, or @inventorylist_equip");
	script->set_constant("EQP_HEAD_LOW", EQP_HEAD_LOW, false, false);
	script->set_constant("EQP_HEAD_MID", EQP_HEAD_MID, false, false);
	script->set_constant("EQP_HEAD_TOP", EQP_HEAD_TOP, false, false);
	script->set_constant("EQP_HAND_R", EQP_HAND_R, false, false);
	script->set_constant("EQP_HAND_L", EQP_HAND_L, false, false);
	script->set_constant("EQP_ARMOR", EQP_ARMOR, false, false);
	script->set_constant("EQP_SHOES", EQP_SHOES, false, false);
	script->set_constant("EQP_GARMENT", EQP_GARMENT, false, false);
	script->set_constant("EQP_ACC_L", EQP_ACC_L, false, false);
	script->set_constant("EQP_ACC_R", EQP_ACC_R, false, false);
	script->set_constant("EQP_COSTUME_HEAD_TOP", EQP_COSTUME_HEAD_TOP, false, false);
	script->set_constant("EQP_COSTUME_HEAD_MID", EQP_COSTUME_HEAD_MID, false, false);
	script->set_constant("EQP_COSTUME_HEAD_LOW", EQP_COSTUME_HEAD_LOW, false, false);
	script->set_constant("EQP_COSTUME_GARMENT", EQP_COSTUME_GARMENT, false, false);
	script->set_constant("EQP_AMMO", EQP_AMMO, false, false);
	script->set_constant("EQP_SHADOW_ARMOR", EQP_SHADOW_ARMOR, false, false);
	script->set_constant("EQP_SHADOW_WEAPON", EQP_SHADOW_WEAPON, false, false);
	script->set_constant("EQP_SHADOW_SHIELD", EQP_SHADOW_SHIELD, false, false);
	script->set_constant("EQP_SHADOW_SHOES", EQP_SHADOW_SHOES, false, false);
	script->set_constant("EQP_SHADOW_ACC_R", EQP_SHADOW_ACC_R, false, false);
	script->set_constant("EQP_SHADOW_ACC_L", EQP_SHADOW_ACC_L, false, false);
	// Synonyms and combined values
	script->set_constant("EQP_WEAPON", EQP_WEAPON, false, false);
	script->set_constant("EQP_SHIELD", EQP_SHIELD, false, false);
	script->set_constant("EQP_ARMS", EQP_ARMS, false, false);
	script->set_constant("EQP_HELM", EQP_HELM, false, false);
	script->set_constant("EQP_ACC", EQP_ACC, false, false);
	script->set_constant("EQP_COSTUME", EQP_COSTUME, false, false);
	script->set_constant("EQP_SHADOW_ACC", EQP_SHADOW_ACC, false, false);
	script->set_constant("EQP_SHADOW_ARMS", EQP_SHADOW_ARMS, false, false);

	script->constdb_comment("Item Option Types");
	script->set_constant("IT_OPT_INDEX", IT_OPT_INDEX, false, false);
	script->set_constant("IT_OPT_VALUE", IT_OPT_VALUE, false, false);
	script->set_constant("IT_OPT_PARAM", IT_OPT_PARAM, false, false);

	script->constdb_comment("Maximum Item Options");
	script->set_constant("MAX_ITEM_OPTIONS", MAX_ITEM_OPTIONS, false, false);

	script->constdb_comment("Navigation constants, use with *navigateto*");
	script->set_constant("NAV_NONE", NAV_NONE, false, false);
	script->set_constant("NAV_AIRSHIP_ONLY", NAV_AIRSHIP_ONLY, false, false);
	script->set_constant("NAV_SCROLL_ONLY", NAV_SCROLL_ONLY, false, false);
	script->set_constant("NAV_AIRSHIP_AND_SCROLL", NAV_AIRSHIP_AND_SCROLL, false, false);
	script->set_constant("NAV_KAFRA_ONLY", NAV_KAFRA_ONLY, false, false);
	script->set_constant("NAV_KAFRA_AND_AIRSHIP", NAV_KAFRA_AND_AIRSHIP, false, false);
	script->set_constant("NAV_KAFRA_AND_SCROLL", NAV_KAFRA_AND_SCROLL, false, false);
	script->set_constant("NAV_ALL", NAV_ALL, false, false);

	script->constdb_comment("BL types");
	script->set_constant("BL_PC",BL_PC,false, false);
	script->set_constant("BL_MOB",BL_MOB,false, false);
	script->set_constant("BL_PET",BL_PET,false, false);
	script->set_constant("BL_HOM",BL_HOM,false, false);
	script->set_constant("BL_MER",BL_MER,false, false);
	script->set_constant("BL_ITEM",BL_ITEM,false, false);
	script->set_constant("BL_SKILL",BL_SKILL,false, false);
	script->set_constant("BL_NPC",BL_NPC,false, false);
	script->set_constant("BL_CHAT",BL_CHAT,false, false);
	script->set_constant("BL_ELEM",BL_ELEM,false, false);
	script->set_constant("BL_CHAR",BL_CHAR,false, false);
	script->set_constant("BL_ALL",BL_ALL,false, false);

	script->constdb_comment("Refine Chance Types");
	script->set_constant("REFINE_CHANCE_TYPE_NORMAL", REFINE_CHANCE_TYPE_NORMAL, false, false);
	script->set_constant("REFINE_CHANCE_TYPE_ENRICHED", REFINE_CHANCE_TYPE_ENRICHED, false, false);
	script->set_constant("REFINE_CHANCE_TYPE_E_NORMAL", REFINE_CHANCE_TYPE_E_NORMAL, false, false);
	script->set_constant("REFINE_CHANCE_TYPE_E_ENRICHED", REFINE_CHANCE_TYPE_E_ENRICHED, false, false);

	script->constdb_comment("Player permissions");
	script->set_constant("PERM_TRADE", PC_PERM_TRADE, false, false);
	script->set_constant("PERM_PARTY", PC_PERM_PARTY, false, false);
	script->set_constant("PERM_ALL_SKILL", PC_PERM_ALL_SKILL, false, false);
	script->set_constant("PERM_USE_ALL_EQUIPMENT", PC_PERM_USE_ALL_EQUIPMENT, false, false);
	script->set_constant("PERM_SKILL_UNCONDITIONAL", PC_PERM_SKILL_UNCONDITIONAL, false, false);
	script->set_constant("PERM_JOIN_ALL_CHAT", PC_PERM_JOIN_ALL_CHAT, false, false);
	script->set_constant("PERM_NO_CHAT_KICK", PC_PERM_NO_CHAT_KICK, false, false);
	script->set_constant("PERM_HIDE_SESSION", PC_PERM_HIDE_SESSION, false, false);
	script->set_constant("PERM_RECEIVE_HACK_INFO", PC_PERM_RECEIVE_HACK_INFO, false, false);
	script->set_constant("PERM_WARP_ANYWHERE", PC_PERM_WARP_ANYWHERE, false, false);
	script->set_constant("PERM_VIEW_HPMETER", PC_PERM_VIEW_HPMETER, false, false);
	script->set_constant("PERM_VIEW_EQUIPMENT", PC_PERM_VIEW_EQUIPMENT, false, false);
	script->set_constant("PERM_USE_CHECK", PC_PERM_USE_CHECK, false, false);
	script->set_constant("PERM_USE_CHANGEMAPTYPE", PC_PERM_USE_CHANGEMAPTYPE, false, false);
	script->set_constant("PERM_USE_ALL_COMMANDS", PC_PERM_USE_ALL_COMMANDS, false, false);
	script->set_constant("PERM_RECEIVE_REQUESTS", PC_PERM_RECEIVE_REQUESTS, false, false);
	script->set_constant("PERM_SHOW_BOSS", PC_PERM_SHOW_BOSS, false, false);
	script->set_constant("PERM_DISABLE_PVM", PC_PERM_DISABLE_PVM, false, false);
	script->set_constant("PERM_DISABLE_PVP", PC_PERM_DISABLE_PVP, false, false);
	script->set_constant("PERM_DISABLE_CMD_DEAD", PC_PERM_DISABLE_CMD_DEAD, false, false);
	script->set_constant("PERM_HCHSYS_ADMIN", PC_PERM_HCHSYS_ADMIN, false, false);
	script->set_constant("PERM_TRADE_BOUND", PC_PERM_TRADE_BOUND, false, false);
	script->set_constant("PERM_DISABLE_PICK_UP", PC_PERM_DISABLE_PICK_UP, false, false);
	script->set_constant("PERM_DISABLE_STORE", PC_PERM_DISABLE_STORE, false, false);
	script->set_constant("PERM_DISABLE_EXP", PC_PERM_DISABLE_EXP, false, false);
	script->set_constant("PERM_DISABLE_SKILL_USAGE", PC_PERM_DISABLE_SKILL_USAGE, false, false);
	script->set_constant("PERM_BYPASS_NOSTORAGE", PC_PERM_BYPASS_NOSTORAGE, false, false);

	script->constdb_comment("Data types");
	script->set_constant("DATATYPE_NIL", DATATYPE_NIL, false, false);
	script->set_constant("DATATYPE_STR", DATATYPE_STR, false, false);
	script->set_constant("DATATYPE_INT", DATATYPE_INT, false, false);
	script->set_constant("DATATYPE_CONST", DATATYPE_CONST, false, false);
	script->set_constant("DATATYPE_PARAM", DATATYPE_PARAM, false, false);
	script->set_constant("DATATYPE_VAR", DATATYPE_VAR, false, false);
	script->set_constant("DATATYPE_LABEL", DATATYPE_LABEL, false, false);

	script->constdb_comment("Logmes types");
	script->set_constant("LOGMES_NPC", LOGMES_NPC, false, false);
	script->set_constant("LOGMES_ATCOMMAND", LOGMES_ATCOMMAND, false, false);

	script->constdb_comment("Item Subtypes (Weapon types)");
	script->set_constant("W_FIST", W_FIST, false, false);
	script->set_constant("W_DAGGER", W_DAGGER, false, false);
	script->set_constant("W_1HSWORD", W_1HSWORD, false, false);
	script->set_constant("W_2HSWORD", W_2HSWORD, false, false);
	script->set_constant("W_1HSPEAR", W_1HSPEAR, false, false);
	script->set_constant("W_2HSPEAR", W_2HSPEAR, false, false);
	script->set_constant("W_1HAXE", W_1HAXE, false, false);
	script->set_constant("W_2HAXE", W_2HAXE, false, false);
	script->set_constant("W_MACE", W_MACE, false, false);
	script->set_constant("W_2HMACE", W_2HMACE, false, false);
	script->set_constant("W_STAFF", W_STAFF, false, false);
	script->set_constant("W_BOW", W_BOW, false, false);
	script->set_constant("W_KNUCKLE", W_KNUCKLE, false, false);
	script->set_constant("W_MUSICAL", W_MUSICAL, false, false);
	script->set_constant("W_WHIP", W_WHIP, false, false);
	script->set_constant("W_BOOK", W_BOOK, false, false);
	script->set_constant("W_KATAR", W_KATAR, false, false);
	script->set_constant("W_REVOLVER", W_REVOLVER, false, false);
	script->set_constant("W_RIFLE", W_RIFLE, false, false);
	script->set_constant("W_GATLING", W_GATLING, false, false);
	script->set_constant("W_SHOTGUN", W_SHOTGUN, false, false);
	script->set_constant("W_GRENADE", W_GRENADE, false, false);
	script->set_constant("W_HUUMA", W_HUUMA, false, false);
	script->set_constant("W_2HSTAFF", W_2HSTAFF, false, false);

	script->constdb_comment("Item Subtypes (Ammunition types)");
	script->set_constant("A_ARROW", A_ARROW, false, false);
	script->set_constant("A_DAGGER", A_DAGGER, false, false);
	script->set_constant("A_BULLET", A_BULLET, false, false);
	script->set_constant("A_SHELL", A_SHELL, false, false);
	script->set_constant("A_GRENADE", A_GRENADE, false, false);
	script->set_constant("A_SHURIKEN", A_SHURIKEN, false, false);
	script->set_constant("A_KUNAI", A_KUNAI, false, false);
	script->set_constant("A_CANNONBALL", A_CANNONBALL, false, false);
	script->set_constant("A_THROWWEAPON", A_THROWWEAPON, false, false);

	script->constdb_comment("Item Upper Masks");
	script->set_constant("ITEMUPPER_NONE", ITEMUPPER_NONE, false, false);
	script->set_constant("ITEMUPPER_NORMAL", ITEMUPPER_NORMAL, false, false);
	script->set_constant("ITEMUPPER_UPPER", ITEMUPPER_UPPER, false, false);
	script->set_constant("ITEMUPPER_BABY", ITEMUPPER_BABY, false, false);
	script->set_constant("ITEMUPPER_THIRD", ITEMUPPER_THIRD, false, false);
	script->set_constant("ITEMUPPER_THIRDUPPER", ITEMUPPER_THIRDUPPER, false, false);
	script->set_constant("ITEMUPPER_THIRDBABY", ITEMUPPER_THIRDBABY, false, false);
	script->set_constant("ITEMUPPER_ALL", ITEMUPPER_ALL, false, false);

	script->constdb_comment("dressroom modes");
	script->set_constant("DRESSROOM_OPEN", DRESSROOM_OPEN, false, false);
	script->set_constant("DRESSROOM_CLOSE", DRESSROOM_CLOSE, false, false);

	script->constdb_comment("getmapinfo options");
	script->set_constant("MAPINFO_NAME", MAPINFO_NAME, false, false);
	script->set_constant("MAPINFO_ID", MAPINFO_ID, false, false);
	script->set_constant("MAPINFO_SIZE_X", MAPINFO_SIZE_X, false, false);
	script->set_constant("MAPINFO_SIZE_Y", MAPINFO_SIZE_Y, false, false);
	script->set_constant("MAPINFO_ZONE", MAPINFO_ZONE, false, false);
	script->set_constant("MAPINFO_NPC_COUNT", MAPINFO_NPC_COUNT, false, false);

	script->constdb_comment("consolemes options");
	script->set_constant("CONSOLEMES_DEBUG", CONSOLEMES_DEBUG, false, false);
	script->set_constant("CONSOLEMES_ERROR", CONSOLEMES_ERROR, false, false);
	script->set_constant("CONSOLEMES_WARNING", CONSOLEMES_WARNING, false, false);
	script->set_constant("CONSOLEMES_INFO", CONSOLEMES_INFO, false, false);
	script->set_constant("CONSOLEMES_STATUS", CONSOLEMES_STATUS, false, false);
	script->set_constant("CONSOLEMES_NOTICE", CONSOLEMES_NOTICE, false, false);

	script->constdb_comment("set/getiteminfo options");
	script->set_constant("ITEMINFO_BUYPRICE", ITEMINFO_BUYPRICE, false, false);
	script->set_constant("ITEMINFO_SELLPRICE", ITEMINFO_SELLPRICE, false, false);
	script->set_constant("ITEMINFO_TYPE", ITEMINFO_TYPE, false, false);
	script->set_constant("ITEMINFO_MAXCHANCE", ITEMINFO_MAXCHANCE, false, false);
	script->set_constant("ITEMINFO_SEX", ITEMINFO_SEX, false, false);
	script->set_constant("ITEMINFO_LOC", ITEMINFO_LOC, false, false);
	script->set_constant("ITEMINFO_WEIGHT", ITEMINFO_WEIGHT, false, false);
	script->set_constant("ITEMINFO_ATK", ITEMINFO_ATK, false, false);
	script->set_constant("ITEMINFO_DEF", ITEMINFO_DEF, false, false);
	script->set_constant("ITEMINFO_RANGE", ITEMINFO_RANGE, false, false);
	script->set_constant("ITEMINFO_SLOTS", ITEMINFO_SLOTS, false, false);
	script->set_constant("ITEMINFO_SUBTYPE", ITEMINFO_SUBTYPE, false, false);
	script->set_constant("ITEMINFO_ELV", ITEMINFO_ELV, false, false);
	script->set_constant("ITEMINFO_WLV", ITEMINFO_WLV, false, false);
	script->set_constant("ITEMINFO_VIEWID", ITEMINFO_VIEWID, false, false);
	script->set_constant("ITEMINFO_MATK", ITEMINFO_MATK, false, false);
	script->set_constant("ITEMINFO_VIEWSPRITE", ITEMINFO_VIEWSPRITE, false, false);
	script->set_constant("ITEMINFO_TRADE", ITEMINFO_TRADE, false, false);
	script->set_constant("ITEMINFO_ELV_MAX", ITEMINFO_ELV_MAX, false, false);
	script->set_constant("ITEMINFO_DROPEFFECT_MODE", ITEMINFO_DROPEFFECT_MODE, false, false);
	script->set_constant("ITEMINFO_DELAY", ITEMINFO_DELAY, false, false);
	script->set_constant("ITEMINFO_CLASS_BASE_1", ITEMINFO_CLASS_BASE_1, false, false);
	script->set_constant("ITEMINFO_CLASS_BASE_2", ITEMINFO_CLASS_BASE_2, false, false);
	script->set_constant("ITEMINFO_CLASS_BASE_3", ITEMINFO_CLASS_BASE_3, false, false);
	script->set_constant("ITEMINFO_CLASS_UPPER", ITEMINFO_CLASS_UPPER, false, false);
	script->set_constant("ITEMINFO_FLAG_NO_REFINE", ITEMINFO_FLAG_NO_REFINE, false, false);
	script->set_constant("ITEMINFO_FLAG_DELAY_CONSUME", ITEMINFO_FLAG_DELAY_CONSUME, false, false);
	script->set_constant("ITEMINFO_FLAG_AUTOEQUIP", ITEMINFO_FLAG_AUTOEQUIP, false, false);
	script->set_constant("ITEMINFO_FLAG_AUTO_FAVORITE", ITEMINFO_FLAG_AUTO_FAVORITE, false, false);
	script->set_constant("ITEMINFO_FLAG_BUYINGSTORE", ITEMINFO_FLAG_BUYINGSTORE, false, false);
	script->set_constant("ITEMINFO_FLAG_BINDONEQUIP", ITEMINFO_FLAG_BINDONEQUIP, false, false);
	script->set_constant("ITEMINFO_FLAG_KEEPAFTERUSE", ITEMINFO_FLAG_KEEPAFTERUSE, false, false);
	script->set_constant("ITEMINFO_FLAG_FORCE_SERIAL", ITEMINFO_FLAG_FORCE_SERIAL, false, false);
	script->set_constant("ITEMINFO_FLAG_NO_OPTIONS", ITEMINFO_FLAG_NO_OPTIONS, false, false);
	script->set_constant("ITEMINFO_FLAG_DROP_ANNOUNCE", ITEMINFO_FLAG_DROP_ANNOUNCE, false, false);
	script->set_constant("ITEMINFO_FLAG_SHOWDROPEFFECT", ITEMINFO_FLAG_SHOWDROPEFFECT, false, false);
	script->set_constant("ITEMINFO_STACK_AMOUNT", ITEMINFO_STACK_AMOUNT, false, false);
	script->set_constant("ITEMINFO_STACK_FLAG", ITEMINFO_STACK_FLAG, false, false);
	script->set_constant("ITEMINFO_ITEM_USAGE_FLAG", ITEMINFO_ITEM_USAGE_FLAG, false, false);
	script->set_constant("ITEMINFO_ITEM_USAGE_OVERRIDE", ITEMINFO_ITEM_USAGE_OVERRIDE, false, false);
	script->set_constant("ITEMINFO_GM_LV_TRADE_OVERRIDE", ITEMINFO_GM_LV_TRADE_OVERRIDE, false, false);
	script->set_constant("ITEMINFO_ID", ITEMINFO_ID, false, false);
	script->set_constant("ITEMINFO_AEGISNAME", ITEMINFO_AEGISNAME, false, false);
	script->set_constant("ITEMINFO_NAME", ITEMINFO_NAME, false, false);

	script->constdb_comment("getmercinfo options");
	script->set_constant("MERCINFO_ID,", MERCINFO_ID, false, false);
	script->set_constant("MERCINFO_CLASS", MERCINFO_CLASS, false, false);
	script->set_constant("MERCINFO_NAME", MERCINFO_NAME, false, false);
	script->set_constant("MERCINFO_FAITH", MERCINFO_FAITH, false, false);
	script->set_constant("MERCINFO_CALLS", MERCINFO_CALLS, false, false);
	script->set_constant("MERCINFO_KILLCOUNT", MERCINFO_KILLCOUNT, false, false);
	script->set_constant("MERCINFO_LIFETIME", MERCINFO_LIFETIME, false, false);
	script->set_constant("MERCINFO_LEVEL", MERCINFO_LEVEL, false, false);
	script->set_constant("MERCINFO_GID", MERCINFO_GID, false, false);

	script->constdb_comment("getpetinfo options");
	script->set_constant("PETINFO_ID", PETINFO_ID, false, false);
	script->set_constant("PETINFO_CLASS", PETINFO_CLASS, false, false);
	script->set_constant("PETINFO_NAME", PETINFO_NAME, false, false);
	script->set_constant("PETINFO_INTIMACY", PETINFO_INTIMACY, false, false);
	script->set_constant("PETINFO_HUNGRY", PETINFO_HUNGRY, false, false);
	script->set_constant("PETINFO_RENAME", PETINFO_RENAME, false, false);
	script->set_constant("PETINFO_GID", PETINFO_GID, false, false);
	script->set_constant("PETINFO_EGGITEM", PETINFO_EGGITEM, false, false);
	script->set_constant("PETINFO_FOODITEM", PETINFO_FOODITEM, false, false);
	script->set_constant("PETINFO_ACCESSORYITEM", PETINFO_ACCESSORYITEM, false, false);
	script->set_constant("PETINFO_ACCESSORYFLAG", PETINFO_ACCESSORYFLAG, false, false);
	script->set_constant("PETINFO_EVO_EGGID", PETINFO_EVO_EGGID, false, false);
	script->set_constant("PETINFO_AUTOFEED", PETINFO_AUTOFEED, false, false);

	script->constdb_comment("Pet hunger levels");
	script->set_constant("PET_HUNGER_STARVING", PET_HUNGER_STARVING, false, false);
	script->set_constant("PET_HUNGER_VERY_HUNGRY", PET_HUNGER_VERY_HUNGRY, false, false);
	script->set_constant("PET_HUNGER_HUNGRY", PET_HUNGER_HUNGRY, false, false);
	script->set_constant("PET_HUNGER_NEUTRAL", PET_HUNGER_NEUTRAL, false, false);
	script->set_constant("PET_HUNGER_SATISFIED", PET_HUNGER_SATISFIED, false, false);
	script->set_constant("PET_HUNGER_STUFFED", PET_HUNGER_STUFFED, false, false);

	script->constdb_comment("Pet intimacy levels");
	script->set_constant("PET_INTIMACY_NONE", PET_INTIMACY_NONE, false, false);
	script->set_constant("PET_INTIMACY_AWKWARD", PET_INTIMACY_AWKWARD, false, false);
	script->set_constant("PET_INTIMACY_SHY", PET_INTIMACY_SHY, false, false);
	script->set_constant("PET_INTIMACY_NEUTRAL", PET_INTIMACY_NEUTRAL, false, false);
	script->set_constant("PET_INTIMACY_CORDIAL", PET_INTIMACY_CORDIAL, false, false);
	script->set_constant("PET_INTIMACY_LOYAL", PET_INTIMACY_LOYAL, false, false);
	script->set_constant("PET_INTIMACY_MAX", PET_INTIMACY_MAX, false, false);

	script->constdb_comment("monster skill states");
	script->set_constant("MSS_ANY", MSS_ANY, false, false);
	script->set_constant("MSS_IDLE", MSS_IDLE, false, false);
	script->set_constant("MSS_WALK", MSS_WALK, false, false);
	script->set_constant("MSS_LOOT", MSS_LOOT, false, false);
	script->set_constant("MSS_DEAD", MSS_DEAD, false, false);
	script->set_constant("MSS_BERSERK", MSS_BERSERK, false, false);
	script->set_constant("MSS_ANGRY", MSS_ANGRY, false, false);
	script->set_constant("MSS_RUSH", MSS_RUSH, false, false);
	script->set_constant("MSS_FOLLOW", MSS_FOLLOW, false, false);
	script->set_constant("MSS_ANYTARGET", MSS_ANYTARGET, false, false);

	script->constdb_comment("monster skill conditions");
	script->set_constant("MSC_ANY", -1, false, false);
	script->set_constant("MSC_ALWAYS", MSC_ALWAYS, false, false);
	script->set_constant("MSC_MYHPLTMAXRATE", MSC_MYHPLTMAXRATE, false, false);
	script->set_constant("MSC_MYHPINRATE", MSC_MYHPINRATE, false, false);
	script->set_constant("MSC_FRIENDHPLTMAXRATE", MSC_FRIENDHPLTMAXRATE, false, false);
	script->set_constant("MSC_FRIENDHPINRATE", MSC_FRIENDHPINRATE, false, false);
	script->set_constant("MSC_MYSTATUSON", MSC_MYSTATUSON, false, false);
	script->set_constant("MSC_MYSTATUSOFF", MSC_MYSTATUSOFF, false, false);
	script->set_constant("MSC_FRIENDSTATUSON", MSC_FRIENDSTATUSON, false, false);
	script->set_constant("MSC_FRIENDSTATUSOFF", MSC_FRIENDSTATUSOFF, false, false);
	script->set_constant("MSC_ATTACKPCGT", MSC_ATTACKPCGT, false, false);
	script->set_constant("MSC_ATTACKPCGE", MSC_ATTACKPCGE, false, false);
	script->set_constant("MSC_SLAVELT", MSC_SLAVELT, false, false);
	script->set_constant("MSC_SLAVELE", MSC_SLAVELE, false, false);
	script->set_constant("MSC_CLOSEDATTACKED", MSC_CLOSEDATTACKED, false, false);
	script->set_constant("MSC_LONGRANGEATTACKED", MSC_LONGRANGEATTACKED, false, false);
	script->set_constant("MSC_SKILLUSED", MSC_SKILLUSED, false, false);
	script->set_constant("MSC_AFTERSKILL", MSC_AFTERSKILL, false, false);
	script->set_constant("MSC_CASTTARGETED", MSC_CASTTARGETED, false, false);
	script->set_constant("MSC_RUDEATTACKED", MSC_RUDEATTACKED, false, false);
	script->set_constant("MSC_MASTERHPLTMAXRATE", MSC_MASTERHPLTMAXRATE, false, false);
	script->set_constant("MSC_MASTERATTACKED", MSC_MASTERATTACKED, false, false);
	script->set_constant("MSC_ALCHEMIST", MSC_ALCHEMIST, false, false);
	script->set_constant("MSC_SPAWN", MSC_SPAWN, false, false);

	script->constdb_comment("monster skill targets");
	script->set_constant("MST_TARGET", MST_TARGET, false, false);
	script->set_constant("MST_RANDOM", MST_RANDOM , false, false);
	script->set_constant("MST_SELF", MST_SELF, false, false);
	script->set_constant("MST_FRIEND", MST_FRIEND , false, false);
	script->set_constant("MST_MASTER", MST_MASTER , false, false);
	script->set_constant("MST_AROUND5", MST_AROUND5, false, false);
	script->set_constant("MST_AROUND6", MST_AROUND6, false, false);
	script->set_constant("MST_AROUND7", MST_AROUND7, false, false);
	script->set_constant("MST_AROUND8", MST_AROUND8, false, false);
	script->set_constant("MST_AROUND1", MST_AROUND1, false, false);
	script->set_constant("MST_AROUND2", MST_AROUND2, false, false);
	script->set_constant("MST_AROUND3", MST_AROUND3, false, false);
	script->set_constant("MST_AROUND4", MST_AROUND4, false, false);
	script->set_constant("MST_AROUND", MST_AROUND , false, false);

	script->constdb_comment("pc block constants, use with *setpcblock* and *checkpcblock*");
	script->set_constant("PCBLOCK_NONE",     PCBLOCK_NONE,     false, false);
	script->set_constant("PCBLOCK_MOVE",     PCBLOCK_MOVE,     false, false);
	script->set_constant("PCBLOCK_ATTACK",   PCBLOCK_ATTACK,   false, false);
	script->set_constant("PCBLOCK_SKILL",    PCBLOCK_SKILL,    false, false);
	script->set_constant("PCBLOCK_USEITEM",  PCBLOCK_USEITEM,  false, false);
	script->set_constant("PCBLOCK_CHAT",     PCBLOCK_CHAT,     false, false);
	script->set_constant("PCBLOCK_IMMUNE",   PCBLOCK_IMMUNE,   false, false);
	script->set_constant("PCBLOCK_SITSTAND", PCBLOCK_SITSTAND, false, false);
	script->set_constant("PCBLOCK_COMMANDS", PCBLOCK_COMMANDS, false, false);
	script->set_constant("PCBLOCK_NPC",      PCBLOCK_NPC,      false, false);

	script->constdb_comment("private airship responds");
	script->set_constant("P_AIRSHIP_NONE", P_AIRSHIP_NONE, false, false);
	script->set_constant("P_AIRSHIP_RETRY", P_AIRSHIP_RETRY, false, false);
	script->set_constant("P_AIRSHIP_INVALID_START_MAP", P_AIRSHIP_INVALID_START_MAP, false, false);
	script->set_constant("P_AIRSHIP_INVALID_END_MAP", P_AIRSHIP_INVALID_END_MAP, false, false);
	script->set_constant("P_AIRSHIP_ITEM_NOT_ENOUGH", P_AIRSHIP_ITEM_NOT_ENOUGH, false, false);
	script->set_constant("P_AIRSHIP_ITEM_INVALID", P_AIRSHIP_ITEM_INVALID, false, false);

	script->constdb_comment("questinfo types");
	script->set_constant("QINFO_JOB", QINFO_JOB, false, false);
	script->set_constant("QINFO_SEX", QINFO_SEX, false, false);
	script->set_constant("QINFO_BASE_LEVEL", QINFO_BASE_LEVEL, false, false);
	script->set_constant("QINFO_JOB_LEVEL", QINFO_JOB_LEVEL, false, false);
	script->set_constant("QINFO_ITEM", QINFO_ITEM, false, false);
	script->set_constant("QINFO_HOMUN_LEVEL", QINFO_HOMUN_LEVEL, false, false);
	script->set_constant("QINFO_HOMUN_TYPE", QINFO_HOMUN_TYPE, false, false);
	script->set_constant("QINFO_QUEST", QINFO_QUEST, false, false);
	script->set_constant("QINFO_MERCENARY_CLASS", QINFO_MERCENARY_CLASS, false, false);

	script->constdb_comment("function types");
	script->set_constant("FUNCTION_IS_COMMAND", FUNCTION_IS_COMMAND, false, false);
	script->set_constant("FUNCTION_IS_GLOBAL", FUNCTION_IS_GLOBAL, false, false);
	script->set_constant("FUNCTION_IS_LOCAL", FUNCTION_IS_LOCAL, false, false);
	script->set_constant("FUNCTION_IS_LABEL", FUNCTION_IS_LABEL, false, false);

	script->constdb_comment("item trade restrictions");
	script->set_constant("ITR_NONE", ITR_NONE, false, false);
	script->set_constant("ITR_NODROP", ITR_NODROP, false, false);
	script->set_constant("ITR_NOTRADE", ITR_NOTRADE, false, false);
	script->set_constant("ITR_PARTNEROVERRIDE", ITR_PARTNEROVERRIDE, false, false);
	script->set_constant("ITR_NOSELLTONPC", ITR_NOSELLTONPC, false, false);
	script->set_constant("ITR_NOCART", ITR_NOCART, false, false);
	script->set_constant("ITR_NOSTORAGE", ITR_NOSTORAGE, false, false);
	script->set_constant("ITR_NOGSTORAGE", ITR_NOGSTORAGE, false, false);
	script->set_constant("ITR_NOMAIL", ITR_NOMAIL, false, false);
	script->set_constant("ITR_NOAUCTION", ITR_NOAUCTION, false, false);
	script->set_constant("ITR_ALL", ITR_ALL, false, false);

	script->constdb_comment("inventory expand ack responds");
	script->set_constant("EXPAND_INV_ASK_CONFIRMATION", EXPAND_INVENTORY_ASK_CONFIRMATION, false, false);
	script->set_constant("EXPAND_INV_FAILED", EXPAND_INVENTORY_FAILED, false, false);
	script->set_constant("EXPAND_INV_OTHER_WORK", EXPAND_INVENTORY_OTHER_WORK, false, false);
	script->set_constant("EXPAND_INV_MISSING_ITEM", EXPAND_INVENTORY_MISSING_ITEM, false, false);
	script->set_constant("EXPAND_INV_MAX_SIZE", EXPAND_INVENTORY_MAX_SIZE, false, false);

	script->constdb_comment("inventory expand final responds");
	script->set_constant("EXPAND_INV_RESULT_SUCCESS", EXPAND_INVENTORY_RESULT_SUCCESS, false, false);
	script->set_constant("EXPAND_INV_RESULT_FAILED", EXPAND_INVENTORY_RESULT_FAILED, false, false);
	script->set_constant("EXPAND_INV_RESULT_OTHER_WORK", EXPAND_INVENTORY_RESULT_OTHER_WORK, false, false);
	script->set_constant("EXPAND_INV_RESULT_MISSING_ITEM", EXPAND_INVENTORY_RESULT_MISSING_ITEM, false, false);
	script->set_constant("EXPAND_INV_RESULT_MAX_SIZE", EXPAND_INVENTORY_RESULT_MAX_SIZE, false, false);

	script->constdb_comment("trader type");
	script->set_constant("NST_ZENY", NST_ZENY, false, false);
	script->set_constant("NST_CASH", NST_CASH, false, false);
	script->set_constant("NST_MARKET", NST_MARKET, false, false);
	script->set_constant("NST_CUSTOM", NST_CUSTOM, false, false);
	script->set_constant("NST_BARTER", NST_BARTER, false, false);
	script->set_constant("NST_EXPANDED_BARTER", NST_EXPANDED_BARTER, false, false);

	script->constdb_comment("script unit data types");
	script->set_constant("UDT_TYPE", UDT_TYPE, false, false);
	script->set_constant("UDT_SIZE", UDT_SIZE, false, false);
	script->set_constant("UDT_LEVEL", UDT_LEVEL, false, false);
	script->set_constant("UDT_HP", UDT_HP, false, false);
	script->set_constant("UDT_MAXHP", UDT_MAXHP, false, false);
	script->set_constant("UDT_SP", UDT_SP, false, false);
	script->set_constant("UDT_MAXSP", UDT_MAXSP, false, false);
	script->set_constant("UDT_MASTERAID", UDT_MASTERAID, false, false);
	script->set_constant("UDT_MASTERCID", UDT_MASTERCID, false, false);
	script->set_constant("UDT_MAPIDXY", UDT_MAPIDXY, false, true);  // for setunitdata use *unitwarp, for getunitdata use *getmapxy
	script->set_constant("UDT_WALKTOXY", UDT_WALKTOXY, false, true);  // use *unitwalk
	script->set_constant("UDT_SPEED", UDT_SPEED, false, false);
	script->set_constant("UDT_MODE", UDT_MODE, false, false);
	script->set_constant("UDT_AI", UDT_AI, false, false);
	script->set_constant("UDT_SCOPTION", UDT_SCOPTION, false, false);
	script->set_constant("UDT_SEX", UDT_SEX, false, false);
	script->set_constant("UDT_CLASS", UDT_CLASS, false, false);
	script->set_constant("UDT_HAIRSTYLE", UDT_HAIRSTYLE, false, false);
	script->set_constant("UDT_HAIRCOLOR", UDT_HAIRCOLOR, false, false);
	script->set_constant("UDT_HEADBOTTOM", UDT_HEADBOTTOM, false, false);
	script->set_constant("UDT_HEADMIDDLE", UDT_HEADMIDDLE, false, false);
	script->set_constant("UDT_HEADTOP", UDT_HEADTOP, false, false);
	script->set_constant("UDT_CLOTHCOLOR", UDT_CLOTHCOLOR, false, false);
	script->set_constant("UDT_SHIELD", UDT_SHIELD, false, false);
	script->set_constant("UDT_WEAPON", UDT_WEAPON, false, false);
	script->set_constant("UDT_LOOKDIR", UDT_LOOKDIR, false, false);
	script->set_constant("UDT_CANMOVETICK", UDT_CANMOVETICK, false, false);
	script->set_constant("UDT_STR", UDT_STR, false, false);
	script->set_constant("UDT_AGI", UDT_AGI, false, false);
	script->set_constant("UDT_VIT", UDT_VIT, false, false);
	script->set_constant("UDT_INT", UDT_INT, false, false);
	script->set_constant("UDT_DEX", UDT_DEX, false, false);
	script->set_constant("UDT_LUK", UDT_LUK, false, false);
	script->set_constant("UDT_ATKRANGE", UDT_ATKRANGE, false, false);
	script->set_constant("UDT_ATKMIN", UDT_ATKMIN, false, false);
	script->set_constant("UDT_ATKMAX", UDT_ATKMAX, false, false);
	script->set_constant("UDT_MATKMIN", UDT_MATKMIN, false, false);
	script->set_constant("UDT_MATKMAX", UDT_MATKMAX, false, false);
	script->set_constant("UDT_DEF", UDT_DEF, false, false);
	script->set_constant("UDT_MDEF", UDT_MDEF, false, false);
	script->set_constant("UDT_HIT", UDT_HIT, false, false);
	script->set_constant("UDT_FLEE", UDT_FLEE, false, false);
	script->set_constant("UDT_PDODGE", UDT_PDODGE, false, false);
	script->set_constant("UDT_CRIT", UDT_CRIT, false, false);
	script->set_constant("UDT_RACE", UDT_RACE, false, false);
	script->set_constant("UDT_ELETYPE", UDT_ELETYPE, false, false);
	script->set_constant("UDT_ELELEVEL", UDT_ELELEVEL, false, false);
	script->set_constant("UDT_AMOTION", UDT_AMOTION, false, false);
	script->set_constant("UDT_ADELAY", UDT_ADELAY, false, false);
	script->set_constant("UDT_DMOTION", UDT_DMOTION, false, false);
	script->set_constant("UDT_HUNGER", UDT_HUNGER, false, false);
	script->set_constant("UDT_INTIMACY", UDT_INTIMACY, false, false);
	script->set_constant("UDT_LIFETIME", UDT_LIFETIME, false, false);
	script->set_constant("UDT_MERC_KILLCOUNT", UDT_MERC_KILLCOUNT, false, false);
	script->set_constant("UDT_STATPOINT", UDT_STATPOINT, false, false);
	script->set_constant("UDT_ROBE", UDT_ROBE, false, false);
	script->set_constant("UDT_BODY2", UDT_BODY2, false, false);
	script->set_constant("UDT_GROUP", UDT_GROUP, false, false);
	script->set_constant("UDT_DAMAGE_TAKEN_RATE", UDT_DAMAGE_TAKEN_RATE, false, false);

	script->constdb_comment("getguildonline types");
	script->set_constant("GUILD_ONLINE_ALL", GUILD_ONLINE_ALL, false, false);
	script->set_constant("GUILD_ONLINE_VENDOR", GUILD_ONLINE_VENDOR, false, false);
	script->set_constant("GUILD_ONLINE_NO_VENDOR", GUILD_ONLINE_NO_VENDOR, false, false);

	script->constdb_comment("Siege Types");
	script->set_constant("SIEGE_TYPE_FE", SIEGE_TYPE_FE, false, false);
	script->set_constant("SIEGE_TYPE_SE", SIEGE_TYPE_SE, false, false);
	script->set_constant("SIEGE_TYPE_TE", SIEGE_TYPE_TE, false, false);

	script->constdb_comment("guildinfo types");
	script->set_constant("GUILDINFO_NAME", GUILDINFO_NAME, false, false);
	script->set_constant("GUILDINFO_ID", GUILDINFO_ID, false, false);
	script->set_constant("GUILDINFO_LEVEL", GUILDINFO_LEVEL, false, false);
	script->set_constant("GUILDINFO_ONLINE", GUILDINFO_ONLINE, false, false);
	script->set_constant("GUILDINFO_AV_LEVEL", GUILDINFO_AV_LEVEL, false, false);
	script->set_constant("GUILDINFO_MAX_MEMBERS", GUILDINFO_MAX_MEMBERS, false, false);
	script->set_constant("GUILDINFO_EXP", GUILDINFO_EXP, false, false);
	script->set_constant("GUILDINFO_NEXT_EXP", GUILDINFO_NEXT_EXP, false, false);
	script->set_constant("GUILDINFO_SKILL_POINTS", GUILDINFO_SKILL_POINTS, false, false);
	script->set_constant("GUILDINFO_MASTER_NAME", GUILDINFO_MASTER_NAME, false, false);
	script->set_constant("GUILDINFO_MASTER_CID", GUILDINFO_MASTER_CID, false, false);

	script->constdb_comment("madogear types");
	script->set_constant("MADO_ROBOT", MADO_ROBOT, false, false);
	script->set_constant("MADO_SUITE", MADO_SUITE, false, false);

	script->constdb_comment("itemskill option flags");
	script->set_constant("ISF_NONE", ISF_NONE, false, false);
	script->set_constant("ISF_CHECKCONDITIONS", ISF_CHECKCONDITIONS, false, false);
	script->set_constant("ISF_INSTANTCAST", ISF_INSTANTCAST, false, false);
	script->set_constant("ISF_CASTONSELF", ISF_CASTONSELF, false, false);

	script->constdb_comment("Item Bound Types");
	script->set_constant("IBT_ANY", IBT_NONE, false, false); // for *checkbound()
	script->set_constant("IBT_ACCOUNT", IBT_ACCOUNT, false, false);
	script->set_constant("IBT_GUILD", IBT_GUILD, false, false);
	script->set_constant("IBT_PARTY", IBT_PARTY, false, false);
	script->set_constant("IBT_CHARACTER", IBT_CHARACTER, false, false);

	script->constdb_comment("Renewal");
#ifdef RENEWAL
	script->set_constant("RENEWAL", 1, false, false);
#else
	script->set_constant("RENEWAL", 0, false, false);
#endif
#ifdef RENEWAL_CAST
	script->set_constant("RENEWAL_CAST", 1, false, false);
#else
	script->set_constant("RENEWAL_CAST", 0, false, false);
#endif
#ifdef RENEWAL_DROP
	script->set_constant("RENEWAL_DROP", 1, false, false);
#else
	script->set_constant("RENEWAL_DROP", 0, false, false);
#endif
#ifdef RENEWAL_EXP
	script->set_constant("RENEWAL_EXP", 1, false, false);
#else
	script->set_constant("RENEWAL_EXP", 0, false, false);
#endif
#ifdef RENEWAL_LVDMG
	script->set_constant("RENEWAL_LVDMG", 1, false, false);
#else
	script->set_constant("RENEWAL_LVDMG", 0, false, false);
#endif
#ifdef RENEWAL_EDP
	script->set_constant("RENEWAL_EDP", 1, false, false);
#else
	script->set_constant("RENEWAL_EDP", 0, false, false);
#endif
#ifdef RENEWAL_ASPD
	script->set_constant("RENEWAL_ASPD", 1, false, false);
#else
	script->set_constant("RENEWAL_ASPD", 0, false, false);
#endif
	script->constdb_comment(NULL);
}

/**
 * a mapindex_name2id wrapper meant to help with invalid name handling
 **/
static unsigned short script_mapindexname2id(struct script_state *st, const char *name)
{
	unsigned short index;

	if( !(index=mapindex->name2id(name)) ) {
		script->reportsrc(st);
		return 0;
	}
	return index;
}

void script_defaults(void)
{
	// aegis->athena slot position conversion table
	unsigned int equip[SCRIPT_EQUIP_TABLE_SIZE] = {EQP_HEAD_TOP,EQP_ARMOR,EQP_HAND_L,EQP_HAND_R,EQP_GARMENT,EQP_SHOES,EQP_ACC_L,EQP_ACC_R,EQP_HEAD_MID,EQP_HEAD_LOW,EQP_COSTUME_HEAD_LOW,EQP_COSTUME_HEAD_MID,EQP_COSTUME_HEAD_TOP,EQP_COSTUME_GARMENT,EQP_SHADOW_ARMOR, EQP_SHADOW_WEAPON, EQP_SHADOW_SHIELD, EQP_SHADOW_SHOES, EQP_SHADOW_ACC_R, EQP_SHADOW_ACC_L};

	script = &script_s;

	script->st_db = NULL;
	script->active_scripts = 0;
	script->next_id = 0;
	script->st_ers = NULL;
	script->stack_ers = NULL;
	script->array_ers = NULL;

	script->buildin = NULL;
	script->buildin_count = 0;

	script->str_data = NULL;
	script->str_data_size = 0;
	script->str_num = LABEL_START;
	script->str_buf = NULL;
	script->str_size = 0;
	script->str_pos = 0;
	memset(script->str_hash, 0, sizeof(script->str_hash));

	script->word_buf = NULL;
	script->word_size = 0;

	script->current_item_id = 0;

	script->labels = NULL;
	script->label_count = 0;
	script->labels_size = 0;

	VECTOR_INIT(script->buf);
	VECTOR_INIT(script->translation_buf);

	script->parse_options = 0;
	script->buildin_set_ref = 0;
	script->buildin_callsub_ref = 0;
	script->buildin_callfunc_ref = 0;
	script->buildin_getelementofarray_ref = 0;

	memset(script->error_jump,0,sizeof(script->error_jump));
	script->error_msg = NULL;
	script->error_pos = NULL;
	script->error_report = 0;
	script->parser_current_src = NULL;
	script->parser_current_file = NULL;
	script->parser_current_line = 0;

	memset(&script->syntax,0,sizeof(script->syntax));

	script->parse_syntax_for_flag = 0;

	memcpy(script->equip, &equip, sizeof(script->equip));

	memset(&script->config, 0, sizeof(script->config));

	script->autobonus_db = NULL;
	script->userfunc_db = NULL;

	script->potion_flag = script->potion_hp = script->potion_per_hp =
	script->potion_sp = script->potion_per_sp = script->potion_target = 0;

	script->generic_ui_array = NULL;
	script->generic_ui_array_size = 0;
	/* */
	script->init = do_init_script;
	script->final = do_final_script;
	script->reload = script_reload;

	/* parse */
	script->parse = parse_script;
	script->add_builtin = script_add_builtin;
	script->parse_builtin = script_parse_builtin;
	script->skip_space = script_skip_space;
	script->error = script_error;
	script->warning = script_warning;
	script->parse_subexpr = script_parse_subexpr;

	script->addScript = script_hp_add;
	script->conv_num = conv_num;
	script->conv_str = conv_str;
	script->rid2sd = script_rid2sd;
	script->id2sd = script_id2sd;
	script->charid2sd = script_charid2sd;
	script->nick2sd = script_nick2sd;
	script->detach_rid = script_detach_rid;
	script->push_val = push_val;
	script->get_val = get_val;
	script->get_val2 = get_val2;
	script->get_val_ref_str = get_val_npcscope_str;
	script->get_val_pc_ref_str = get_val_pc_ref_str;
	script->get_val_scope_str = get_val_npcscope_str;
	script->get_val_npc_str = get_val_npcscope_str;
	script->get_val_instance_str = get_val_instance_str;
	script->get_val_ref_num = get_val_npcscope_num;
	script->get_val_pc_ref_num = get_val_pc_ref_num;
	script->get_val_scope_num = get_val_npcscope_num;
	script->get_val_npc_num = get_val_npcscope_num;
	script->get_val_instance_num = get_val_instance_num;
	script->push_str = push_str;
	script->push_conststr = push_conststr;
	script->push_copy = push_copy;
	script->pop_stack = pop_stack;
	script->set_constant = script_set_constant;
	script->set_constant2 = script_set_constant2;
	script->get_constant = script_get_constant;
	script->label_add = script_label_add;
	script->run = run_script;
	script->run_npc = run_script;
	script->run_pet = run_script;
	script->run_main = run_script_main;
	script->run_timer = run_script_timer;
	script->set_var = set_var;
	script->stop_instances = script_stop_instances;
	script->free_code = script_free_code;
	script->free_vars = script_free_vars;
	script->alloc_state = script_alloc_state;
	script->free_state = script_free_state;
	script->add_pending_ref = script_add_pending_ref;
	script->run_autobonus = script_run_autobonus;
	script->cleararray_pc = script_cleararray_pc;
	script->setarray_pc = script_setarray_pc;
	script->config_read = script_config_read;
	script->add_str = script_add_str;
	script->add_variable = script_add_variable;
	script->get_str = script_get_str;
	script->search_str = script_search_str;
	script->setd_sub = setd_sub;
	script->attach_state = script_attach_state;
	script->sprintf_helper = script_sprintf_helper;

	script->queue = script_hqueue_get;
	script->queue_add = script_hqueue_add;
	script->queue_del = script_hqueue_del;
	script->queue_remove = script_hqueue_remove;
	script->queue_create = script_hqueue_create;
	script->queue_clear = script_hqueue_clear;

	script->parse_curly_close = parse_curly_close;
	script->parse_syntax_close = parse_syntax_close;
	script->parse_syntax_close_sub = parse_syntax_close_sub;
	script->parse_syntax = parse_syntax;
	script->get_com = get_com;
	script->get_num = get_num;
	script->op2name = script_op2name;
	script->reportsrc = script_reportsrc;
	script->reportdata = script_reportdata;
	script->reportfunc = script_reportfunc;
	script->disp_warning_message = disp_warning_message;
	script->check_event = check_event;
	script->calc_hash = calc_hash;
	script->addb = add_scriptb;
	script->addc = add_scriptc;
	script->addi = add_scripti;
	script->addl = add_scriptl;
	script->set_label = set_label;
	script->skip_word = skip_word;
	script->add_word = add_word;
	script->parse_callfunc = parse_callfunc;
	script->parse_nextline = parse_nextline;
	script->parse_variable = parse_variable;
	script->parse_simpleexpr = parse_simpleexpr;
	script->parse_simpleexpr_paren = parse_simpleexpr_paren;
	script->parse_simpleexpr_number = parse_simpleexpr_number;
	script->parse_simpleexpr_string = parse_simpleexpr_string;
	script->parse_simpleexpr_name = parse_simpleexpr_name;
	script->add_translatable_string = script_add_translatable_string;
	script->parse_expr = parse_expr;
	script->parse_line = parse_line;
	script->read_constdb = read_constdb;
	script->constdb_comment = script_constdb_comment;
	script->load_parameters = script_load_parameters;
	script->print_line = script_print_line;
	script->errorwarning_sub = script_errorwarning_sub;
	script->set_reg = set_reg;
	script->set_reg_ref_str = set_reg_npcscope_str;
	script->set_reg_pc_ref_str = set_reg_pc_ref_str;
	script->set_reg_scope_str = set_reg_npcscope_str;
	script->set_reg_npc_str = set_reg_npcscope_str;
	script->set_reg_instance_str = set_reg_instance_str;
	script->set_reg_ref_num = set_reg_npcscope_num;
	script->set_reg_pc_ref_num = set_reg_pc_ref_num;
	script->set_reg_scope_num = set_reg_npcscope_num;
	script->set_reg_npc_num = set_reg_npcscope_num;
	script->set_reg_instance_num = set_reg_instance_num;

	script->stack_expand = stack_expand;
	script->push_retinfo = push_retinfo;
	script->op_3 = op_3;
	script->op_2str = op_2str;
	script->op_2num = op_2num;
	script->op_2 = op_2;
	script->op_1 = op_1;
	script->check_buildin_argtype = script_check_buildin_argtype;
	script->detach_state = script_detach_state;
	script->db_free_code_sub = db_script_free_code_sub;
	script->add_autobonus = script_add_autobonus;
	script->menu_countoptions = menu_countoptions;
	script->buildin_recovery_sub = buildin_recovery_sub;
	script->buildin_recovery_pc_sub = buildin_recovery_pc_sub;
	script->buildin_recovery_bl_sub = buildin_recovery_bl_sub;
	script->buildin_areawarp_sub = buildin_areawarp_sub;
	script->buildin_areapercentheal_sub = buildin_areapercentheal_sub;
	script->buildin_delitem_delete = buildin_delitem_delete;
	script->buildin_delitem_search = buildin_delitem_search;
	script->buildin_killmonster_sub_strip = buildin_killmonster_sub_strip;
	script->buildin_killmonster_sub = buildin_killmonster_sub;
	script->buildin_killmonsterall_sub_strip = buildin_killmonsterall_sub_strip;
	script->buildin_killmonsterall_sub = buildin_killmonsterall_sub;
	script->buildin_announce_sub = buildin_announce_sub;
	script->buildin_getareausers_sub = buildin_getareausers_sub;
	script->buildin_getareadropitem_sub = buildin_getareadropitem_sub;
	script->mapflag_pvp_sub = script_mapflag_pvp_sub;
	script->buildin_pvpoff_sub = buildin_pvpoff_sub;
	script->buildin_maprespawnguildid_sub_pc = buildin_maprespawnguildid_sub_pc;
	script->buildin_maprespawnguildid_sub_mob = buildin_maprespawnguildid_sub_mob;
	script->buildin_mobcount_sub = buildin_mobcount_sub;
	script->playbgm_sub = playbgm_sub;
	script->playbgm_foreachpc_sub = playbgm_foreachpc_sub;
	script->soundeffect_sub = soundeffect_sub;
	script->buildin_query_sql_sub = buildin_query_sql_sub;
	script->buildin_instance_warpall_sub = buildin_instance_warpall_sub;
	script->buildin_mobuseskill_sub = buildin_mobuseskill_sub;
	script->cleanfloor_sub = script_cleanfloor_sub;
	script->run_func = run_func;
	script->getfuncname = script_getfuncname;

	/* script_config base */
	script->config.warn_func_mismatch_argtypes = true;
	script->config.warn_func_mismatch_paramnum = true;
	script->config.check_cmdcount = 65535;
	script->config.check_gotocount = 2048;
	script->config.input_min_value = 0;
	script->config.input_max_value = INT_MAX;
	script->config.die_event_name = "OnPCDieEvent";
	script->config.kill_pc_event_name = "OnPCKillEvent";
	script->config.kill_mob_event_name = "OnNPCKillEvent";
	script->config.login_event_name = "OnPCLoginEvent";
	script->config.logout_event_name = "OnPCLogoutEvent";
	script->config.loadmap_event_name = "OnPCLoadMapEvent";
	script->config.baselvup_event_name = "OnPCBaseLvUpEvent";
	script->config.joblvup_event_name = "OnPCJobLvUpEvent";
	script->config.ontouch_name = "OnTouch_";  //ontouch_name (runs on first visible char to enter area, picks another char if the first char leaves)
	script->config.ontouch2_name = "OnTouch";  //ontouch2_name (run whenever a char walks into the OnTouch area)
	script->config.onuntouch_name = "OnUnTouch";  //onuntouch_name (run whenever a char walks from the OnTouch area)

	// for ENABLE_CASE_CHECK
	script->calc_hash_ci = calc_hash_ci;
	script->local_casecheck.add_str = script_local_casecheck_add_str;
	script->local_casecheck.clear = script_local_casecheck_clear;
	script->local_casecheck.str_data = NULL;
	script->local_casecheck.str_data_size = 0;
	script->local_casecheck.str_num = 1;
	script->local_casecheck.str_buf = NULL;
	script->local_casecheck.str_size = 0;
	script->local_casecheck.str_pos = 0;
	memset(script->local_casecheck.str_hash, 0, sizeof(script->local_casecheck.str_hash));
	script->global_casecheck.add_str = script_global_casecheck_add_str;
	script->global_casecheck.clear = script_global_casecheck_clear;
	script->global_casecheck.str_data = NULL;
	script->global_casecheck.str_data_size = 0;
	script->global_casecheck.str_num = 1;
	script->global_casecheck.str_buf = NULL;
	script->global_casecheck.str_size = 0;
	script->global_casecheck.str_pos = 0;
	memset(script->global_casecheck.str_hash, 0, sizeof(script->global_casecheck.str_hash));
	// end ENABLE_CASE_CHECK

	/**
	 * Array Handling
	 **/
	script->array_src = script_array_src;
	script->array_update = script_array_update;
	script->array_add_member = script_array_add_member;
	script->array_remove_member = script_array_remove_member;
	script->array_delete = script_array_delete;
	script->array_size = script_array_size;
	script->array_free_db = script_free_array_db;
	script->array_highest_key = script_array_highest_key;
	script->array_ensure_zero = script_array_ensure_zero;
	/* */
	script->reg_destroy_single = script_reg_destroy_single;
	script->reg_destroy = script_reg_destroy;
	/* */
	script->generic_ui_array_expand = script_generic_ui_array_expand;
	script->array_cpy_list = script_array_cpy_list;
	/* */
	script->hardcoded_constants = script_hardcoded_constants;
	script->mapindexname2id = script_mapindexname2id;
	script->string_dup = script_string_dup;
	script->load_translations = script_load_translations;
	script->load_translation_addstring = script_load_translation_addstring;
	script->load_translation_file = script_load_translation_file;
	script->load_translation = script_load_translation;
	script->translation_db_destroyer = script_translation_db_destroyer;
	script->clear_translations = script_clear_translations;
	script->parse_cleanup_timer = script_parse_cleanup_timer;
	script->add_language = script_add_language;
	script->get_translation_dir_name = script_get_translation_dir_name;
	script->parser_clean_leftovers = script_parser_clean_leftovers;

	script->run_use_script = script_run_use_script;
	script->run_item_equip_script = script_run_item_equip_script;
	script->run_item_unequip_script = script_run_item_unequip_script;
	script->run_item_rental_start_script = script_run_item_rental_start_script;
	script->run_item_rental_end_script = script_run_item_rental_end_script;
	script->run_item_lapineddukddak_script = script_run_item_lapineddukddak_script;

	script->sellitemcurrency_add = script_sellitemcurrency_add;
}
