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

#include "config/core.h" // RENEWAL, RENEWAL_ASPD, RENEWAL_CAST, RENEWAL_DROP, RENEWAL_EDP, RENEWAL_EXP, RENEWAL_LVDMG, SCRIPT_CALLFUNC_CHECK, SECURE_NPCTIMEOUT, SECURE_NPCTIMEOUT_INTERVAL
#include "script.h"

#include "map/atcommand.h"
#include "map/battle.h"
#include "map/battleground.h"
#include "map/channel.h"
#include "map/chat.h"
#include "map/chrif.h"
#include "map/clif.h"
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
#include "map/mob.h"
#include "map/npc.h"
#include "map/party.h"
#include "map/path.h"
#include "map/pc.h"
#include "map/pet.h"
#include "map/pet.h"
#include "map/quest.h"
#include "map/skill.h"
#include "map/status.h"
#include "map/status.h"
#include "map/storage.h"
#include "map/unit.h"
#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/memmgr.h"
#include "common/md5calc.h"
#include "common/mmo.h" // NEW_CARTS
#include "common/nullpo.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/socket.h" // usage: getcharip
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

struct script_interface script_s;
struct script_interface *script;

static inline int GETVALUE(const unsigned char* buf, int i) {
	return (int)MakeDWord(MakeWord(buf[i], buf[i+1]), MakeWord(buf[i+2], 0));
}
static inline void SETVALUE(unsigned char* buf, int i, int n) {
	buf[i]   = GetByte(n, 0);
	buf[i+1] = GetByte(n, 1);
	buf[i+2] = GetByte(n, 2);
}

static inline void script_string_buf_ensure(struct script_string_buf *buf, size_t ensure) {
	if( buf->pos+ensure >= buf->size ) {
		do {
			buf->size += 512;
		} while ( buf->pos+ensure >= buf->size );
		RECREATE(buf->ptr, char, buf->size);
	}
}

static inline void script_string_buf_addb(struct script_string_buf *buf,uint8 b) {
	if( buf->pos+1 >= buf->size ) {
		buf->size += 512;
		RECREATE(buf->ptr, char, buf->size);
	}
	buf->ptr[buf->pos++] = b;
}

static inline void script_string_buf_destroy(struct script_string_buf *buf) {
	if( buf->ptr )
		aFree(buf->ptr);
	memset(buf,0,sizeof(struct script_string_buf));
}

const char* script_op2name(int op) {
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
static void script_dump_stack(struct script_state* st)
{
	int i;
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
void script_reportsrc(struct script_state *st) {
	struct block_list* bl;

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
				ShowDebug("Source (Non-NPC type %u): name %s at %s (%d,%d)\n", bl->type, status->get_name(bl), map->list[bl->m].name, bl->x, bl->y);
			else
				ShowDebug("Source (Non-NPC type %u): name %s (invisible/not on a map)\n", bl->type, status->get_name(bl));
			break;
	}
}

/// Reports on the console information about the script data.
void script_reportdata(struct script_data* data)
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
void script_reportfunc(struct script_state* st)
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
static void disp_error_message2(const char *mes,const char *pos,int report) analyzer_noreturn;
static void disp_error_message2(const char *mes,const char *pos,int report) {
	script->error_msg = aStrdup(mes);
	script->error_pos = pos;
	script->error_report = report;
	longjmp( script->error_jump, 1 );
}
#define disp_error_message(mes,pos) (disp_error_message2((mes),(pos),1))

void disp_warning_message(const char *mes, const char *pos) {
	script->warning(script->parser_current_src,script->parser_current_file,script->parser_current_line,mes,pos);
}

/// Checks event parameter validity
void check_event(struct script_state *st, const char *evt)
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
unsigned int calc_hash(const char* p) {
	unsigned int h;

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
unsigned int calc_hash_ci(const char* p) {
	unsigned int h = 0;
#ifdef ENABLE_CASE_CHECK

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
const char* script_get_str(int id)
{
	Assert_retr(NULL, id >= LABEL_START && id < script->str_size);
	return script->str_buf+script->str_data[id].str;
}

/// Returns the uid of the string, or -1.
int script_search_str(const char* p)
{
	int i;

	for( i = script->str_hash[script->calc_hash(p)]; i != 0; i = script->str_data[i].next ) {
		if( strcmp(script->get_str(i),p) == 0 ) {
			return i;
		}
	}

	return -1;
}

void script_casecheck_clear_sub(struct casecheck_data *ccd) {
#ifdef ENABLE_CASE_CHECK
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

void script_global_casecheck_clear(void) {
	script_casecheck_clear_sub(&script->global_casecheck);
}

void script_local_casecheck_clear(void) {
	script_casecheck_clear_sub(&script->local_casecheck);
}

const char *script_casecheck_add_str_sub(struct casecheck_data *ccd, const char *p)
{
#ifdef ENABLE_CASE_CHECK
	int len;
	int h = script->calc_hash_ci(p);
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

const char *script_global_casecheck_add_str(const char *p) {
	return script_casecheck_add_str_sub(&script->global_casecheck, p);
}

const char *script_local_casecheck_add_str(const char *p) {
	return script_casecheck_add_str_sub(&script->local_casecheck, p);
}

/// Stores a copy of the string and returns its id.
/// If an identical string is already present, returns its id instead.
int script_add_str(const char* p)
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

/// Appends 1 byte to the script buffer.
void add_scriptb(int a)
{
	if( script->pos+1 >= script->size )
	{
		script->size += SCRIPT_BLOCK_SIZE;
		RECREATE(script->buf,unsigned char,script->size);
	}
	script->buf[script->pos++] = (uint8)(a);
}

/// Appends a c_op value to the script buffer.
/// The value is variable-length encoded into 8-bit blocks.
/// The encoding scheme is ( 01?????? )* 00??????, LSB first.
/// All blocks but the last hold 7 bits of data, topmost bit is always 1 (carries).
void add_scriptc(int a)
{
	while( a >= 0x40 )
	{
		script->addb((a&0x3f)|0x40);
		a = (a - 0x40) >> 6;
	}

	script->addb(a);
}

/// Appends an integer value to the script buffer.
/// The value is variable-length encoded into 8-bit blocks.
/// The encoding scheme is ( 11?????? )* 10??????, LSB first.
/// All blocks but the last hold 7 bits of data, topmost bit is always 1 (carries).
void add_scripti(int a)
{
	while( a >= 0x40 )
	{
		script->addb((a&0x3f)|0xc0);
		a = (a - 0x40) >> 6;
	}
	script->addb(a|0x80);
}

/// Appends a script->str_data object (label/function/variable/integer) to the script buffer.

///
/// @param l The id of the script->str_data entry
// Maximum up to 16M
void add_scriptl(int l)
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
			script->str_data[l].backpatch = script->pos;
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
void set_label(int l,int pos, const char* script_pos)
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
		int next = GETVALUE(script->buf,i);
		script->buf[i-1]=(script->str_data[l].type == C_USERFUNC ? C_USERFUNC_POS : C_POS);
		SETVALUE(script->buf,i,pos);
		i = next;
	}
}

/// Skips spaces and/or comments.
const char* script_skip_space(const char* p)
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
const char* skip_word(const char* p) {
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

	while( ISALNUM(*p) || *p == '_' || *p == '\'' )
		++p;

	// postfix
	if( *p == '$' )// string
		p++;

	return p;
}
/// Adds a word to script->str_data.
/// @see skip_word
/// @see script->add_str
int add_word(const char* p) {
	size_t len;
	int i;

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
static
const char* parse_callfunc(const char* p, int require_paren, int is_custom)
{
	const char *p2;
	char *arg = NULL;
	char null_arg = '\0';
	int func;
	bool nested_call = false, macro = false;

	// is need add check for arg null pointer below?
	func = script->add_word(p);
	if( script->str_data[func].type == C_FUNC ) {
		/** only when unset (-1), valid values are >= 0 **/
		if( script->syntax.last_func == -1 )
			script->syntax.last_func = script->str_data[func].val;
		else { //Nested function call
			script->syntax.nested_call++;
			nested_call = true;

			if( script->str_data[func].val == script->buildin_lang_macro_offset ) {
				script->syntax.lang_macro_active = true;
				macro = true;
			}
		}

		if( !macro ) {
			// buildin function
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

		if( script->str_data[func].val == script->buildin_lang_macro_offset )
			script->syntax.lang_macro_active = false;
	}

	if( nested_call )
		script->syntax.nested_call--;

	if( !script->syntax.nested_call )
		script->syntax.last_func = -1;

	if( !macro )
		script->addc(C_FUNC);
	return p;
}

/// Processes end of logical script line.
/// @param first When true, only fix up scheduling data is initialized
/// @param p Script position for error reporting in set_label
void parse_nextline(bool first, const char* p)
{
	if( !first )
	{
		script->addc(C_EOL);  // mark end of line for stack cleanup
		script->set_label(LABEL_NEXTLINE, script->pos, p);  // fix up '-' labels
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
void parse_variable_sub_push(int word, const char *p2)
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
const char* parse_variable(const char* p)
{
	int word;
	c_op type = C_NOP;
	const char *p2 = NULL;
	const char *var = p;

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
bool is_number(const char *p) {
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
 *
 **/
int script_string_dup(char *str) {
	size_t len = strlen(str);
	int pos = script->string_list_pos;

	while( pos+len+1 >= script->string_list_size ) {
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
const char* parse_simpleexpr(const char *p)
{
	p=script->skip_space(p);

	if(*p==';' || *p==',')
		disp_error_message("parse_simpleexpr: unexpected end of expression",p);
	if(*p=='(') {
		int i = script->syntax.curly_count-1;
		if (i >= 0 && script->syntax.curly[i].type == TYPE_ARGLIST)
			++script->syntax.curly[i].count;
		p=script->parse_subexpr(p+1,-1);
		p=script->skip_space(p);
		if( (i=script->syntax.curly_count-1) >= 0 && script->syntax.curly[i].type == TYPE_ARGLIST
		  && script->syntax.curly[i].flag == ARGLIST_UNDEFINED && --script->syntax.curly[i].count == 0
		) {
			if( *p == ',' ) {
				script->syntax.curly[i].flag = ARGLIST_PAREN;
				return p;
			} else {
				script->syntax.curly[i].flag = ARGLIST_NO_PAREN;
			}
		}
		if( *p != ')' )
			disp_error_message("parse_simpleexpr: unmatched ')'",p);
		++p;
	} else if(is_number(p)) {
		char *np;
		long long lli;
		while(*p == '0' && ISDIGIT(p[1])) p++; // Skip leading zeros, we don't support octal literals
		lli=strtoll(p,&np,0);
		if( lli < INT_MIN ) {
			lli = INT_MIN;
			script->disp_warning_message("parse_simpleexpr: underflow detected, capping value to INT_MIN",p);
		} else if( lli > INT_MAX ) {
			lli = INT_MAX;
			script->disp_warning_message("parse_simpleexpr: overflow detected, capping value to INT_MAX",p);
		}
		script->addi((int)lli); // Cast is safe, as it's already been checked for overflows
		p=np;
	} else if(*p=='"') {
		struct string_translation *st = NULL;
		const char *start_point = p;
		bool duplicate = true;
		struct script_string_buf *sbuf = &script->parse_simpleexpr_str;

		do {
			p++;
			while( *p && *p != '"' ) {
				if( (unsigned char)p[-1] <= 0x7e && *p == '\\' ) {
					char buf[8];
					size_t len = sv->skip_escaped_c(p) - p;
					size_t n = sv->unescape_c(buf, p, len);
					if( n != 1 )
						ShowDebug("parse_simpleexpr: unexpected length %d after unescape (\"%.*s\" -> %.*s)\n", (int)n, (int)len, p, (int)n, buf);
					p += len;
					script_string_buf_addb(sbuf, *buf);
					continue;
				} else if( *p == '\n' ) {
					disp_error_message("parse_simpleexpr: unexpected newline @ string",p);
				}
				script_string_buf_addb(sbuf, *p++);
			}
			if(!*p)
				disp_error_message("parse_simpleexpr: unexpected end of file @ string",p);
			p++; //'"'
			p = script->skip_space(p);
		} while( *p && *p == '"' );

		script_string_buf_addb(sbuf, 0);

		if (!(script->syntax.translation_db && (st = strdb_get(script->syntax.translation_db, sbuf->ptr)) != NULL)) {
			script->addc(C_STR);

			if( script->pos+sbuf->pos >= script->size ) {
				do {
					script->size += SCRIPT_BLOCK_SIZE;
				} while( script->pos+sbuf->pos >= script->size );
				RECREATE(script->buf,unsigned char,script->size);
			}

			memcpy(script->buf+script->pos, sbuf->ptr, sbuf->pos);
			script->pos += sbuf->pos;

		} else {
			int expand = sizeof(int) + sizeof(uint8);
			unsigned char j;
			unsigned int st_cursor = 0;

			script->addc(C_LSTR);

			expand += (sizeof(char*) + sizeof(uint8)) * st->translations;

			while( script->pos+expand >= script->size ) {
				script->size += SCRIPT_BLOCK_SIZE;
				RECREATE(script->buf,unsigned char,script->size);
			}

			*((int *)(&script->buf[script->pos])) = st->string_id;
			*((uint8 *)(&script->buf[script->pos + sizeof(int)])) = st->translations;

			script->pos += sizeof(int) + sizeof(uint8);

			for(j = 0; j < st->translations; j++) {
				*((uint8 *)(&script->buf[script->pos])) = RBUFB(st->buf, st_cursor);
				*((char **)(&script->buf[script->pos+sizeof(uint8)])) = &st->buf[st_cursor + sizeof(uint8)];
				script->pos += sizeof(char*) + sizeof(uint8);
				st_cursor += sizeof(uint8);
				while(st->buf[st_cursor++]);
				st_cursor += sizeof(uint8);
			}
		}

		/* When exporting we don't know what is a translation and what isn't */
		if( script->lang_export_fp && sbuf->pos > 1 ) {//sbuf->pos will always be at least 1 because of the '\0'
			if( !script->syntax.strings ) {
				script->syntax.strings = strdb_alloc(DB_OPT_DUP_KEY|DB_OPT_ALLOW_NULL_DATA, 0);
			}

			if( !strdb_exists(script->syntax.strings,sbuf->ptr) ) {
				strdb_put(script->syntax.strings, sbuf->ptr, NULL);
				duplicate = false;
			}
		}

		if( script->lang_export_fp && !duplicate &&
			( ( ( script->syntax.last_func == script->buildin_mes_offset ||
				 script->syntax.last_func == script->buildin_select_offset ) && !script->syntax.nested_call
				) || script->syntax.lang_macro_active ) ) {
			const char *line_start = start_point;
			const char *line_end = start_point;
			struct script_string_buf *lbuf = &script->lang_export_line_buf;
			struct script_string_buf *ubuf = &script->lang_export_unescaped_buf;
			size_t line_length, cursor;

			while( line_start > script->parser_current_src ) {
				if( *line_start != '\n' )
					line_start--;
				else
					break;
			}

			while( *line_end != '\n' && *line_end != '\0' )
				line_end++;

			line_length = (size_t)(line_end - line_start);
			if( line_length > 0 ) {
				script_string_buf_ensure(lbuf,line_length + 1);

				memcpy(lbuf->ptr, line_start, line_length);
				lbuf->pos = line_length;
				script_string_buf_addb(lbuf, 0);

				normalize_name(lbuf->ptr, "\r\n\t ");
			}

			for(cursor = 0; cursor < sbuf->pos; cursor++) {
				if( sbuf->ptr[cursor] == '"' )
					script_string_buf_addb(ubuf, '\\');
				script_string_buf_addb(ubuf, sbuf->ptr[cursor]);
			}
			script_string_buf_addb(ubuf, 0);

			fprintf(script->lang_export_fp, "#: %s\n"
					"# %s\n"
					"msgctxt \"%s\"\n"
					"msgid \"%s\"\n"
					"msgstr \"\"\n",
					script->parser_current_file ? script->parser_current_file : "Unknown File",
					lbuf->ptr,
					script->parser_current_npc_name ? script->parser_current_npc_name : "Unknown NPC",
					ubuf->ptr
			);
			lbuf->pos = 0;
			ubuf->pos = 0;
		}
		sbuf->pos = 0;
	} else {
		int l;
		const char* pv;

		// label , register , function etc
		if(script->skip_word(p)==p)
			disp_error_message("parse_simpleexpr: unexpected character",p);

		l=script->add_word(p);
		if( script->str_data[l].type == C_FUNC || script->str_data[l].type == C_USERFUNC || script->str_data[l].type == C_USERFUNC_POS) {
			return script->parse_callfunc(p,1,0);
#ifdef SCRIPT_CALLFUNC_CHECK
		} else {
			const char* name = script->get_str(l);
			if( strdb_get(script->userfunc_db,name) != NULL ) {
				return script->parse_callfunc(p,1,1);
			}
#endif
		}

		if( (pv = script->parse_variable(p)) ) {
			// successfully processed a variable assignment
			return pv;
		}

		if (script->str_data[l].type == C_INT && script->str_data[l].deprecated) {
			disp_warning_message("This constant is deprecated and it will be removed in a future version. Please see the script documentation and constants.conf for an alternative.\n", p);
		}

		p=script->skip_word(p);
		if( *p == '[' ) {
			// array(name[i] => getelementofarray(name,i) )
			script->addl(script->buildin_getelementofarray_ref);
			script->addc(C_ARG);
			script->addl(l);

			p=script->parse_subexpr(p+1,-1);
			p=script->skip_space(p);
			if( *p != ']' )
				disp_error_message("parse_simpleexpr: unmatched ']'",p);
			++p;
			script->addc(C_FUNC);
		} else {
			script->addl(l);
		}

	}

	return p;
}

/*==========================================
 * Analysis of the expression
 *------------------------------------------*/
const char* script_parse_subexpr(const char* p,int limit)
{
	int op,opl,len;

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
	|| (op=C_ADD,    opl=9, len=1,*p=='+')              // +
	|| (op=C_SUB,    opl=9, len=1,*p=='-')              // -
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
const char* parse_expr(const char *p)
{
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
const char* parse_line(const char* p)
{
	const char* p2;

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
const char* parse_curly_close(const char* p)
{
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
		script->set_label(l,script->pos, p);

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
		script->set_label(l,script->pos, p);
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
const char* parse_syntax(const char* p)
{
	const char *p2 = script->skip_word(p);

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
					script->set_label(l,script->pos, p);
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
					script->set_label(l,script->pos,p);
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
				script->set_label(l,script->pos,p);

				// Skip to the next link w/o condition
				sprintf(label, "goto __SW%x_%x;", (unsigned int)script->syntax.curly[pos].index, (unsigned int)script->syntax.curly[pos].count + 1);
				script->syntax.curly[script->syntax.curly_count++].type = TYPE_NULL;
				script->parse_line(label);
				script->syntax.curly_count--;

				// The default label
				sprintf(label, "__SW%x_DEF", (unsigned int)script->syntax.curly[pos].index);
				l=script->add_str(label);
				script->set_label(l,script->pos,p);

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
			script->set_label(l,script->pos,p);
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
			script->set_label(l,script->pos,p);

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
			script->set_label(l,script->pos,p);

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
			script->set_label(l,script->pos,p);
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
					script->set_label(l, script->pos, p);
					if( script->parse_options&SCRIPT_USE_LABEL_DB )
						script->label_add(l,script->pos);
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
			script->set_label(l,script->pos,p);

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

const char* parse_syntax_close(const char *p) {
	// If (...) for (...) hoge (); as to make sure closed closed once again
	int flag;

	do {
		p = script->parse_syntax_close_sub(p,&flag);
	} while(flag);
	return p;
}

// Close judgment if, for, while, of do
// flag == 1 : closed
// flag == 0 : not closed
const char* parse_syntax_close_sub(const char* p,int* flag)
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
		script->set_label(l,script->pos,p);

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
		script->set_label(l,script->pos,p);
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
			script->set_label(l,script->pos,p);
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
		script->set_label(l,script->pos,p);
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
		script->set_label(l,script->pos,p);
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
		script->set_label(l,script->pos,p);
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
		script->set_label(l,script->pos,p);
		script->syntax.curly_count--;
		return p;
	} else {
		*flag = 0;
		return p;
	}
}

/// Retrieves the value of a constant.
bool script_get_constant(const char* name, int* value)
{
	int n = script->search_str(name);

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
void script_set_constant(const char *name, int value, bool is_parameter, bool is_deprecated)
{
	int n = script->add_str(name);

	if( script->str_data[n].type == C_NOP ) {// new
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
void script_set_constant2(const char *name, int value, bool is_parameter, bool is_deprecated)
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
void read_constdb(void)
{
	struct config_t constants_conf;
	char filepath[256];
	struct config_setting_t *cdb;
	struct config_setting_t *t;
	int i = 0;

	sprintf(filepath, "%s/constants.conf", map->db_path);

	if (!libconfig->load_file(&constants_conf, filepath))
		return;

	if ((cdb = libconfig->setting_get_member(constants_conf.root, "constants_db")) == NULL) {
		ShowError("can't read %s\n", filepath);
		return;
	}

	while ((t = libconfig->setting_get_elem(cdb, i++))) {
		bool is_parameter = false;
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
			if (libconfig->setting_lookup_bool(t, "Parameter", &i32)) {
				if (i32 != 0)
					is_parameter = true;
			}
			if (libconfig->setting_lookup_bool(t, "Deprecated", &i32)) {
				if (i32 != 0)
					is_deprecated = true;
			}
		} else {
			value = libconfig->setting_get_int(t);
		}
		if (is_parameter)
			ShowWarning("read_constdb: Defining parameters in the constants configuration is deprecated and will no longer be possible in a future version. Parameters should be defined in source. (parameter = '%s')\n", name);
		script->set_constant(name, value, is_parameter, is_deprecated);
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
void script_constdb_comment(const char *comment)
{
	(void)comment;
}

void script_load_parameters(void)
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
		{"BaseThird", SP_BASETHIRD},
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
const char* script_print_line(StringBuf* buf, const char* p, const char* mark, int line)
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
void script_errorwarning_sub(StringBuf *buf, const char* src, const char* file, int start_line, const char* error_msg, const char* error_pos) {
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

void script_error(const char* src, const char* file, int start_line, const char* error_msg, const char* error_pos) {
	StringBuf buf;

	StrBuf->Init(&buf);
	StrBuf->AppendStr(&buf, "\a");

	script->errorwarning_sub(&buf, src, file, start_line, error_msg, error_pos);

	ShowError("%s", StrBuf->Value(&buf));
	StrBuf->Destroy(&buf);
}

void script_warning(const char* src, const char* file, int start_line, const char* error_msg, const char* error_pos) {
	StringBuf buf;

	StrBuf->Init(&buf);

	script->errorwarning_sub(&buf, src, file, start_line, error_msg, error_pos);

	ShowWarning("%s", StrBuf->Value(&buf));
	StrBuf->Destroy(&buf);
}

/*==========================================
 * Analysis of the script
 *------------------------------------------*/
struct script_code* parse_script(const char *src,const char *file,int line,int options, int *retval) {
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

	if( script->syntax.strings ) /* used only when generating translation file */
		db_destroy(script->syntax.strings);

	memset(&script->syntax,0,sizeof(script->syntax));
	script->syntax.last_func = -1;/* as valid values are >= 0 */
	if( script->parser_current_npc_name ) {
		if( !script->translation_db )
			script->load_translations();
		if( script->translation_db )
			script->syntax.translation_db = strdb_get(script->translation_db, script->parser_current_npc_name);
	}

	if( !script->buf ) {
		script->buf = (unsigned char *)aMalloc(SCRIPT_BLOCK_SIZE*sizeof(unsigned char));
		script->size = SCRIPT_BLOCK_SIZE;
	}
	script->pos=0;
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
		script->pos  = 0;
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
		if( *p == '\0' && !(options&SCRIPT_RETURN_EMPTY_SCRIPT) )
		{// empty script and can return NULL
			script->pos = 0;
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
		if( *p == '}' && !(options&SCRIPT_RETURN_EMPTY_SCRIPT) )
		{// empty script and can return NULL
			script->pos  = 0;
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
			script->set_label(i,script->pos,p);
			if( script->parse_options&SCRIPT_USE_LABEL_DB )
				script->label_add(i,script->pos);
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
				int next = GETVALUE(script->buf,j);
				SETVALUE(script->buf,j,i);
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
	for(i=0;i<script->pos;i++) {
		if((i&15)==0) ShowMessage("%04x : ",i);
		ShowMessage("%02x ",script->buf[i]);
		if((i&15)==15) ShowMessage("\n");
	}
	ShowMessage("\n");
#endif
#ifdef SCRIPT_DEBUG_DISASM
	i = 0;
	while(i < script->pos) {
		int j = i;
		c_op op = script->get_com(script->buf,&i);

		ShowMessage("%06x %s", i, script->op2name(op));
		j = i;
		switch(op) {
		case C_INT:
			ShowMessage(" %d", script->get_num(script->buf,&i));
			break;
		case C_POS:
			ShowMessage(" 0x%06x", *(int*)(script->buf+i)&0xffffff);
			i += 3;
			break;
		case C_NAME:
			j = (*(int*)(script->buf+i)&0xffffff);
			ShowMessage(" %s", ( j == 0xffffff ) ? "?? unknown ??" : script->get_str(j));
			i += 3;
			break;
		case C_STR:
			j = (int)strlen((char*)script->buf + i);
			ShowMessage(" %s", script->buf + i);
			i += j+1;
			break;
		}
		ShowMessage(CL_CLL"\n");
	}
#endif

	CREATE(code,struct script_code,1);
	code->script_buf = (unsigned char *)aMalloc(script->pos*sizeof(unsigned char));
	memcpy(code->script_buf, script->buf, script->pos);
	code->script_size = script->pos;
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
struct map_session_data *script_rid2sd(struct script_state *st)
{
	struct map_session_data *sd;
	if( !( sd = map->id2sd(st->rid) ) ) {
		ShowError("script_rid2sd: fatal error ! player not attached!\n");
		script->reportfunc(st);
		script->reportsrc(st);
		st->state = END;
	}
	return sd;
}

struct map_session_data *script_id2sd(struct script_state *st, int account_id)
{
	struct map_session_data *sd;
	if ((sd = map->id2sd(account_id)) == NULL) {
		ShowWarning("script_id2sd: Player with account ID '%d' not found!\n", account_id);
		script->reportfunc(st);
		script->reportsrc(st);
	}
	return sd;
}

struct map_session_data *script_charid2sd(struct script_state *st, int char_id)
{
	struct map_session_data *sd;
	if ((sd = map->charid2sd(char_id)) == NULL) {
		ShowWarning("script_charid2sd: Player with char ID '%d' not found!\n", char_id);
		script->reportfunc(st);
		script->reportsrc(st);
	}
	return sd;
}

struct map_session_data *script_nick2sd(struct script_state *st, const char *name)
{
	struct map_session_data *sd;
	if ((sd = map->nick2sd(name)) == NULL) {
		ShowWarning("script_nick2sd: Player name '%s' not found!\n", name);
		script->reportfunc(st);
		script->reportsrc(st);
	}
	return sd;
}

char *get_val_npcscope_str(struct script_state* st, struct reg_db *n, struct script_data* data) {
	if (n)
		return (char*)i64db_get(n->vars, reference_getuid(data));
	else
		return NULL;
}

char *get_val_instance_str(struct script_state* st, const char* name, struct script_data* data) {
	if (st->instance_id >= 0) {
		return (char*)i64db_get(instance->list[st->instance_id].regs.vars, reference_getuid(data));
	} else {
		ShowWarning("script_get_val: cannot access instance variable '%s', defaulting to \"\"\n", name);
		return NULL;
	}
}

int get_val_npcscope_num(struct script_state* st, struct reg_db *n, struct script_data* data) {
	if (n)
		return (int)i64db_iget(n->vars, reference_getuid(data));
	else
		return 0;
}

int get_val_instance_num(struct script_state* st, const char* name, struct script_data* data) {
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
struct script_data *get_val(struct script_state* st, struct script_data* data) {
	const char* name;
	char prefix;
	char postfix;
	struct map_session_data *sd = NULL;

	if( !data_isreference(data) )
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

	//##TODO use reference_tovariable(data) when it's confirmed that it works [FlavioJS]
	if( !reference_toconstant(data) && not_server_variable(prefix) ) {
		sd = script->rid2sd(st);
		if( sd == NULL ) {// needs player attached
			if( postfix == '$' ) {// string variable
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

	if( postfix == '$' ) {// string variable

		switch( prefix ) {
			case '@':
				data->u.str = pc->readregstr(sd, data->u.num);
				break;
			case '$':
				data->u.str = mapreg->readregstr(data->u.num);
				break;
			case '#':
				if( name[1] == '#' )
					data->u.str = pc_readaccountreg2str(sd, data->u.num);// global
				else
					data->u.str = pc_readaccountregstr(sd, data->u.num);// local
				break;
			case '.':
				if (data->ref)
					data->u.str = script->get_val_ref_str(st, data->ref, data);
				else if (name[1] == '@')
					data->u.str = script->get_val_scope_str(st, &st->stack->scope, data);
				else
					data->u.str = script->get_val_npc_str(st, &st->script->local, data);
				break;
			case '\'':
				data->u.str = script->get_val_instance_str(st, name, data);
				break;
			default:
				data->u.str = pc_readglobalreg_str(sd, data->u.num);
				break;
		}

		if( data->u.str == NULL || data->u.str[0] == '\0' ) {// empty string
			data->type = C_CONSTSTR;
			data->u.str = "";
		} else {// duplicate string
			data->type = C_STR;
			data->u.str = aStrdup(data->u.str);
		}

	} else {// integer variable

		data->type = C_INT;

		if( reference_toconstant(data) ) {
			data->u.num = reference_getconstant(data);
		} else if( reference_toparam(data) ) {
			data->u.num = pc->readparam(sd, reference_getparamtype(data));
		} else
			switch( prefix ) {
				case '@':
					data->u.num = pc->readreg(sd, data->u.num);
					break;
				case '$':
					data->u.num = mapreg->readreg(data->u.num);
					break;
				case '#':
					if( name[1] == '#' )
						data->u.num = pc_readaccountreg2(sd, data->u.num);// global
					else
						data->u.num = pc_readaccountreg(sd, data->u.num);// local
					break;
				case '.':
					if (data->ref)
						data->u.num = script->get_val_ref_num(st, data->ref, data);
					else if (name[1] == '@')
						data->u.num = script->get_val_scope_num(st, &st->stack->scope, data);
					else
						data->u.num = script->get_val_npc_num(st, &st->script->local, data);
					break;
				case '\'':
					data->u.num = script->get_val_instance_num(st, name, data);
					break;
				default:
					data->u.num = pc_readglobalreg(sd, data->u.num);
					break;
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
void* get_val2(struct script_state* st, int64 uid, struct reg_db *ref) {
	struct script_data* data;
	script->push_val(st->stack, C_NAME, uid, ref);
	data = script_getdatatop(st, -1);
	script->get_val(st, data);
	return (data->type == C_INT ? (void*)h64BPTRSIZE((int32)data->u.num) : (void*)h64BPTRSIZE(data->u.str)); // u.num is int32 because it comes from script->get_val
}
/**
 * Because, currently, array members with key 0 are indifferenciable from normal variables, we should ensure its actually in
 * Will be gone as soon as undefined var feature is implemented
 **/
void script_array_ensure_zero(struct script_state *st, struct map_session_data *sd, int64 uid, struct reg_db *ref) {
	const char *name = script->get_str(script_getvarid(uid));
	// is here st can be null pointer and st->rid is wrong?
	struct reg_db *src = script->array_src(st, sd ? sd : st->rid ? map->id2sd(st->rid) : NULL, name, ref);
	bool insert = false;

	if (sd && !st) {
		/* when sd comes, st isn't available */
		insert = true;
	} else {
		if( is_string_variable(name) ) {
			char* str = (char*)script->get_val2(st, uid, ref);
			if( str && *str )
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
unsigned int script_array_size(struct script_state *st, struct map_session_data *sd, const char *name, struct reg_db *ref) {
	struct script_array *sa = NULL;
	struct reg_db *src = script->array_src(st, sd, name, ref);

	if( src && src->arrays )
		sa = idb_get(src->arrays, script->search_str(name));

	return sa ? sa->size : 0;
}
/**
 * Returns array's highest key (for that awful getarraysize implementation that doesn't really gets the array size)
 **/
unsigned int script_array_highest_key(struct script_state *st, struct map_session_data *sd, const char *name, struct reg_db *ref) {
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
int script_free_array_db(DBKey key, DBData *data, va_list ap) {
	struct script_array *sa = DB->data2ptr(data);
	aFree(sa->members);
	ers_free(script->array_ers, sa);
	return 0;
}
/**
 * Clears script_array and removes it from script->array_db
 **/
void script_array_delete(struct reg_db *src, struct script_array *sa) {
	aFree(sa->members);
	idb_remove(src->arrays, sa->id);
	ers_free(script->array_ers, sa);
}
/**
 * Removes a member from a script_array list
 *
 * @param idx the index of the member in script_array struct list, not of the actual array member
 **/
void script_array_remove_member(struct reg_db *src, struct script_array *sa, unsigned int idx) {
	unsigned int i, cursor;

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
void script_array_add_member(struct script_array *sa, unsigned int idx) {
	RECREATE(sa->members, unsigned int, ++sa->size);

	sa->members[sa->size - 1] = idx;
}
/**
 * Obtains the source of the array database for this type and scenario
 * Initializes such database when not yet initialized.
 **/
struct reg_db *script_array_src(struct script_state *st, struct map_session_data *sd, const char *name, struct reg_db *ref) {
	struct reg_db *src = NULL;

	switch( name[0] ) {
		/* from player */
		default: /* char reg */
		case '@':/* temp char reg */
		case '#':/* account reg */
			src = &sd->regs;
			break;
		case '$':/* map reg */
			src = &mapreg->regs;
			break;
		case '.':/* npc/script */
			if( ref )
				src = ref;
			else
				src = (name[1] == '@') ? &st->stack->scope : &st->script->local;
			break;
		case '\'':/* instance */
			if( st->instance_id >= 0 ) {
				src = &instance->list[st->instance_id].regs;
			}
			break;
	}

	if( src ) {
		if( !src->arrays )
			src->arrays = idb_alloc(DB_OPT_BASE);
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
void script_array_update(struct reg_db *src, int64 num, bool empty) {
	struct script_array *sa = NULL;
	int id = script_getvarid(num);
	unsigned int index = script_getvaridx(num);

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

void set_reg_npcscope_str(struct script_state* st, struct reg_db *n, int64 num, const char* name, const char *str)
{
	if (n)
	{
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

void set_reg_npcscope_num(struct script_state* st, struct reg_db *n, int64 num, const char* name, int val)
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

void set_reg_instance_str(struct script_state* st, int64 num, const char* name, const char *str)
{
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

void set_reg_instance_num(struct script_state* st, int64 num, const char* name, int val)
{
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
int set_reg(struct script_state *st, struct map_session_data *sd, int64 num, const char *name, const void *value, struct reg_db *ref)
{
	char prefix = name[0];

	if (strlen(name) > SCRIPT_VARNAME_LENGTH) {
		ShowError("script:set_reg: variable name too long. '%s'\n", name);
		script->reportsrc(st);
		st->state = END;
		return 0;
	}

	if( is_string_variable(name) ) {// string variable
		const char *str = (const char*)value;

		switch (prefix) {
			case '@':
				pc->setregstr(sd, num, str);
				return 1;
			case '$':
				return mapreg->setregstr(num, str);
			case '#':
				return (name[1] == '#') ?
					pc_setaccountreg2str(sd, num, str) :
					pc_setaccountregstr(sd, num, str);
			case '.':
				if (ref)
					script->set_reg_ref_str(st, ref, num, name, str);
				else if (name[1] == '@')
					script->set_reg_scope_str(st, &st->stack->scope, num, name, str);
				else
					script->set_reg_npc_str(st, &st->script->local, num, name, str);
				return 1;
			case '\'':
				set_reg_instance_str(st, num, name, str);
				return 1;
			default:
				return pc_setglobalreg_str(sd, num, str);
		}
	} else {// integer variable
		// FIXME: This isn't safe, in 32bits systems we're converting a 64bit pointer
		// to a 32bit int, this will lead to overflows! [Panikon]
		int val = (int)h64BPTRSIZE(value);

		if(script->str_data[script_getvarid(num)].type == C_PARAM) {
			if( pc->setparam(sd, script->str_data[script_getvarid(num)].val, val) == 0 ) {
				if( st != NULL ) {
					ShowError("script:set_reg: failed to set param '%s' to %d.\n", name, val);
					script->reportsrc(st);
					// Instead of just stop the script execution we let the character close
					// the window if it was open.
					st->state = (sd->state.dialog) ? CLOSE : END;
					if( st->state == CLOSE )
						clif->scriptclose(sd, st->oid);
				}
				return 0;
			}
			return 1;
		}

		switch (prefix) {
			case '@':
				pc->setreg(sd, num, val);
				return 1;
			case '$':
				return mapreg->setreg(num, val);
			case '#':
				return (name[1] == '#') ?
					pc_setaccountreg2(sd, num, val) :
					pc_setaccountreg(sd, num, val);
			case '.':
				if (ref)
					script->set_reg_ref_num(st, ref, num, name, val);
				else if (name[1] == '@')
					script->set_reg_scope_num(st, &st->stack->scope, num, name, val);
				else
					script->set_reg_npc_num(st, &st->script->local, num, name, val);
				return 1;
			case '\'':
				set_reg_instance_num(st, num, name, val);
				return 1;
			default:
				return pc_setglobalreg(sd, num, val);
		}
	}
}

int set_var(struct map_session_data *sd, char *name, void *val)
{
	return script->set_reg(NULL, sd, reference_uid(script->add_str(name),0), name, val, NULL);
}

void setd_sub(struct script_state *st, struct map_session_data *sd, const char *varname, int elem, void *value, struct reg_db *ref)
{
	script->set_reg(st, sd, reference_uid(script->add_str(varname),elem), varname, value, ref);
}

/// Converts the data to a string
const char* conv_str(struct script_state* st, struct script_data* data)
{
	char* p;

	script->get_val(st, data);
	if( data_isstring(data) )
	{// nothing to convert
	}
	else if( data_isint(data) )
	{// int -> string
		CREATE(p, char, ITEM_NAME_LENGTH);
		snprintf(p, ITEM_NAME_LENGTH, "%"PRId64"", data->u.num);
		p[ITEM_NAME_LENGTH-1] = '\0';
		data->type = C_STR;
		data->u.str = p;
	}
	else if( data_isreference(data) )
	{// reference -> string
		//##TODO when does this happen (check script->get_val) [FlavioJS]
		data->type = C_CONSTSTR;
		data->u.str = reference_getname(data);
	}
	else
	{// unsupported data type
		ShowError("script:conv_str: cannot convert to string, defaulting to \"\"\n");
		script->reportdata(data);
		script->reportsrc(st);
		data->type = C_CONSTSTR;
		data->u.str = "";
	}
	return data->u.str;
}

/// Converts the data to an int
int conv_num(struct script_state* st, struct script_data* data) {
	char* p;
	long num;

	script->get_val(st, data);
	if( data_isint(data) )
	{// nothing to convert
	}
	else if( data_isstring(data) )
	{// string -> int
		// the result does not overflow or underflow, it is capped instead
		// ex: 999999999999 is capped to INT_MAX (2147483647)
		p = data->u.str;
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
		if( data->type == C_STR )
			aFree(p);
		data->type = C_INT;
		data->u.num = (int)num;
	}
#if 0
	// FIXME this function is being used to retrieve the position of labels and
	// probably other stuff [FlavioJS]
	else
	{// unsupported data type
		ShowError("script:conv_num: cannot convert to number, defaulting to 0\n");
		script->reportdata(data);
		script->reportsrc(st);
		data->type = C_INT;
		data->u.num = 0;
	}
#endif
	return (int)data->u.num;
}

//
// Stack operations
//

/// Increases the size of the stack
void stack_expand(struct script_stack* stack) {
	stack->sp_max += 64;
	stack->stack_data = (struct script_data*)aRealloc(stack->stack_data,
			stack->sp_max * sizeof(stack->stack_data[0]) );
	memset(stack->stack_data + (stack->sp_max - 64), 0,
			64 * sizeof(stack->stack_data[0]) );
}

/// Pushes a value into the stack (with reference)
struct script_data* push_val(struct script_stack* stack, enum c_op type, int64 val, struct reg_db *ref) {
	if( stack->sp >= stack->sp_max )
		script->stack_expand(stack);
	stack->stack_data[stack->sp].type  = type;
	stack->stack_data[stack->sp].u.num = val;
	stack->stack_data[stack->sp].ref   = ref;
	stack->sp++;
	return &stack->stack_data[stack->sp-1];
}

/// Pushes a string into the stack
struct script_data* push_str(struct script_stack* stack, enum c_op type, char* str)
{
	if( stack->sp >= stack->sp_max )
		script->stack_expand(stack);
	stack->stack_data[stack->sp].type  = type;
	stack->stack_data[stack->sp].u.str = str;
	stack->stack_data[stack->sp].ref   = NULL;
	stack->sp++;
	return &stack->stack_data[stack->sp-1];
}

/// Pushes a retinfo into the stack
struct script_data* push_retinfo(struct script_stack* stack, struct script_retinfo* ri, struct reg_db *ref) {
	if( stack->sp >= stack->sp_max )
		script->stack_expand(stack);
	stack->stack_data[stack->sp].type = C_RETINFO;
	stack->stack_data[stack->sp].u.ri = ri;
	stack->stack_data[stack->sp].ref  = ref;
	stack->sp++;
	return &stack->stack_data[stack->sp-1];
}

/// Pushes a copy of the target position into the stack
struct script_data* push_copy(struct script_stack* stack, int pos) {
	switch( stack->stack_data[pos].type ) {
		case C_CONSTSTR:
			return script->push_str(stack, C_CONSTSTR, stack->stack_data[pos].u.str);
			break;
		case C_STR:
			return script->push_str(stack, C_STR, aStrdup(stack->stack_data[pos].u.str));
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
void pop_stack(struct script_state* st, int start, int end) {
	struct script_stack* stack = st->stack;
	struct script_data* data;
	int i;

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
		if( data->type == C_STR )
			aFree(data->u.str);
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
void script_free_vars(struct DBMap* var_storage) {
	if( var_storage ) {
		// destroy the storage construct containing the variables
		db_destroy(var_storage);
	}
}

void script_free_code(struct script_code* code)
{
	nullpo_retv(code);

	if (code->instances)
		script->stop_instances(code);
	script->free_vars(code->local.vars);
	if (code->local.arrays)
		code->local.arrays->destroy(code->local.arrays,script->array_free_db);
	aFree(code->script_buf);
	aFree(code);
}

/// Creates a new script state.
///
/// @param script Script code
/// @param pos Position in the code
/// @param rid Who is running the script (attached player)
/// @param oid Where the code is being run (npc 'object')
/// @return Script state
struct script_state* script_alloc_state(struct script_code* rootscript, int pos, int rid, int oid) {
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
void script_free_state(struct script_state* st) {
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
void script_add_pending_ref(struct script_state *st, struct reg_db *ref) {
	RECREATE(st->pending_refs, struct reg_db*, ++st->pending_ref_count);
	st->pending_refs[st->pending_ref_count-1] = ref;
}

//
// Main execution unit
//
/*==========================================
 * Read command
 *------------------------------------------*/
c_op get_com(unsigned char *scriptbuf,int *pos)
{
	int i = 0, j = 0;

	if(scriptbuf[*pos]>=0x80) {
		return C_INT;
	}
	while(scriptbuf[*pos]>=0x40) {
		i=scriptbuf[(*pos)++]<<j;
		j+=6;
	}
	return (c_op)(i+(scriptbuf[(*pos)++]<<j));
}

/*==========================================
 *  Income figures
 *------------------------------------------*/
int get_num(unsigned char *scriptbuf,int *pos)
{
	int i,j;
	i=0; j=0;
	while(scriptbuf[*pos]>=0xc0) {
		i+=(scriptbuf[(*pos)++]&0x7f)<<j;
		j+=6;
	}
	return i+((scriptbuf[(*pos)++]&0x7f)<<j);
}

/// Ternary operators
/// test ? if_true : if_false
void op_3(struct script_state* st, int op)
{
	struct script_data* data;
	int flag = 0;

	data = script_getdatatop(st, -3);
	script->get_val(st, data);

	if( data_isstring(data) )
		flag = data->u.str[0];// "" -> false
	else if( data_isint(data) )
		flag = data->u.num == 0 ? 0 : 1;// 0 -> false
	else
	{
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
void op_2str(struct script_state* st, int op, const char* s1, const char* s2)
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
			int pcre_erroroffset, offsetcount, i;
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

			if( op == C_RE_EQ ) {
				for( i = 0; i < offsetcount; i++ ) {
					libpcre->get_substring(s1, offsets, offsetcount, i, &pcre_match);
					mapreg->setregstr(reference_uid(script->add_str("$@regexmatch$"), i), pcre_match);
					libpcre->free_substring(pcre_match);
				}
				mapreg->setreg(script->add_str("$@regexmatchcount"), i);
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
void op_2num(struct script_state* st, int op, int i1, int i2)
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
void op_2(struct script_state *st, int op)
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

		if (leftref.type != C_NOP)
		{
			if (left->type == C_STR) // don't free C_CONSTSTR
				aFree(left->u.str);
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
void op_1(struct script_state* st, int op)
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
void script_check_buildin_argtype(struct script_state* st, int func)
{
	int idx, invalid = 0;
	char* sf = script->buildin[script->str_data[func].val];

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
}

/// Executes a buildin command.
/// Stack: C_NAME(<command>) C_ARG <arg0> <arg1> ... <argN>
int run_func(struct script_state *st)
{
	struct script_data* data;
	int i,start_sp,end_sp,func;

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
		script->check_buildin_argtype(st, func);
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
void run_script(struct script_code *rootscript, int pos, int rid, int oid) {
	struct script_state *st;

	if( rootscript == NULL || pos < 0 )
		return;

	// TODO In jAthena, this function can take over the pending script in the player. [FlavioJS]
	//      It is unclear how that can be triggered, so it needs the be traced/checked in more detail.
	// NOTE At the time of this change, this function wasn't capable of taking over the script state because st->scriptroot was never set.
	st = script->alloc_state(rootscript, pos, rid, oid);

	script->run_main(st);
}

void script_stop_instances(struct script_code *code) {
	DBIterator *iter;
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
int run_script_timer(int tid, int64 tick, int id, intptr_t data) {
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
void script_detach_state(struct script_state* st, bool dequeue_event) {
	struct map_session_data* sd;

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
void script_attach_state(struct script_state* st) {
	struct map_session_data* sd;

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
void run_script_main(struct script_state *st) {
	int cmdcount = script->config.check_cmdcount;
	int gotocount = script->config.check_gotocount;
	struct map_session_data *sd;
	struct script_stack *stack = st->stack;
	struct npc_data *nd;

	script->attach_state(st);

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
		enum c_op c = script->get_com(st->script->script_buf,&st->pos);
		switch(c) {
			case C_EOL:
				if( stack->defsp > stack->sp )
					ShowError("script:run_script_main: unexpected stack position (defsp=%d sp=%d). please report this!!!\n", stack->defsp, stack->sp);
				else
					script->pop_stack(st, stack->defsp, stack->sp);// pop unused stack data. (unused return value)
				break;
			case C_INT:
				script->push_val(stack,C_INT,script->get_num(st->script->script_buf,&st->pos),NULL);
				break;
			case C_POS:
			case C_NAME:
				script->push_val(stack,c,GETVALUE(st->script->script_buf,st->pos),NULL);
				st->pos+=3;
				break;
			case C_ARG:
				script->push_val(stack,c,0,NULL);
				break;
			case C_STR:
				script->push_str(stack,C_CONSTSTR,(char*)(st->script->script_buf+st->pos));
				while(st->script->script_buf[st->pos++]);
				break;
			case C_LSTR:
			{
				int string_id = *((int *)(&st->script->script_buf[st->pos]));
				uint8 translations = *((uint8 *)(&st->script->script_buf[st->pos+sizeof(int)]));
				struct map_session_data *lsd = NULL;

				st->pos += sizeof(int) + sizeof(uint8);

				if( (!st->rid || !(lsd = map->id2sd(st->rid)) || !lsd->lang_id) && !map->default_lang_id )
					script->push_str(stack,C_CONSTSTR,script->string_list+string_id);
				else {
					uint8 k, wlang_id = lsd ? lsd->lang_id : map->default_lang_id;
					int offset = st->pos;

					for(k = 0; k < translations; k++) {
						uint8 lang_id = *(uint8 *)(&st->script->script_buf[offset]);
						offset += sizeof(uint8);
						if( lang_id == wlang_id )
							break;
						offset += sizeof(char*);
					}
					script->push_str(stack,C_CONSTSTR,
							( k == translations ) ? script->string_list+string_id : *(char**)(&st->script->script_buf[offset]) );
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

int script_config_read(char *cfgName) {
	int i;
	char line[1024],w1[1024],w2[1024];
	FILE *fp;

	if( !( fp = fopen(cfgName,"r") ) ) {
		ShowError("File not found: %s\n", cfgName);
		return 1;
	}
	while (fgets(line, sizeof(line), fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;
		i = sscanf(line,"%1023[^:]: %1023[^\r\n]", w1, w2);
		if(i!=2)
			continue;

		if(strcmpi(w1,"warn_func_mismatch_paramnum")==0) {
			script->config.warn_func_mismatch_paramnum = config_switch(w2);
		}
		else if(strcmpi(w1,"check_cmdcount")==0) {
			script->config.check_cmdcount = config_switch(w2);
		}
		else if(strcmpi(w1,"check_gotocount")==0) {
			script->config.check_gotocount = config_switch(w2);
		}
		else if(strcmpi(w1,"input_min_value")==0) {
			script->config.input_min_value = config_switch(w2);
		}
		else if(strcmpi(w1,"input_max_value")==0) {
			script->config.input_max_value = config_switch(w2);
		}
		else if(strcmpi(w1,"warn_func_mismatch_argtypes")==0) {
			script->config.warn_func_mismatch_argtypes = config_switch(w2);
		}
		else if(strcmpi(w1,"import")==0) {
			script->config_read(w2);
		}
		else if(HPM->parseConf(w1, w2, HPCT_SCRIPT)) {
			; // handled by plugin
		} else {
			ShowWarning("Unknown setting '%s' in file %s\n", w1, cfgName);
		}
	}
	fclose(fp);

	return 0;
}

/**
 * @see DBApply
 */
int db_script_free_code_sub(DBKey key, DBData *data, va_list ap)
{
	struct script_code *code = DB->data2ptr(data);
	if (code)
		script->free_code(code);
	return 0;
}

void script_run_autobonus(const char *autobonus, int id, int pos)
{
	struct script_code *scriptroot = (struct script_code *)strdb_get(script->autobonus_db, autobonus);

	if( scriptroot ) {
		status->current_equip_item_index = pos;
		script->run(scriptroot,0,id,0);
	}
}

void script_add_autobonus(const char *autobonus)
{
	if( strdb_get(script->autobonus_db, autobonus) == NULL ) {
		struct script_code *scriptroot = script->parse(autobonus, "autobonus", 0, 0, NULL);

		if( scriptroot )
			strdb_put(script->autobonus_db, autobonus, scriptroot);
	}
}

/// resets a temporary character array variable to given value
void script_cleararray_pc(struct map_session_data* sd, const char* varname, void* value) {
	struct script_array *sa = NULL;
	struct reg_db *src = NULL;
	unsigned int i, *list = NULL, size = 0;
	int key;

	key = script->add_str(varname);

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
void script_setarray_pc(struct map_session_data* sd, const char* varname, uint32 idx, void* value, int* refcache) {
	int key;

	if( idx >= SCRIPT_MAX_ARRAYSIZE ) {
		ShowError("script_setarray_pc: Variable '%s' has invalid index '%u' (char_id=%d).\n", varname, idx, sd->status.char_id);
		return;
	}

	key = ( refcache && refcache[0] ) ? refcache[0] : script->add_str(varname);

	script->set_reg(NULL,sd,reference_uid(key, idx),varname,value,NULL);

	if( refcache )
	{// save to avoid repeated script->add_str calls
		refcache[0] = key;
	}
}
/**
 * Clears persistent variables from memory
 **/
int script_reg_destroy(DBKey key, DBData *data, va_list ap) {
	struct script_reg_state *src;

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
void script_reg_destroy_single(struct map_session_data *sd, int64 reg, struct script_reg_state *data) {
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
unsigned int *script_array_cpy_list(struct script_array *sa) {
	if( sa->size > script->generic_ui_array_size )
		script->generic_ui_array_expand(sa->size);
	memcpy(script->generic_ui_array, sa->members, sizeof(unsigned int)*sa->size);
	return script->generic_ui_array;
}
void script_generic_ui_array_expand (unsigned int plus) {
	script->generic_ui_array_size += plus + 100;
	RECREATE(script->generic_ui_array, unsigned int, script->generic_ui_array_size);
}
/*==========================================
 * Destructor
 *------------------------------------------*/
void do_final_script(void) {
	int i;
	DBIterator *iter;
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

	if( script->lang_export_file )
		aFree(script->lang_export_file);
}

/**
 *
 **/
uint8 script_add_language(const char *name) {
	uint8 lang_id = script->max_lang_id;

	RECREATE(script->languages, char *, ++script->max_lang_id);
	script->languages[lang_id] = aStrdup(name);

	return lang_id;
}
/**
 * Goes thru db/translations.conf file
 **/
void script_load_translations(void) {
	struct config_t translations_conf;
	const char *config_filename = "db/translations.conf"; // FIXME hardcoded name
	struct config_setting_t *translations = NULL;
	int i, size;
	uint32 total = 0;
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
		const char *translation_file = libconfig->setting_get_string_elem(translations, i);
		script->load_translation(translation_file, ++lang_id, &total);
	}
	libconfig->destroy(&translations_conf);

	if( total ) {
		DBIterator *main_iter;
		DBIterator *sub_iter;
		DBMap *string_db;
		struct string_translation *st = NULL;
		uint32 j = 0;

		CREATE(script->translation_buf, char *, total);
		script->translation_buf_size = total;

		main_iter = db_iterator(script->translation_db);
		for( string_db = dbi_first(main_iter); dbi_exists(main_iter); string_db = dbi_next(main_iter) ) {
			sub_iter = db_iterator(string_db);
			for( st = dbi_first(sub_iter); dbi_exists(sub_iter); st = dbi_next(sub_iter) ) {
				script->translation_buf[j++] = st->buf;
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
 *
 **/
const char * script_get_translation_file_name(const char *file) {
	static char file_name[200];
	int i, len = (int)strlen(file), last_bar = -1, last_dot = -1;

	for(i = 0; i < len; i++) {
		if( file[i] == '/' || file[i] == '\\' )
			last_bar = i;
		else if ( file[i] == '.' )
			last_dot = i;
	}

	if( last_bar != -1 || last_dot != -1 ) {
		if( last_bar != -1 && last_dot < last_bar )
			last_dot = -1;
		safestrncpy(file_name, file+(last_bar >= 0 ? last_bar+1 : 0), ( last_dot >= 0 ? ( last_bar >= 0 ? last_dot - last_bar : last_dot ) : sizeof(file_name) ));
		return file_name;
	}

	return file;
}

/**
 * Parses a individual translation file
 **/
void script_load_translation(const char *file, uint8 lang_id, uint32 *total) {
	uint32 translations = 0;
	char line[1024];
	char msgctxt[NAME_LENGTH*2+1] = { 0 };
	DBMap *string_db;
	size_t i;
	FILE *fp;
	struct script_string_buf msgid = { 0 }, msgstr = { 0 };

	if( !(fp = fopen(file,"rb")) ) {
		ShowError("load_translation: failed to open '%s' for reading\n",file);
		return;
	}

	script->add_language(script->get_translation_file_name(file));
	if( lang_id >= atcommand->max_message_table )
		atcommand->expand_message_table();

	while(fgets(line, sizeof(line), fp)) {
		size_t len = strlen(line), cursor = 0;

		if( len <= 1 )
			continue;

		if( line[0] == '#' )
			continue;

		if( strncasecmp(line,"msgctxt \"", 9) == 0 ) {
			msgctxt[0] = '\0';
			for(i = 9; i < len - 2; i++) {
				if( line[i] == '\\' && line[i+1] == '"' ) {
					msgctxt[cursor] = '"';
					i++;
				} else
					msgctxt[cursor] = line[i];
				if( ++cursor >= sizeof(msgctxt) - 1 )
					break;
			}
			msgctxt[cursor] = '\0';
		} else if ( strncasecmp(line, "msgid \"", 7) == 0 ) {
			msgid.pos = 0;
			for(i = 7; i < len - 2; i++) {
				if( line[i] == '\\' && line[i+1] == '"' ) {
					script_string_buf_addb(&msgid, '"');
					i++;
				} else
					script_string_buf_addb(&msgid, line[i]);
			}
			script_string_buf_addb(&msgid,0);
		} else if ( len > 9 && line[9] != '"' && strncasecmp(line, "msgstr \"",8) == 0 ) {
			msgstr.pos = 0;
			for(i = 8; i < len - 2; i++) {
				if( line[i] == '\\' && line[i+1] == '"' ) {
					script_string_buf_addb(&msgstr, '"');
					i++;
				} else
					script_string_buf_addb(&msgstr, line[i]);
			}
			script_string_buf_addb(&msgstr,0);
		}

		if( msgctxt[0] && msgid.pos > 1 && msgstr.pos > 1 ) {
			size_t msgstr_len = msgstr.pos;
			unsigned int inner_len = 1 + (uint32)msgstr_len + 1; //uint8 lang_id + msgstr_len + '\0'

			if( strcasecmp(msgctxt, "messages.conf") == 0 ) {
				int k;

				for(k = 0; k < MAX_MSG; k++) {
					if( atcommand->msg_table[0][k] && strcmpi(atcommand->msg_table[0][k],msgid.ptr) == 0 ) {
						if( atcommand->msg_table[lang_id][k] )
							aFree(atcommand->msg_table[lang_id][k]);
						atcommand->msg_table[lang_id][k] = aStrdup(msgstr.ptr);
						break;
					}
				}
			} else {
				struct string_translation *st = NULL;

				if( !( string_db = strdb_get(script->translation_db, msgctxt) ) ) {
					string_db = strdb_alloc(DB_OPT_DUP_KEY, 0);
					strdb_put(script->translation_db, msgctxt, string_db);
				}

				if( !(st = strdb_get(string_db, msgid.ptr) ) ) {
					CREATE(st, struct string_translation, 1);
					st->string_id = script->string_dup(msgid.ptr);
					strdb_put(string_db, msgid.ptr, st);
				}
				RECREATE(st->buf, char, st->len + inner_len);

				WBUFB(st->buf, st->len) = lang_id;
				safestrncpy(WBUFP(st->buf, st->len + 1), msgstr.ptr, msgstr_len + 1);

				st->translations++;
				st->len += inner_len;
			}
			msgctxt[0] = '\0';
			msgid.pos = msgstr.pos = 0;
			translations++;
		}
	}

	*total += translations;

	fclose(fp);

	script_string_buf_destroy(&msgid);
	script_string_buf_destroy(&msgstr);

	ShowStatus("Done reading '"CL_WHITE"%u"CL_RESET"' translations in '"CL_WHITE"%s"CL_RESET"'.\n", translations, file);
}

/**
 *
 **/
void script_clear_translations(bool reload) {
	uint32 i;

	if( script->string_list )
		aFree(script->string_list);

	script->string_list = NULL;
	script->string_list_pos = 0;
	script->string_list_size = 0;

	if( script->translation_buf ) {
		for(i = 0; i < script->translation_buf_size; i++) {
			aFree(script->translation_buf[i]);
		}
		aFree(script->translation_buf);
	}

	script->translation_buf = NULL;
	script->translation_buf_size = 0;

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
int script_translation_db_destroyer(DBKey key, DBData *data, va_list ap) {
	DBMap *string_db = DB->data2ptr(data);

	if( db_size(string_db) ) {
		struct string_translation *st = NULL;
		DBIterator *iter = db_iterator(string_db);

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
void script_parser_clean_leftovers(void) {
	if( script->buf )
		aFree(script->buf);

	script->buf = NULL;
	script->size = 0;

	if( script->translation_db ) {
		script->translation_db->destroy(script->translation_db,script->translation_db_destroyer);
		script->translation_db = NULL;
	}

	if( script->syntax.strings ) { /* used only when generating translation file */
		db_destroy(script->syntax.strings);
		script->syntax.strings = NULL;
	}

	script_string_buf_destroy(&script->parse_simpleexpr_str);
	script_string_buf_destroy(&script->lang_export_line_buf);
	script_string_buf_destroy(&script->lang_export_unescaped_buf);
}

/**
 * Performs cleanup after all parsing is processed
 **/
int script_parse_cleanup_timer(int tid, int64 tick, int id, intptr_t data) {
	script->parser_clean_leftovers();

	script->parse_cleanup_timer_id = INVALID_TIMER;

	return 0;
}

/*==========================================
 * Initialization
 *------------------------------------------*/
void do_init_script(bool minimal) {
	script->parse_cleanup_timer_id = INVALID_TIMER;

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
	script->read_constdb();
	script->load_parameters();
	script->hardcoded_constants();

	if (minimal)
		return;

	mapreg->init();
	script->load_translations();
}

int script_reload(void) {
	int i;
	DBIterator *iter;
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

	mapreg->reload();

	itemdb->name_constants();

	sysinfo->vcsrevision_reload();

	return 0;
}
/* returns name of current function being run, from within the stack [Ind/Hercules] */
const char *script_getfuncname(struct script_state *st) {
	struct script_data *data;

	data = &st->stack->stack_data[st->start];

	if( data->type == C_NAME && script->str_data[data->u.num].type == C_FUNC )
		return script->get_str(script_getvarid(data->u.num));

	return NULL;
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
BUILDIN(mes)
{
	struct map_session_data *sd = script->rid2sd(st);
	if( sd == NULL )
		return true;

	if( !script_hasdata(st, 3) ) {// only a single line detected in the script
		clif->scriptmes(sd, st->oid, script_getstr(st, 2));
	} else {// parse multiple lines as they exist
		int i;

		for( i = 2; script_hasdata(st, i); i++ ) {
			// send the message to the client
			clif->scriptmes(sd, st->oid, script_getstr(st, i));
		}
	}

	return true;
}

/// Displays the button 'next' in the npc dialog.
/// The dialog text is cleared and the script continues when the button is pressed.
///
/// next;
BUILDIN(next)
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

/// Ends the script and displays the button 'close' on the npc dialog.
/// The dialog is closed when the button is pressed.
///
/// close;
BUILDIN(close)
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
BUILDIN(close2)
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
int menu_countoptions(const char* str, int max_count, int* total)
{
	int count = 0;
	int bogus_total;

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
BUILDIN(menu)
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
		if( StrBuf->Length(&buf) >= 2047 ) {
			struct npc_data * nd = map->id2nd(st->oid);
			char* menu;
			CREATE(menu, char, 2048);
			safestrncpy(menu, StrBuf->Value(&buf), 2047);
			ShowWarning("NPC Menu too long! (source:%s / length:%d)\n",nd?nd->name:"Unknown",StrBuf->Length(&buf));
			clif->scriptmenu(sd, st->oid, menu);
			aFree(menu);
		} else
			clif->scriptmenu(sd, st->oid, StrBuf->Value(&buf));

		StrBuf->Destroy(&buf);

		if( sd->npc_menu >= 0xff )
		{// client supports only up to 254 entries; 0 is not used and 255 is reserved for cancel; excess entries are displayed but cause 'uint8' overflow
			ShowWarning("buildin_menu: Too many options specified (current=%d, max=254).\n", sd->npc_menu);
			script->reportsrc(st);
		}
	}
	else if( sd->npc_menu == 0xff )
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
		pc->setreg(sd, script->add_str("@menu"), menu);
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
BUILDIN(select)
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
		if( StrBuf->Length(&buf) >= 2047 ) {
			struct npc_data * nd = map->id2nd(st->oid);
			char* menu;
			CREATE(menu, char, 2048);
			safestrncpy(menu, StrBuf->Value(&buf), 2047);
			ShowWarning("NPC Menu too long! (source:%s / length:%d)\n",nd?nd->name:"Unknown",StrBuf->Length(&buf));
			clif->scriptmenu(sd, st->oid, menu);
			aFree(menu);
		} else
			clif->scriptmenu(sd, st->oid, StrBuf->Value(&buf));
		StrBuf->Destroy(&buf);

		if( sd->npc_menu >= 0xff ) {
			ShowWarning("buildin_select: Too many options specified (current=%d, max=254).\n", sd->npc_menu);
			script->reportsrc(st);
		}
	} else if( sd->npc_menu == 0xff ) {// Cancel was pressed
		sd->state.menu_or_input = 0;
		st->state = END;
	} else {// return selected option
		int menu = 0;

		sd->state.menu_or_input = 0;
		for( i = 2; i <= script_lastdata(st); ++i ) {
			text = script_getstr(st, i);
			sd->npc_menu -= script->menu_countoptions(text, sd->npc_menu, &menu);
			if( sd->npc_menu <= 0 )
				break;// entry found
		}
		pc->setreg(sd, script->add_str("@menu"), menu);
		script_pushint(st, menu);
		st->state = RUN;
	}
	return true;
}

/// Displays a menu with options and returns the selected option.
/// Behaves like 'menu' without the target labels, except when cancel is
/// pressed.
/// When cancel is pressed, the script continues and 255 is returned.
///
/// prompt(<option_text>{,<option_text>,...}) -> <selected_option>
///
/// @see menu
BUILDIN(prompt)
{
	int i;
	const char *text;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

#ifdef SECURE_NPCTIMEOUT
	sd->npc_idle_type = NPCT_MENU;
#endif

	if( sd->state.menu_or_input == 0 )
	{
		struct StringBuf buf;

		StrBuf->Init(&buf);
		sd->npc_menu = 0;
		for( i = 2; i <= script_lastdata(st); ++i )
		{
			text = script_getstr(st, i);
			if( sd->npc_menu > 0 )
				StrBuf->AppendStr(&buf, ":");
			StrBuf->AppendStr(&buf, text);
			sd->npc_menu += script->menu_countoptions(text, 0, NULL);
		}

		st->state = RERUNLINE;
		sd->state.menu_or_input = 1;

		/* menus beyond this length crash the client (see bugreport:6402) */
		if( StrBuf->Length(&buf) >= 2047 ) {
			struct npc_data * nd = map->id2nd(st->oid);
			char* menu;
			CREATE(menu, char, 2048);
			safestrncpy(menu, StrBuf->Value(&buf), 2047);
			ShowWarning("NPC Menu too long! (source:%s / length:%d)\n",nd?nd->name:"Unknown",StrBuf->Length(&buf));
			clif->scriptmenu(sd, st->oid, menu);
			aFree(menu);
		} else
			clif->scriptmenu(sd, st->oid, StrBuf->Value(&buf));
		StrBuf->Destroy(&buf);

		if( sd->npc_menu >= 0xff )
		{
			ShowWarning("buildin_prompt: Too many options specified (current=%d, max=254).\n", sd->npc_menu);
			script->reportsrc(st);
		}
	}
	else if( sd->npc_menu == 0xff )
	{// Cancel was pressed
		sd->state.menu_or_input = 0;
		pc->setreg(sd, script->add_str("@menu"), 0xff);
		script_pushint(st, 0xff);
		st->state = RUN;
	}
	else
	{// return selected option
		int menu = 0;

		sd->state.menu_or_input = 0;
		for( i = 2; i <= script_lastdata(st); ++i )
		{
			text = script_getstr(st, i);
			sd->npc_menu -= script->menu_countoptions(text, sd->npc_menu, &menu);
			if( sd->npc_menu <= 0 )
				break;// entry found
		}
		pc->setreg(sd, script->add_str("@menu"), menu);
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
BUILDIN(goto)
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
BUILDIN(callfunc)
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
BUILDIN(callsub)
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
BUILDIN(getarg)
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
BUILDIN(return) {
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
BUILDIN(rand)
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
BUILDIN(warp)
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
int buildin_areawarp_sub(struct block_list *bl, va_list ap)
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

		// choose a suitable max number of attempts
		if( (max = (y3-y2+1)*(x3-x2+1)*3) > 1000 )
			max = 1000;

		// find a suitable map cell
		do {
			tx = rnd()%(x3-x2+1)+x2;
			ty = rnd()%(y3-y2+1)+y2;
			j++;
		} while (map->getcell(index, bl, tx, ty, CELL_CHKNOPASS) && j < max);

		pc->setpos(sd, index, tx, ty, CLR_OUTSIGHT);
	} else {
		pc->setpos(sd, index, x2, y2, CLR_OUTSIGHT);
	}
	return 0;
}
BUILDIN(areawarp)
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
int buildin_areapercentheal_sub(struct block_list *bl, va_list ap)
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

BUILDIN(areapercentheal) {
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
BUILDIN(warpchar) {
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
/*==========================================
 * Warpparty - [Fredzilla] [Paradox924X]
 * Syntax: warpparty "to_mapname",x,y,Party_ID,{"from_mapname"};
 * If 'from_mapname' is specified, only the party members on that map will be warped
 *------------------------------------------*/
BUILDIN(warpparty)
{
	struct map_session_data *sd = NULL;
	struct map_session_data *pl_sd;
	struct party_data* p;
	int type;
	int map_index;
	int i;

	const char* str = script_getstr(st,2);
	int x = script_getnum(st,3);
	int y = script_getnum(st,4);
	int p_id = script_getnum(st,5);
	const char* str2 = NULL;
	if ( script_hasdata(st,6) )
		str2 = script_getstr(st,6);

	p = party->search(p_id);
	if(!p)
		return true;

	type = ( strcmp(str,"Random")==0 ) ? 0
	: ( strcmp(str,"SavePointAll")==0 ) ? 1
	: ( strcmp(str,"SavePoint")==0 ) ? 2
	: ( strcmp(str,"Leader")==0 ) ? 3
	: 4;

	switch (type)
	{
		case 3:
			for(i = 0; i < MAX_PARTY && !p->party.member[i].leader; i++);
			if (i == MAX_PARTY || !p->data[i].sd) //Leader not found / not online
				return true;
			pl_sd = p->data[i].sd;
			map_index = pl_sd->mapindex;
			x = pl_sd->bl.x;
			y = pl_sd->bl.y;
			break;
		case 4:
			map_index = script->mapindexname2id(st,str);
			break;
		case 2:
			//"SavePoint" uses save point of the currently attached player
			if (( sd = script->rid2sd(st) ) == NULL )
				return true;
			/* Fall through */
		default:
			map_index = 0;
			break;
	}

	for (i = 0; i < MAX_PARTY; i++) {
		if( !(pl_sd = p->data[i].sd) || pl_sd->status.party_id != p_id )
			continue;

		if( str2 && strcmp(str2, map->list[pl_sd->bl.m].name) != 0 )
			continue;

		if( pc_isdead(pl_sd) )
			continue;

		switch( type )
		{
			case 0: // Random
				if(!map->list[pl_sd->bl.m].flag.nowarp)
					pc->randomwarp(pl_sd,CLR_TELEPORT);
				break;
			case 1: // SavePointAll
				if(!map->list[pl_sd->bl.m].flag.noreturn)
					pc->setpos(pl_sd,pl_sd->status.save_point.map,pl_sd->status.save_point.x,pl_sd->status.save_point.y,CLR_TELEPORT);
				break;
			case 2: // SavePoint
				if(!map->list[pl_sd->bl.m].flag.noreturn)
					pc->setpos(pl_sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,CLR_TELEPORT);
				break;
			case 3: // Leader
			case 4: // m,x,y
				if(!map->list[pl_sd->bl.m].flag.noreturn && !map->list[pl_sd->bl.m].flag.nowarp)
					pc->setpos(pl_sd,map_index,x,y,CLR_TELEPORT);
				break;
		}
	}

	return true;
}
/*==========================================
 * Warpguild - [Fredzilla]
 * Syntax: warpguild "mapname",x,y,Guild_ID;
 *------------------------------------------*/
BUILDIN(warpguild)
{
	struct map_session_data *sd = NULL;
	struct map_session_data *pl_sd;
	struct guild* g;
	struct s_mapiterator* iter;
	int type;

	const char* str = script_getstr(st,2);
	int x           = script_getnum(st,3);
	int y           = script_getnum(st,4);
	int gid         = script_getnum(st,5);

	g = guild->search(gid);
	if( g == NULL )
		return true;

	type = ( strcmp(str,"Random")==0 ) ? 0
	: ( strcmp(str,"SavePointAll")==0 ) ? 1
	: ( strcmp(str,"SavePoint")==0 ) ? 2
	: 3;

	if( type == 2 && ( sd = script->rid2sd(st) ) == NULL )
	{// "SavePoint" uses save point of the currently attached player
		return true;
	}

	iter = mapit_getallusers();
	for (pl_sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); pl_sd = BL_UCAST(BL_PC, mapit->next(iter))) {
		if( pl_sd->status.guild_id != gid )
			continue;

		switch( type )
		{
			case 0: // Random
				if(!map->list[pl_sd->bl.m].flag.nowarp)
					pc->randomwarp(pl_sd,CLR_TELEPORT);
				break;
			case 1: // SavePointAll
				if(!map->list[pl_sd->bl.m].flag.noreturn)
					pc->setpos(pl_sd,pl_sd->status.save_point.map,pl_sd->status.save_point.x,pl_sd->status.save_point.y,CLR_TELEPORT);
				break;
			case 2: // SavePoint
				if(!map->list[pl_sd->bl.m].flag.noreturn)
					pc->setpos(pl_sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,CLR_TELEPORT);
				break;
			case 3: // m,x,y
				if(!map->list[pl_sd->bl.m].flag.noreturn && !map->list[pl_sd->bl.m].flag.nowarp)
					pc->setpos(pl_sd,script->mapindexname2id(st,str),x,y,CLR_TELEPORT);
				break;
		}
	}
	mapit->free(iter);

	return true;
}
/*==========================================
 * Force Heal a player (hp and sp)
 *------------------------------------------*/
BUILDIN(heal)
{
	int hp,sp;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	hp=script_getnum(st,2);
	sp=script_getnum(st,3);
	status->heal(&sd->bl, hp, sp, 1);
	return true;
}
/*==========================================
 * Heal a player by item (get vit bonus etc)
 *------------------------------------------*/
BUILDIN(itemheal)
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
	if (!sd) return true;
	pc->itemheal(sd,sd->itemid,hp,sp);
	return true;
}
/*==========================================
 *
 *------------------------------------------*/
BUILDIN(percentheal)
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
	if( sd == NULL )
		return true;
#ifdef RENEWAL
	if( sd->sc.data[SC_EXTREMITYFIST2] )
		sp = 0;
#endif
	pc->percentheal(sd,hp,sp);
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN(jobchange)
{
	int job, upper=-1;

	job=script_getnum(st,2);
	if( script_hasdata(st,3) )
		upper=script_getnum(st,3);

	if (pc->db_checkid(job)) {
		struct map_session_data *sd = script->rid2sd(st);
		if (sd == NULL)
			return true;

		pc->jobchange(sd, job, upper);
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN(jobname)
{
	int class_=script_getnum(st,2);
	script_pushconststr(st, (char*)pc->job_name(class_));
	return true;
}

/// Get input from the player.
/// For numeric inputs the value is capped to the range [min,max]. Returns 1 if
/// the value was higher than 'max', -1 if lower than 'min' and 0 otherwise.
/// For string inputs it returns 1 if the string was longer than 'max', -1 is
/// shorter than 'min' and 0 otherwise.
///
/// input(<var>{,<min>{,<max>}}) -> <int>
BUILDIN(input)
{
	struct script_data* data;
	int64 uid;
	const char* name;
	int min;
	int max;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	data = script_getdata(st,2);
	if( !data_isreference(data) ) {
		ShowError("script:input: not a variable\n");
		script->reportdata(data);
		st->state = END;
		return false;
	}
	uid = reference_getuid(data);
	name = reference_getname(data);
	min = (script_hasdata(st,3) ? script_getnum(st,3) : script->config.input_min_value);
	max = (script_hasdata(st,4) ? script_getnum(st,4) : script->config.input_max_value);

#ifdef SECURE_NPCTIMEOUT
	sd->npc_idle_type = NPCT_WAIT;
#endif

	if( !sd->state.menu_or_input ) {
		// first invocation, display npc input box
		sd->state.menu_or_input = 1;
		st->state = RERUNLINE;
		if( is_string_variable(name) )
			clif->scriptinputstr(sd,st->oid);
		else
			clif->scriptinput(sd,st->oid);
	} else {
		// take received text/value and store it in the designated variable
		sd->state.menu_or_input = 0;
		if( is_string_variable(name) )
		{
			int len = (int)strlen(sd->npc_str);
			script->set_reg(st, sd, uid, name, (void*)sd->npc_str, script_getref(st,2));
			script_pushint(st, (len > max ? 1 : len < min ? -1 : 0));
		}
		else
		{
			int amount = sd->npc_amount;
			script->set_reg(st, sd, uid, name, (void*)h64BPTRSIZE(cap_value(amount,min,max)), script_getref(st,2));
			script_pushint(st, (amount > max ? 1 : amount < min ? -1 : 0));
		}
		st->state = RUN;
	}
	return true;
}

// declare the copyarray method here for future reference
BUILDIN(copyarray);

/// Sets the value of a variable.
/// The value is converted to the type of the variable.
///
/// set(<variable>,<value>) -> <variable>
BUILDIN(__setr)
{
	struct map_session_data *sd = NULL;
	struct script_data* data;
	//struct script_data* datavalue;
	int64 num;
	const char* name;
	char prefix;

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
	prefix = *name;

	if (not_server_variable(prefix)) {
		sd = script->rid2sd(st);
		if (sd == NULL) {
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
		script->set_reg(st,sd,num,name,(void*)script_getstr(st,3),script_getref(st,2));
	else
		script->set_reg(st,sd,num,name,(void*)h64BPTRSIZE(script_getnum(st,3)),script_getref(st,2));

	return true;
}

/////////////////////////////////////////////////////////////////////
/// Array variables
///

/// Sets values of an array, from the starting index.
/// ex: setarray arr[1],1,2,3;
///
/// setarray <array variable>,<value1>{,<value2>...};
BUILDIN(setarray)
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

	if( is_string_variable(name) )
	{// string array
		for( i = 3; start < end; ++start, ++i )
			script->set_reg(st, sd, reference_uid(id, start), name, (void*)script_getstr(st,i), reference_getref(data));
	}
	else
	{// int array
		for( i = 3; start < end; ++start, ++i )
			script->set_reg(st, sd, reference_uid(id, start), name, (void*)h64BPTRSIZE(script_getnum(st,i)), reference_getref(data));
	}
	return true;
}

/// Sets count values of an array, from the starting index.
/// ex: cleararray arr[0],0,1;
///
/// cleararray <array variable>,<value>,<count>;
BUILDIN(cleararray)
{
	struct script_data* data;
	const char* name;
	uint32 start;
	uint32 end;
	int32 id;
	void* v;
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

	if( is_string_variable(name) )
		v = (void*)script_getstr(st, 3);
	else
		v = (void*)h64BPTRSIZE(script_getnum(st, 3));

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
BUILDIN(copyarray)
{
	struct script_data* data1;
	struct script_data* data2;
	const char* name1;
	const char* name2;
	int32 idx1;
	int32 idx2;
	int32 id1;
	int32 id2;
	void* v;
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
			v = script->get_val2(st, reference_uid(id2, idx2 + i), reference_getref(data2));
			script->set_reg(st, sd, reference_uid(id1, idx1 + i), name1, v, reference_getref(data1));
			script_removetop(st, -1, 0);
		}
	} else {
		// normal copy
		for( i = 0; i < count; ++i ) {
			if( idx2 + i < SCRIPT_MAX_ARRAYSIZE ) {
				v = script->get_val2(st, reference_uid(id2, idx2 + i), reference_getref(data2));
				script->set_reg(st, sd, reference_uid(id1, idx1 + i), name1, v, reference_getref(data1));
				script_removetop(st, -1, 0);
			} else {
				// out of range - assume ""/0
				script->set_reg(st, sd, reference_uid(id1, idx1 + i), name1, (is_string_variable(name1)?(void*)"":(void*)0), reference_getref(data1));
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
BUILDIN(getarraysize)
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
int script_array_index_cmp(const void *a, const void *b) {
	return ( *(const unsigned int*)a - *(const unsigned int*)b );
}

/// Deletes count or all the elements in an array, from the starting index.
/// ex: deletearray arr[4],2;
///
/// deletearray <array variable>;
/// deletearray <array variable>,<count>;
BUILDIN(deletearray)
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
				void* v = script->get_val2(st, reference_uid(id, start + count), reference_getref(data));
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
				void* v = script->get_val2(st, reference_uid(id, list[i]), reference_getref(data));
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
BUILDIN(getelementofarray)
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
	if (i < 0 || i >= SCRIPT_MAX_ARRAYSIZE) {
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
BUILDIN(setlook)
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

BUILDIN(changelook)
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
BUILDIN(cutin)
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
BUILDIN(viewpoint)
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
BUILDIN(countitem) {
	int nameid, i;
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

	nameid = id->nameid;

	for(i = 0; i < MAX_INVENTORY; i++)
		if(sd->status.inventory[i].nameid == nameid)
			count += sd->status.inventory[i].amount;

	script_pushint(st,count);
	return true;
}

/*==========================================
 * countitem2(nameID,Identified,Refine,Attribute,Card0,Card1,Card2,Card3) [Lupus]
 * returns number of items that meet the conditions
 *------------------------------------------*/
BUILDIN(countitem2) {
	int nameid, iden, ref, attr, c1, c2, c3, c4;
	int count = 0;
	int i;
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
	c1 = (short)script_getnum(st,6);
	c2 = (short)script_getnum(st,7);
	c3 = (short)script_getnum(st,8);
	c4 = (short)script_getnum(st,9);

	for(i = 0; i < MAX_INVENTORY; i++)
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
 * Check if item with this amount can fit in inventory
 * Checking : weight, stack amount >32k, slots amount >(MAX_INVENTORY)
 * Return
 * 0 : fail
 * 1 : success (npc side only)
 *------------------------------------------*/
BUILDIN(checkweight)
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

BUILDIN(checkweight2)
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
BUILDIN(getitem) {
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
					map->addflooritem(&sd->bl, &it, get_count, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0);
			}
		}
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN(getitem2)
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
	c1=(short)script_getnum(st,7);
	c2=(short)script_getnum(st,8);
	c3=(short)script_getnum(st,9);
	c4=(short)script_getnum(st,10);

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
		item_tmp.card[0]=(short)c1;
		item_tmp.card[1]=(short)c2;
		item_tmp.card[2]=(short)c3;
		item_tmp.card[3]=(short)c4;

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
						map->addflooritem(&sd->bl, &item_tmp, get_count, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0);
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
BUILDIN(rentitem) {
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
BUILDIN(getnameditem) {
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
	item_tmp.nameid=nameid;
	item_tmp.amount=1;
	item_tmp.identify=1;
	item_tmp.card[0]=CARD0_CREATE; //we don't use 255! because for example SIGNED WEAPON shouldn't get TOP10 BS Fame bonus [Lupus]
	item_tmp.card[2]=tsd->status.char_id;
	item_tmp.card[3]=tsd->status.char_id >> 16;
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
BUILDIN(grouprandomitem) {
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
BUILDIN(makeitem)
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

	map->addflooritem(NULL, &item_tmp, amount, m, x, y, 0, 0, 0, 0);

	return true;
}

/// Counts / deletes the current item given by idx.
/// Used by buildin_delitem_search
/// Relies on all input data being already fully valid.
void buildin_delitem_delete(struct map_session_data* sd, int idx, int* amount, bool delete_items)
{
	int delamount;
	struct item* inv = &sd->status.inventory[idx];

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
bool buildin_delitem_search(struct map_session_data* sd, struct item* it, bool exact_match)
{
	bool delete_items = false;
	int i, amount;
	struct item* inv;

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
BUILDIN(delitem)
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
BUILDIN(delitem2)
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
	it.card[0]=(short)script_getnum(st,7);
	it.card[1]=(short)script_getnum(st,8);
	it.card[2]=(short)script_getnum(st,9);
	it.card[3]=(short)script_getnum(st,10);

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

/*==========================================
 * Enables/Disables use of items while in an NPC [Skotlex]
 *------------------------------------------*/
BUILDIN(enableitemuse)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd != NULL)
		st->npc_item_flag = sd->npc_item_flag = 1;
	return true;
}

BUILDIN(disableitemuse)
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
BUILDIN(readparam) {
	int type;
	struct map_session_data *sd;

	type=script_getnum(st,2);
	if (script_hasdata(st,3))
		sd = script->nick2sd(st, script_getstr(st,3));
	else
		sd=script->rid2sd(st);

	if (sd == NULL) {
		script_pushint(st,-1);
		return true;
	}

	script_pushint(st,pc->readparam(sd,type));

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
 *------------------------------------------*/
BUILDIN(getcharid) {
	int num;
	struct map_session_data *sd;

	num = script_getnum(st,2);
	if( script_hasdata(st,3) )
		sd=map->nick2sd(script_getstr(st,3));
	else
		sd=script->rid2sd(st);

	if(sd==NULL) {
		script_pushint(st,0); //return 0, according docs
		return true;
	}

	switch( num ) {
		case 0: script_pushint(st,sd->status.char_id); break;
		case 1: script_pushint(st,sd->status.party_id); break;
		case 2: script_pushint(st,sd->status.guild_id); break;
		case 3: script_pushint(st,sd->status.account_id); break;
		case 4: script_pushint(st,sd->bg_id); break;
		default:
			ShowError("buildin_getcharid: invalid parameter (%d).\n", num);
			script_pushint(st,0);
			break;
	}

	return true;
}
/*==========================================
 * returns the GID of an NPC
 *------------------------------------------*/
BUILDIN(getnpcid)
{
	int num = script_getnum(st,2);
	struct npc_data* nd = NULL;

	if( script_hasdata(st,3) )
	{// unique npc name
		if( ( nd = npc->name2id(script_getstr(st,3)) ) == NULL )
		{
			ShowError("buildin_getnpcid: No such NPC '%s'.\n", script_getstr(st,3));
			script_pushint(st,0);
			return false;
		}
	}

	switch (num) {
		case 0:
			script_pushint(st,nd ? nd->bl.id : st->oid);
			break;
		default:
			ShowError("buildin_getnpcid: invalid parameter (%d).\n", num);
			script_pushint(st,0);
			return false;
	}

	return true;
}

/*==========================================
 * Return the name of the party_id
 * null if not found
 *------------------------------------------*/
BUILDIN(getpartyname)
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
BUILDIN(getpartymember)
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
						mapreg->setreg(reference_uid(script->add_str("$@partymemberaid"), j),p->party.member[i].account_id);
						break;
					case 1:
						mapreg->setreg(reference_uid(script->add_str("$@partymembercid"), j),p->party.member[i].char_id);
						break;
					default:
						mapreg->setregstr(reference_uid(script->add_str("$@partymembername$"), j),p->party.member[i].name);
				}
				j++;
			}
		}
	}
	mapreg->setreg(script->add_str("$@partymembercount"),j);

	return true;
}

/*==========================================
 * Retrieves party leader. if flag is specified,
 * return some of the leader data. Otherwise, return name.
 *------------------------------------------*/
BUILDIN(getpartyleader)
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
		case 3: script_pushint(st,p->party.member[i].class_); break;
		case 4: script_pushstrcopy(st,mapindex_id2name(p->party.member[i].map)); break;
		case 5: script_pushint(st,p->party.member[i].lv); break;
		default: script_pushstrcopy(st,p->party.member[i].name); break;
	}
	return true;
}

/*==========================================
 * Return the name of the @guild_id
 * null if not found
 *------------------------------------------*/
BUILDIN(getguildname)
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
BUILDIN(getguildmaster)
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

BUILDIN(getguildmasterid)
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
BUILDIN(getguildmember)
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
					mapreg->setreg(reference_uid(script->add_str("$@guildmemberaid"), j),g->member[i].account_id);
					break;
				case 1:
					mapreg->setreg(reference_uid(script->add_str("$@guildmembercid"), j), g->member[i].char_id);
					break;
				default:
					mapreg->setregstr(reference_uid(script->add_str("$@guildmembername$"), j), g->member[i].name);
					break;
				}
				j++;
			}
		}
	}
	mapreg->setreg(script->add_str("$@guildmembercount"), j);
	return true;
}

/*==========================================
 * Get char string information by type :
 * Return by @type :
 * 0 : char_name
 * 1 : party_name or ""
 * 2 : guild_name or ""
 * 3 : map_name
 * - : ""
 *------------------------------------------*/
BUILDIN(strcharinfo)
{
	int num;
	struct guild* g;
	struct party_data* p;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL) //Avoid crashing....
		return true;

	num=script_getnum(st,2);
	switch(num) {
		case 0:
			script_pushstrcopy(st,sd->status.name);
			break;
		case 1:
			if( ( p = party->search(sd->status.party_id) ) != NULL ) {
				script_pushstrcopy(st,p->party.name);
			} else {
				script_pushconststr(st,"");
			}
			break;
		case 2:
			if( ( g = sd->guild ) != NULL ) {
				script_pushstrcopy(st,g->name);
			} else {
				script_pushconststr(st,"");
			}
			break;
		case 3:
			script_pushconststr(st,map->list[sd->bl.m].name);
			break;
		default:
			ShowWarning("buildin_strcharinfo: unknown parameter.\n");
			script_pushconststr(st,"");
			break;
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
BUILDIN(strnpcinfo)
{
	int num;
	char *buf,*name=NULL;
	struct npc_data *nd = map->id2nd(st->oid);
	if (nd == NULL) {
		script_pushconststr(st, "");
		return true;
	}

	num = script_getnum(st,2);
	switch(num) {
		case 0: // display name
			name = aStrdup(nd->name);
			break;
		case 1: // visible part of display name
			if((buf = strchr(nd->name,'#')) != NULL)
			{
				name = aStrdup(nd->name);
				name[buf - nd->name] = 0;
			} else // Return the name, there is no '#' present
				name = aStrdup(nd->name);
			break;
		case 2: // # fragment
			if((buf = strchr(nd->name,'#')) != NULL)
				name = aStrdup(buf+1);
			break;
		case 3: // unique name
			name = aStrdup(nd->exname);
			break;
		case 4: // map name
			if( nd->bl.m >= 0 ) // Only valid map indexes allowed (issue:8034)
				name = aStrdup(map->list[nd->bl.m].name);
			break;
	}

	if(name)
		script_pushstr(st, name);
	else
		script_pushconststr(st, "");

	return true;
}

/**
 * charid2rid: Returns the RID associated to the given character ID
 */
BUILDIN(charid2rid)
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
BUILDIN(getequipid)
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
BUILDIN(getequipname)
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
BUILDIN(getbrokenid)
{
	int i,num,id=0,brokencounter=0;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	num=script_getnum(st,2);
	for(i=0; i<MAX_INVENTORY; i++) {
		if(sd->status.inventory[i].attribute) {
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
BUILDIN(getbrokencount)
{
	int i, counter = 0;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].attribute)
			counter++;
	}

	script_pushint(st, counter);

	return true;
}

/*==========================================
 * repair [Valaris]
 *------------------------------------------*/
BUILDIN(repair)
{
	int i,num;
	int repaircounter=0;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	num=script_getnum(st,2);
	for(i=0; i<MAX_INVENTORY; i++) {
		if(sd->status.inventory[i].attribute) {
			repaircounter++;
			if(num==repaircounter) {
				sd->status.inventory[i].attribute=0;
				clif->equiplist(sd);
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
BUILDIN(repairall)
{
	int i, repaircounter = 0;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	for(i = 0; i < MAX_INVENTORY; i++)
	{
		if(sd->status.inventory[i].nameid && sd->status.inventory[i].attribute)
		{
			sd->status.inventory[i].attribute = 0;
			clif->produce_effect(sd,0,sd->status.inventory[i].nameid);
			repaircounter++;
		}
	}

	if(repaircounter)
	{
		clif->misceffect(&sd->bl, 3);
		clif->equiplist(sd);
	}

	return true;
}

/*==========================================
 * Chk if player have something equiped at pos
 *------------------------------------------*/
BUILDIN(getequipisequiped)
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
BUILDIN(getequipisenableref)
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

/*==========================================
 * Chk if the item equiped at pos is identify (huh ?)
 * return (npc)
 * 1 : true
 * 0 : false
 *------------------------------------------*/
BUILDIN(getequipisidentify)
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
 * Get the item refined value at pos
 * return (npc)
 * x : refine amount
 * 0 : false (not refined)
 *------------------------------------------*/
BUILDIN(getequiprefinerycnt)
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
		script_pushint(st,sd->status.inventory[i].refine);
	else
		script_pushint(st,0);

	return true;
}

/*==========================================
 * Get the weapon level value at pos
 * (pos should normally only be EQI_HAND_L or EQI_HAND_R)
 * return (npc)
 * x : weapon level
 * 0 : false
 *------------------------------------------*/
BUILDIN(getequipweaponlv)
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
BUILDIN(getequippercentrefinery) {
	int i = -1,num;
	struct map_session_data *sd;

	num = script_getnum(st,2);
	sd = script->rid2sd(st);
	if( sd == NULL )
		return true;

	if (num > 0 && num <= ARRAYLENGTH(script->equip))
		i=pc->checkequip(sd,script->equip[num-1]);
	if(i >= 0 && sd->status.inventory[i].nameid && sd->status.inventory[i].refine < MAX_REFINE)
		script_pushint(st,status->get_refine_chance(itemdb_wlv(sd->status.inventory[i].nameid), (int)sd->status.inventory[i].refine));
	else
		script_pushint(st,0);

	return true;
}

/*==========================================
 * Refine +1 item at pos and log and display refine
 *------------------------------------------*/
BUILDIN(successrefitem)
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
		if(sd->status.inventory[i].refine == 10 &&
		   sd->status.inventory[i].card[0] == CARD0_FORGE &&
		   sd->status.char_id == (int)MakeDWord(sd->status.inventory[i].card[2],sd->status.inventory[i].card[3])
		  ) { // Fame point system [DracoRPG]
			switch (sd->inventory_data[i]->wlv) {
				case 1:
					pc->addfame(sd,1); // Success to refine to +10 a lv1 weapon you forged = +1 fame point
					break;
				case 2:
					pc->addfame(sd,25); // Success to refine to +10 a lv2 weapon you forged = +25 fame point
					break;
				case 3:
					pc->addfame(sd,1000); // Success to refine to +10 a lv3 weapon you forged = +1000 fame point
					break;
			}
		}
	}

	return true;
}

/*==========================================
 * Show a failed Refine +1 attempt
 *------------------------------------------*/
BUILDIN(failedrefitem)
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
BUILDIN(downrefitem)
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
		clif->misceffect(&sd->bl,2);
	}

	return true;
}

/*==========================================
 * Delete the item equipped at pos.
 *------------------------------------------*/
BUILDIN(delequip)
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
BUILDIN(statusup) {
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
BUILDIN(statusup2)
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

/// See 'doc/item_bonus.txt'
///
/// bonus <bonus type>,<val1>;
/// bonus2 <bonus type>,<val1>,<val2>;
/// bonus3 <bonus type>,<val1>,<val2>,<val3>;
/// bonus4 <bonus type>,<val1>,<val2>,<val3>,<val4>;
/// bonus5 <bonus type>,<val1>,<val2>,<val3>,<val4>,<val5>;
BUILDIN(bonus) {
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
			// else fall through
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

BUILDIN(autobonus) {
	unsigned int dur;
	short rate;
	short atk_type = 0;
	const char *bonus_script, *other_script = NULL;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true; // no player attached

	if( sd->state.autobonus&sd->status.inventory[status->current_equip_item_index].equip )
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

BUILDIN(autobonus2) {
	unsigned int dur;
	short rate;
	short atk_type = 0;
	const char *bonus_script, *other_script = NULL;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true; // no player attached

	if( sd->state.autobonus&sd->status.inventory[status->current_equip_item_index].equip )
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

BUILDIN(autobonus3) {
	unsigned int dur;
	short rate,atk_type;
	const char *bonus_script, *other_script = NULL;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true; // no player attached

	if( sd->state.autobonus&sd->status.inventory[status->current_equip_item_index].equip )
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
BUILDIN(skill) {
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
BUILDIN(addtoskill) {
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
BUILDIN(guildskill) {
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
BUILDIN(getskilllv)
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
BUILDIN(getgdskilllv) {
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
BUILDIN(basicskillcheck)
{
	script_pushint(st, battle_config.basic_skill_check);
	return true;
}

/// Returns the GM level of the player.
///
/// getgmlevel() -> <level>
BUILDIN(getgmlevel)
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
BUILDIN(setgroupid) {
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
/// getgroupid() -> <int>
BUILDIN(getgroupid)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true; // no player attached, report source
	script_pushint(st, pc_get_group_id(sd));

	return true;
}

/// Terminates the execution of this script instance.
///
/// end
BUILDIN(end) {
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
BUILDIN(checkoption)
{
	int option;
	struct map_session_data *sd = script->rid2sd(st);
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
BUILDIN(checkoption1)
{
	int opt1;
	struct map_session_data *sd = script->rid2sd(st);
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
BUILDIN(checkoption2)
{
	int opt2;
	struct map_session_data *sd = script->rid2sd(st);
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
BUILDIN(setoption)
{
	int option;
	int flag = 1;
	struct map_session_data *sd = script->rid2sd(st);
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
BUILDIN(checkcart)
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
BUILDIN(setcart)
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
BUILDIN(checkfalcon)
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
BUILDIN(setfalcon)
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
BUILDIN(checkmount)
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
 */
BUILDIN(setmount)
{
	int flag = SETMOUNT_TYPE_AUTODETECT;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;// no player attached, report source

	if (script_hasdata(st,2))
		flag = script_getnum(st,2);

	// Color variants for Rune Knight dragon mounts.
	if (flag != SETMOUNT_TYPE_NONE) {
		if (flag < SETMOUNT_TYPE_AUTODETECT || flag >= SETMOUNT_TYPE_MAX) {
			ShowWarning("script_setmount: Unknown flag %d specified. Using auto-detected value.\n", flag);
			flag = SETMOUNT_TYPE_AUTODETECT;
		}
		// Sanity checks and auto-detection
		if ((sd->class_&MAPID_THIRDMASK) == MAPID_RUNE_KNIGHT) {
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
		} else if ((sd->class_&MAPID_THIRDMASK) == MAPID_RANGER) {
			// Ranger (Warg)
			if (pc->checkskill(sd, RA_WUGRIDER))
				pc->setridingwug(sd, true);
		} else if ((sd->class_&MAPID_THIRDMASK) == MAPID_MECHANIC) {
			// Mechanic (Mado Gear)
			if (pc->checkskill(sd, NC_MADOLICENCE))
				pc->setmadogear(sd, true);
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
			pc->setmadogear(sd, false);
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
BUILDIN(checkwug)
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
BUILDIN(savepoint) {
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
BUILDIN(gettimetick) { /* Asgard Version */
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
BUILDIN(gettime) { /* Asgard Version */
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

/*==========================================
 * GetTimeStr("TimeFMT", Length);
 *------------------------------------------*/
BUILDIN(gettimestr)
{
	char *tmpstr;
	const char *fmtstr;
	int maxlen;
	time_t now = time(NULL);

	fmtstr=script_getstr(st,2);
	maxlen=script_getnum(st,3);

	tmpstr=(char *)aMalloc((maxlen+1)*sizeof(char));
	strftime(tmpstr,maxlen,fmtstr,localtime(&now));
	tmpstr[maxlen]='\0';

	script_pushstr(st,tmpstr);
	return true;
}

/*==========================================
 * Open player storage
 *------------------------------------------*/
BUILDIN(openstorage)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	storage->open(sd);
	return true;
}

BUILDIN(guildopenstorage)
{
	int ret;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	ret = gstorage->open(sd);
	script_pushint(st,ret);
	return true;
}

/*==========================================
 * Make player use a skill trought item usage
 *------------------------------------------*/
/// itemskill <skill id>,<level>{,flag
/// itemskill "<skill name>",<level>{,flag
BUILDIN(itemskill)
{
	int id;
	int lv;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL || sd->ud.skilltimer != INVALID_TIMER)
		return true;

	id = ( script_isstringtype(st,2) ? skill->name2id(script_getstr(st,2)) : script_getnum(st,2) );
	lv = script_getnum(st,3);
/* temporarily disabled, awaiting for kenpachi to detail this so we can make it work properly */
#if 0
	if( !script_hasdata(st, 4) ) {
		if( !skill->check_condition_castbegin(sd,id,lv) || !skill->check_condition_castend(sd,id,lv) )
			return true;
	}
#endif
	sd->skillitem=id;
	sd->skillitemlv=lv;
	clif->item_skill(sd,id,lv);
	return true;
}
/*==========================================
 * Attempt to create an item
 *------------------------------------------*/
BUILDIN(produce)
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
BUILDIN(cooking)
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
BUILDIN(makepet)
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
		                  (short)pet->db[pet_id].class_, (short)mob->db(pet->db[pet_id].class_)->lv,
		                  (short)pet->db[pet_id].EggID, 0, (short)pet->db[pet_id].intimate,
		                  100, 0, 1, pet->db[pet_id].jname);
	}

	return true;
}
/*==========================================
 * Give player exp base,job * quest_exp_rate/100
 *------------------------------------------*/
BUILDIN(getexp)
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
BUILDIN(guildgetexp)
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
BUILDIN(guildchangegm)
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
		script_pushint(st,guild->gm_change(guild_id, sd));

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
BUILDIN(monster)
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
BUILDIN(getmobdrops)
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

		mapreg->setreg(reference_uid(script->add_str("$@MobDrop_item"), j), monster->dropitem[i].nameid);
		mapreg->setreg(reference_uid(script->add_str("$@MobDrop_rate"), j), monster->dropitem[i].p);

		j++;
	}

	mapreg->setreg(script->add_str("$@MobDrop_count"), j);
	script_pushint(st, 1);

	return true;
}
/*==========================================
 * Same as monster but randomize location in x0,x1,y0,y1 area
 *------------------------------------------*/
BUILDIN(areamonster) {
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
int buildin_killmonster_sub_strip(struct block_list *bl, va_list ap)
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
int buildin_killmonster_sub(struct block_list *bl, va_list ap)
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
BUILDIN(killmonster) {
	const char *mapname,*event;
	int16 m,allflag=0;
	mapname=script_getstr(st,2);
	event=script_getstr(st,3);
	if(strcmp(event,"All")==0)
		allflag = 1;
	else
		script->check_event(st, event);

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

int buildin_killmonsterall_sub_strip(struct block_list *bl,va_list ap)
{ //Strips the event from the mob if it's killed the old method.
	struct mob_data *md;

	md = BL_CAST(BL_MOB, bl);
	if (md->npc_event[0])
		md->npc_event[0] = 0;

	status_kill(bl);
	return 0;
}
int buildin_killmonsterall_sub(struct block_list *bl,va_list ap)
{
	status_kill(bl);
	return 0;
}
BUILDIN(killmonsterall) {
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

/*==========================================
 * Creates a clone of a player.
 * clone map, x, y, event, char_id, master_id, mode, flag, duration
 *------------------------------------------*/
BUILDIN(clone) {
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
BUILDIN(doevent)
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
BUILDIN(donpcevent)
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
BUILDIN(addtimer)
{
	int tick = script_getnum(st,2);
	const char* event = script_getstr(st, 3);
	struct map_session_data *sd;

	script->check_event(st, event);
	sd = script->rid2sd(st);
	if( sd == NULL )
		return true;

	if (!pc->addeventtimer(sd,tick,event)) {
		ShowWarning("buildin_addtimer: Event timer is full, can't add new event timer. (cid:%d timer:%s)\n",sd->status.char_id,event);
		return false;
	}
	return true;
}
/*==========================================
 *------------------------------------------*/
BUILDIN(deltimer)
{
	const char *event;
	struct map_session_data *sd;

	event=script_getstr(st, 2);
	sd = script->rid2sd(st);
	if( sd == NULL )
		return true;

	script->check_event(st, event);
	pc->deleventtimer(sd,event);
	return true;
}
/*==========================================
 *------------------------------------------*/
BUILDIN(addtimercount)
{
	const char *event;
	int tick;
	struct map_session_data *sd;

	event=script_getstr(st, 2);
	tick=script_getnum(st,3);
	sd = script->rid2sd(st);
	if( sd == NULL )
		return true;

	script->check_event(st, event);
	pc->addeventtimercount(sd,event,tick);
	return true;
}

/*==========================================
 *------------------------------------------*/
BUILDIN(initnpctimer)
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
BUILDIN(startnpctimer)
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
BUILDIN(stopnpctimer) {
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
BUILDIN(getnpctimer)
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
BUILDIN(setnpctimer)
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
BUILDIN(attachnpctimer)
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
BUILDIN(detachnpctimer) {
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
BUILDIN(playerattached) {
	if(st->rid == 0 || map->id2sd(st->rid) == NULL)
		script_pushint(st,0);
	else
		script_pushint(st,st->rid);
	return true;
}

/*==========================================
 *------------------------------------------*/
BUILDIN(announce) {
	const char *mes       = script_getstr(st,2);
	int         flag      = script_getnum(st,3);
	const char *fontColor = script_hasdata(st,4) ? script_getstr(st,4) : NULL;
	int         fontType  = script_hasdata(st,5) ? script_getnum(st,5) : 0x190; // default fontType (FW_NORMAL)
	int         fontSize  = script_hasdata(st,6) ? script_getnum(st,6) : 12;    // default fontSize
	int         fontAlign = script_hasdata(st,7) ? script_getnum(st,7) : 0;     // default fontAlign
	int         fontY     = script_hasdata(st,8) ? script_getnum(st,8) : 0;     // default fontY

	if( flag&(BC_TARGET_MASK|BC_SOURCE_MASK) ) {
		// Broadcast source or broadcast region defined
		send_target target;
		struct block_list *bl = NULL;
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

		if (fontColor)
			clif->broadcast2(bl, mes, (int)strlen(mes)+1, (unsigned int)strtoul(fontColor, (char **)NULL, 0), fontType, fontSize, fontAlign, fontY, target);
		else
			clif->broadcast(bl, mes, (int)strlen(mes)+1, flag&BC_COLOR_MASK, target);
	} else {
		if (fontColor)
			intif->broadcast2(mes, (int)strlen(mes)+1, (unsigned int)strtoul(fontColor, (char **)NULL, 0), fontType, fontSize, fontAlign, fontY);
		else
			intif->broadcast(mes, (int)strlen(mes)+1, flag&BC_COLOR_MASK);
	}
	return true;
}
/*==========================================
 *------------------------------------------*/
int buildin_announce_sub(struct block_list *bl, va_list ap)
{
	char *mes       = va_arg(ap, char *);
	int   len       = va_arg(ap, int);
	int   type      = va_arg(ap, int);
	char *fontColor = va_arg(ap, char *);
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
BUILDIN(itemeffect)
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

BUILDIN(mapannounce) {
	const char *mapname   = script_getstr(st,2);
	const char *mes       = script_getstr(st,3);
	int         flag      = script_getnum(st,4);
	const char *fontColor = script_hasdata(st,5) ? script_getstr(st,5) : NULL;
	int         fontType  = script_hasdata(st,6) ? script_getnum(st,6) : 0x190; // default fontType (FW_NORMAL)
	int         fontSize  = script_hasdata(st,7) ? script_getnum(st,7) : 12;    // default fontSize
	int         fontAlign = script_hasdata(st,8) ? script_getnum(st,8) : 0;     // default fontAlign
	int         fontY     = script_hasdata(st,9) ? script_getnum(st,9) : 0;     // default fontY
	int16 m;

	if ((m = map->mapname2mapid(mapname)) < 0)
		return true;

	map->foreachinmap(script->buildin_announce_sub, m, BL_PC,
	                  mes, strlen(mes)+1, flag&BC_COLOR_MASK, fontColor, fontType, fontSize, fontAlign, fontY);
	return true;
}
/*==========================================
 *------------------------------------------*/
BUILDIN(areaannounce) {
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

	if ((m = map->mapname2mapid(mapname)) < 0)
		return true;

	map->foreachinarea(script->buildin_announce_sub, m, x0, y0, x1, y1, BL_PC,
	                   mes, strlen(mes)+1, flag&BC_COLOR_MASK, fontColor, fontType, fontSize, fontAlign, fontY);
	return true;
}

/*==========================================
 *------------------------------------------*/
BUILDIN(getusers) {
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
BUILDIN(getusersname)
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
BUILDIN(getmapguildusers)
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
BUILDIN(getmapusers) {
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
int buildin_getareausers_sub(struct block_list *bl,va_list ap)
{
	int *users=va_arg(ap,int *);
	(*users)++;
	return 0;
}

BUILDIN(getareausers)
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
int buildin_getareadropitem_sub(struct block_list *bl, va_list ap)
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

BUILDIN(getareadropitem) {
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
BUILDIN(enablenpc)
{
	const char *str;
	str=script_getstr(st,2);
	npc->enable(str,1);
	return true;
}
/*==========================================
 *------------------------------------------*/
BUILDIN(disablenpc)
{
	const char *str;
	str=script_getstr(st,2);
	npc->enable(str,0);
	return true;
}

/*==========================================
 *------------------------------------------*/
BUILDIN(hideoffnpc)
{
	const char *str;
	str=script_getstr(st,2);
	npc->enable(str,2);
	return true;
}
/*==========================================
 *------------------------------------------*/
BUILDIN(hideonnpc)
{
	const char *str;
	str=script_getstr(st,2);
	npc->enable(str,4);
	return true;
}

/* Starts a status effect on the target unit or on the attached player.
 *
 * sc_start  <effect_id>,<duration>,<val1>{,<rate>,<flag>,{<unit_id>}};
 * sc_start2 <effect_id>,<duration>,<val1>,<val2>{,<rate,<flag>,{<unit_id>}};
 * sc_start4 <effect_id>,<duration>,<val1>,<val2>,<val3>,<val4>{,<rate,<flag>,{<unit_id>}};
 * <flag>: @see enum scstart_flag
 */
BUILDIN(sc_start)
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
BUILDIN(sc_end) {
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
		sce->val1 = sce->val2 = sce->val3 = sce->val4 = 0;
		status_change_end(bl, (sc_type)type, INVALID_TIMER);
	}
	else
		status->change_clear(bl, 3); // remove all effects

	return true;
}

/*==========================================
 * @FIXME atm will return reduced tick, 0 immune, 1 no tick
 *------------------------------------------*/
BUILDIN(getscrate) {
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
BUILDIN(getstatus)
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
		{
			const struct TimerData* td = (const struct TimerData*)timer->get(sd->sc.data[id]->timer);

			if( td ) {
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
BUILDIN(debugmes)
{
	const char *str;
	str=script_getstr(st,2);
	ShowDebug("script debug : %d %d : %s\n",st->rid,st->oid,str);
	return true;
}

/*==========================================
 *------------------------------------------*/
BUILDIN(catchpet)
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
BUILDIN(homunculus_evolution)
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
BUILDIN(homunculus_mutate)
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
			homun_id = 6048 + (rnd() % 4);

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
BUILDIN(homunculus_morphembryo)
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
				homun->vaporize(sd, HOM_ST_MORPH);
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
BUILDIN(homunculus_checkcall)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL || sd->hd == NULL)
		script_pushint(st, -1);
	else
		script_pushint(st, sd->hd->homunculus.vaporize);

	return true;
}

// [Zephyrus]
BUILDIN(homunculus_shuffle)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	if(homun_alive(sd->hd))
		homun->shuffle(sd->hd);

	return true;
}

//These two functions bring the eA MAPID_* class functionality to scripts.
BUILDIN(eaclass)
{
	int class_;
	if (script_hasdata(st,2)) {
		class_ = script_getnum(st,2);
	} else {
		struct map_session_data *sd = script->rid2sd(st);
		if (sd == NULL)
			return true;
		class_ = sd->status.class_;
	}
	script_pushint(st,pc->jobid2mapid(class_));
	return true;
}

BUILDIN(roclass)
{
	int class_ =script_getnum(st,2);
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
	script_pushint(st,pc->mapid2jobid(class_, sex));
	return true;
}

/*==========================================
 * Tells client to open a hatching window, used for pet incubator
 *------------------------------------------*/
BUILDIN(birthpet)
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
BUILDIN(resetlvl)
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
BUILDIN(resetstatus)
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
BUILDIN(resetskill)
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
BUILDIN(skillpointcount)
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
BUILDIN(changebase)
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
			sd->class_&JOBL_BABY //Baby classes screw up when showing wedding sprites. [Skotlex] They don't seem to anymore.
			)
			return true;
	}

	if(sd->disguise == -1 && vclass != sd->vd.class_)
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
BUILDIN(changesex)
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
BUILDIN(changecharsex)
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
BUILDIN(globalmes)
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
BUILDIN(waitingroom)
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
BUILDIN(delwaitingroom) {
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
BUILDIN(waitingroomkickall) {
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
BUILDIN(enablewaitingroomevent) {
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
BUILDIN(disablewaitingroomevent) {
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
BUILDIN(getwaitingroomstate)
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
				mapreg->setreg(reference_uid(script->add_str("$@chatmembers"), i), sd->bl.id);
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

		case 34: script_pushint(st, cd->minLvl); break;
		case 35: script_pushint(st, cd->maxLvl); break;
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
BUILDIN(warpwaitingpc)
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

		mapreg->setreg(reference_uid(script->add_str("$@warpwaitingpc"), i), sd->bl.id);

		if( strcmp(map_name,"Random") == 0 )
			pc->randomwarp(sd,CLR_TELEPORT);
		else if( strcmp(map_name,"SavePoint") == 0 )
			pc->setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, CLR_TELEPORT);
		else
			pc->setpos(sd, script->mapindexname2id(st,map_name), x, y, CLR_OUTSIGHT);
	}
	mapreg->setreg(script->add_str("$@warpwaitingpcnum"), i);
	return true;
}

/////////////////////////////////////////////////////////////////////
// ...
//

/// Detaches a character from a script.
///
/// @param st Script state to detach the character from.
void script_detach_rid(struct script_state* st) {
	if(st->rid) {
		script->detach_state(st, false);
		st->rid = 0;
	}
}

/*==========================================
 * Attach sd char id to script and detach current one if any
 *------------------------------------------*/
BUILDIN(attachrid) {
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
BUILDIN(detachrid)
{
	script->detach_rid(st);
	return true;
}
/*==========================================
 * Chk if account connected, (and charid from account if specified)
 *------------------------------------------*/
BUILDIN(isloggedin)
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
BUILDIN(setmapflagnosave) {
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

BUILDIN(getmapflag)
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
			case MF_NOVIEWID:           script_pushint(st,map->list[m].flag.noviewid); break;
		}
	}

	return true;
}
/* pvp timer handling */
int script_mapflag_pvp_sub(struct block_list *bl, va_list ap)
{
	struct map_session_data *sd = NULL;

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_PC);
	sd = BL_UCAST(BL_PC, bl);

	if (sd->pvp_timer == INVALID_TIMER) {
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

BUILDIN(setmapflag) {
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
			case MF_NOVIEWID:           map->list[m].flag.noviewid = (val <= 0) ? EQP_NONE : val; break;
		}
	}

	return true;
}

BUILDIN(removemapflag) {
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
			case MF_NOVIEWID:           map->list[m].flag.noviewid = EQP_NONE; break;
		}
	}

	return true;
}

BUILDIN(pvpon)
{
	int16 m;
	const char *str;
	struct map_session_data *sd = NULL;
	struct s_mapiterator* iter;
	struct block_list bl;

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

int buildin_pvpoff_sub(struct block_list *bl, va_list ap)
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

BUILDIN(pvpoff) {
	int16 m;
	const char *str;
	struct block_list bl;

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

BUILDIN(gvgon) {
	int16 m;
	const char *str;

	str=script_getstr(st,2);
	m = map->mapname2mapid(str);
	if(m >= 0 && !map->list[m].flag.gvg) {
		struct block_list bl;

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
BUILDIN(gvgoff) {
	int16 m;
	const char *str;

	str=script_getstr(st,2);
	m = map->mapname2mapid(str);
	if(m >= 0 && map->list[m].flag.gvg) {
		struct block_list bl;
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
BUILDIN(emotion) {
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

int buildin_maprespawnguildid_sub_pc(struct map_session_data* sd, va_list ap)
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

int buildin_maprespawnguildid_sub_mob(struct block_list *bl, va_list ap)
{
	struct mob_data *md = NULL;

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_MOB);
	md = BL_UCAST(BL_MOB, bl);

	if (md->guardian_data == NULL && md->class_ != MOBID_EMPELIUM)
		status_kill(bl);

	return 0;
}

BUILDIN(maprespawnguildid) {
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

BUILDIN(agitstart) {
	if(map->agit_flag==1) return true;      // Agit already Start.
	map->agit_flag=1;
	guild->agit_start();
	return true;
}

BUILDIN(agitend) {
	if(map->agit_flag==0) return true;      // Agit already End.
	map->agit_flag=0;
	guild->agit_end();
	return true;
}

BUILDIN(agitstart2) {
	if(map->agit2_flag==1) return true;      // Agit2 already Start.
	map->agit2_flag=1;
	guild->agit2_start();
	return true;
}

BUILDIN(agitend2) {
	if(map->agit2_flag==0) return true;      // Agit2 already End.
	map->agit2_flag=0;
	guild->agit2_end();
	return true;
}

/*==========================================
 * Returns whether woe is on or off.
 *------------------------------------------*/
BUILDIN(agitcheck) {
	script_pushint(st,map->agit_flag);
	return true;
}

/*==========================================
 * Returns whether woese is on or off.
 *------------------------------------------*/
BUILDIN(agitcheck2) {
	script_pushint(st,map->agit2_flag);
	return true;
}

/// Sets the guild_id of this npc.
///
/// flagemblem <guild_id>;
BUILDIN(flagemblem)
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

BUILDIN(getcastlename)
{
	const char* mapname = mapindex->getmapname(script_getstr(st,2),NULL);
	struct guild_castle* gc = guild->mapname2gc(mapname);
	const char* name = (gc) ? gc->castle_name : "";
	script_pushstrcopy(st,name);
	return true;
}

BUILDIN(getcastledata)
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

BUILDIN(setcastledata)
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
BUILDIN(requestguildinfo)
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
BUILDIN(getequipcardcnt)
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
/// successremovecards <slot>;
BUILDIN(successremovecards)
{
	int i=-1,c,cardflag=0;

	struct map_session_data *sd = script->rid2sd(st);
	int num = script_getnum(st,2);

	if (sd == NULL)
		return true;

	if (num > 0 && num <= ARRAYLENGTH(script->equip))
		i=pc->checkequip(sd,script->equip[num-1]);

	if (i < 0 || !sd->inventory_data[i]) {
		return true;
	}

	if(itemdb_isspecial(sd->status.inventory[i].card[0]))
		return true;

	for( c = sd->inventory_data[i]->slot - 1; c >= 0; --c ) {
		if( sd->status.inventory[i].card[c] && itemdb_type(sd->status.inventory[i].card[c]) == IT_CARD ) {// extract this card from the item
			int flag;
			struct item item_tmp;
			memset(&item_tmp,0,sizeof(item_tmp));
			cardflag = 1;
			item_tmp.nameid   = sd->status.inventory[i].card[c];
			item_tmp.identify = 1;

			if((flag=pc->additem(sd,&item_tmp,1,LOG_TYPE_SCRIPT))) {
				// get back the cart in inventory
				clif->additem(sd,0,0,flag);
				map->addflooritem(&sd->bl, &item_tmp, 1, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0);
			}
		}
	}

	if (cardflag == 1) {
		//if card was remove replace item with no card
		int flag, j;
		struct item item_tmp;
		memset(&item_tmp,0,sizeof(item_tmp));

		item_tmp.nameid      = sd->status.inventory[i].nameid;
		item_tmp.identify    = 1;
		item_tmp.refine      = sd->status.inventory[i].refine;
		item_tmp.attribute   = sd->status.inventory[i].attribute;
		item_tmp.expire_time = sd->status.inventory[i].expire_time;
		item_tmp.bound       = sd->status.inventory[i].bound;

		for (j = sd->inventory_data[i]->slot; j < MAX_SLOTS; j++)
			item_tmp.card[j]=sd->status.inventory[i].card[j];

		pc->delitem(sd, i, 1, 0, DELITEM_MATERIALCHANGE, LOG_TYPE_SCRIPT);
		if ((flag=pc->additem(sd,&item_tmp,1,LOG_TYPE_SCRIPT))) {
			//chk if can be spawn in inventory otherwise put on floor
			clif->additem(sd,0,0,flag);
			map->addflooritem(&sd->bl, &item_tmp, 1, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0);
		}

		clif->misceffect(&sd->bl,3);
	}
	return true;
}

/// Removes all cards from the item found in the specified equipment slot of the invoking character.
/// failedremovecards <slot>, <type>;
/// <type>=0 : will destroy both the item and the cards.
/// <type>=1 : will keep the item, but destroy the cards.
/// <type>=2 : will keep the cards, but destroy the item.
/// <type>=? : will just display the failure effect.
BUILDIN(failedremovecards)
{
	int i=-1,c,cardflag=0;

	struct map_session_data *sd = script->rid2sd(st);
	int num = script_getnum(st,2);
	int typefail = script_getnum(st,3);

	if (sd == NULL)
		return true;

	if (num > 0 && num <= ARRAYLENGTH(script->equip))
		i=pc->checkequip(sd,script->equip[num-1]);

	if (i < 0 || !sd->inventory_data[i])
		return true;

	if(itemdb_isspecial(sd->status.inventory[i].card[0]))
		return true;

	for( c = sd->inventory_data[i]->slot - 1; c >= 0; --c ) {
		if( sd->status.inventory[i].card[c] && itemdb_type(sd->status.inventory[i].card[c]) == IT_CARD ) {
			cardflag = 1;

			if(typefail == 2) {// add cards to inventory, clear
				int flag;
				struct item item_tmp;

				memset(&item_tmp,0,sizeof(item_tmp));

				item_tmp.nameid   = sd->status.inventory[i].card[c];
				item_tmp.identify = 1;

				if((flag=pc->additem(sd,&item_tmp,1,LOG_TYPE_SCRIPT))) {
					clif->additem(sd,0,0,flag);
					map->addflooritem(&sd->bl, &item_tmp, 1, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0);
				}
			}
		}
	}

	if (cardflag == 1) {
		if (typefail == 0 || typefail == 2) {
			// destroy the item
			pc->delitem(sd, i, 1, 0, DELITEM_FAILREFINE, LOG_TYPE_SCRIPT);
		} else if (typefail == 1) {
			// destroy the card
			int flag, j;
			struct item item_tmp;

			memset(&item_tmp,0,sizeof(item_tmp));

			item_tmp.nameid      = sd->status.inventory[i].nameid;
			item_tmp.identify    = 1;
			item_tmp.refine      = sd->status.inventory[i].refine;
			item_tmp.attribute   = sd->status.inventory[i].attribute;
			item_tmp.expire_time = sd->status.inventory[i].expire_time;
			item_tmp.bound       = sd->status.inventory[i].bound;

			for (j = sd->inventory_data[i]->slot; j < MAX_SLOTS; j++)
				item_tmp.card[j]=sd->status.inventory[i].card[j];

			pc->delitem(sd, i, 1, 0, DELITEM_FAILREFINE, LOG_TYPE_SCRIPT);

			if((flag=pc->additem(sd,&item_tmp,1,LOG_TYPE_SCRIPT))) {
				clif->additem(sd,0,0,flag);
				map->addflooritem(&sd->bl, &item_tmp, 1, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0);
			}
		}
		clif->misceffect(&sd->bl,2);
	}

	return true;
}

/* ================================================================
 * mapwarp "<from map>","<to map>",<x>,<y>,<type>,<ID for Type>;
 * type: 0=everyone, 1=guild, 2=party; [Reddozen]
 * improved by [Lance]
 * ================================================================*/
// Added by RoVeRT
BUILDIN(mapwarp) {
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
int buildin_mobcount_sub(struct block_list *bl, va_list ap)
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
BUILDIN(mobcount) {
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

BUILDIN(marriage) {
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
BUILDIN(wedding_effect)
{
	struct map_session_data *sd = script->rid2sd(st);
	struct block_list *bl;

	if (sd == NULL)
		return true; //bl=map->id2bl(st->oid);

	bl=&sd->bl;
	clif->wedding_effect(bl);
	return true;
}
BUILDIN(divorce)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL || pc->divorce(sd) < 0) {
		script_pushint(st,0);
		return true;
	}
	script_pushint(st,1);
	return true;
}

BUILDIN(ispartneron) {
	struct map_session_data *sd = script->rid2sd(st);

	if (sd==NULL || !pc->ismarried(sd)
	 || map->charid2sd(sd->status.partner_id) == NULL) {
		script_pushint(st,0);
		return true;
	}

	script_pushint(st,1);
	return true;
}

BUILDIN(getpartnerid)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	script_pushint(st,sd->status.partner_id);
	return true;
}

BUILDIN(getchildid)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	script_pushint(st,sd->status.child);
	return true;
}

BUILDIN(getmotherid)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	script_pushint(st,sd->status.mother);
	return true;
}

BUILDIN(getfatherid) {
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	script_pushint(st,sd->status.father);
	return true;
}

BUILDIN(warppartner)
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
BUILDIN(strmobinfo)
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

/*==========================================
 * Summon guardians [Valaris]
 * guardian("<map name>",<x>,<y>,"<name to show>",<mob id>{,"<event label>"}{,<guardian index>}) -> <id>
 *------------------------------------------*/
BUILDIN(guardian) {
	int class_ = 0, x = 0, y = 0, guardian = 0;
	const char *str, *mapname, *evt="";
	bool has_index = false;

	mapname = script_getstr(st,2);
	x       = script_getnum(st,3);
	y       = script_getnum(st,4);
	str     = script_getstr(st,5);
	class_  = script_getnum(st,6);

	if( script_hasdata(st,8) )
	{// "<event label>",<guardian index>
		evt=script_getstr(st,7);
		guardian=script_getnum(st,8);
		has_index = true;
	} else if( script_hasdata(st,7) ) {
		struct script_data *data = script_getdata(st,7);
		script->get_val(st,data); // Dereference if it's a variable
		if( data_isstring(data) ) {
			// "<event label>"
			evt=script_getstr(st,7);
		} else if( data_isint(data) ) {
			// <guardian index>
			guardian=script_getnum(st,7);
			has_index = true;
		} else {
			ShowError("script:guardian: invalid data type for argument #6 (from 1)\n");
			script->reportdata(data);
			return false;
		}
	}

	script->check_event(st, evt);
	script_pushint(st, mob->spawn_guardian(mapname,x,y,str,class_,evt,guardian,has_index));

	return true;
}
/*==========================================
 * Invisible Walls [Zephyrus]
 *------------------------------------------*/
BUILDIN(setwall) {
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
BUILDIN(delwall) {
	const char *name = script_getstr(st,2);
	map->iwall_remove(name);

	return true;
}

/// Retrieves various information about the specified guardian.
///
/// guardianinfo("<map_name>", <index>, <type>) -> <value>
/// type: 0 - whether it is deployed or not
///       1 - maximum hp
///       2 - current hp
///
BUILDIN(guardianinfo) {
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
BUILDIN(getitemname) {
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
BUILDIN(getitemslots)
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

// TODO: add matk here if needed/once we get rid of RENEWAL

/*==========================================
 * Returns some values of an item [Lupus]
 * Price, Weight, etc...
 * getiteminfo(itemID,n), where n
 * 0 value_buy;
 * 1 value_sell;
 * 2 type;
 * 3 maxchance = Max drop chance of this item e.g. 1 = 0.01% , etc..
 * if = 0, then monsters don't drop it at all (rare or a quest item)
 * if = -1, then this item is sold in NPC shops only
 * 4 sex;
 * 5 equip;
 * 6 weight;
 * 7 atk;
 * 8 def;
 * 9 range;
 * 10 slot;
 * 11 look;
 * 12 elv;
 * 13 wlv;
 * 14 view id
 *------------------------------------------*/
BUILDIN(getiteminfo)
{
	int item_id,n;
	struct item_data *i_data;

	item_id = script_getnum(st,2);
	n       = script_getnum(st,3);
	i_data  = itemdb->exists(item_id);

	if (i_data && n>=0 && n<=14) {
		int *item_arr =  (int*)&i_data->value_buy;
		script_pushint(st,item_arr[n]);
	} else {
		script_pushint(st,-1);
	}
	return true;
}

/*==========================================
 * Set some values of an item [Lupus]
 * Price, Weight, etc...
 * setiteminfo(itemID,n,Value), where n
 * 0 value_buy;
 * 1 value_sell;
 * 2 type;
 * 3 maxchance = Max drop chance of this item e.g. 1 = 0.01% , etc..
 * if = 0, then monsters don't drop it at all (rare or a quest item)
 * if = -1, then this item is sold in NPC shops only
 * 4 sex;
 * 5 equip;
 * 6 weight;
 * 7 atk;
 * 8 def;
 * 9 range;
 * 10 slot;
 * 11 look;
 * 12 elv;
 * 13 wlv;
 * 14 view id
 * Returns Value or -1 if the wrong field's been set
 *------------------------------------------*/
BUILDIN(setiteminfo)
{
	int item_id,n,value;
	struct item_data *i_data;

	item_id = script_getnum(st,2);
	n       = script_getnum(st,3);
	value   = script_getnum(st,4);
	i_data  = itemdb->exists(item_id);

	if (i_data && n>=0 && n<=14) {
		int *item_arr = (int*)&i_data->value_buy;
		item_arr[n] = value;
		script_pushint(st,value);
	} else {
		script_pushint(st,-1);
	}
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
BUILDIN(getequipcardid)
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
BUILDIN(petskillbonus)
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
BUILDIN(petloot)
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
 * @inventorylist_card(0..3), @inventorylist_expire
 * @inventorylist_count = scalar
 *------------------------------------------*/
BUILDIN(getinventorylist)
{
	struct map_session_data *sd = script->rid2sd(st);
	char card_var[NAME_LENGTH];

	int i,j=0,k;
	if(!sd) return true;

	for(i=0;i<MAX_INVENTORY;i++) {
		if(sd->status.inventory[i].nameid > 0 && sd->status.inventory[i].amount > 0) {
			pc->setreg(sd,reference_uid(script->add_str("@inventorylist_id"), j),sd->status.inventory[i].nameid);
			pc->setreg(sd,reference_uid(script->add_str("@inventorylist_amount"), j),sd->status.inventory[i].amount);
			if(sd->status.inventory[i].equip) {
				pc->setreg(sd,reference_uid(script->add_str("@inventorylist_equip"), j),pc->equippoint(sd,i));
			} else {
				pc->setreg(sd,reference_uid(script->add_str("@inventorylist_equip"), j),0);
			}
			pc->setreg(sd,reference_uid(script->add_str("@inventorylist_refine"), j),sd->status.inventory[i].refine);
			pc->setreg(sd,reference_uid(script->add_str("@inventorylist_identify"), j),sd->status.inventory[i].identify);
			pc->setreg(sd,reference_uid(script->add_str("@inventorylist_attribute"), j),sd->status.inventory[i].attribute);
			for (k = 0; k < MAX_SLOTS; k++) {
				sprintf(card_var, "@inventorylist_card%d",k+1);
				pc->setreg(sd,reference_uid(script->add_str(card_var), j),sd->status.inventory[i].card[k]);
			}
			pc->setreg(sd,reference_uid(script->add_str("@inventorylist_expire"), j),sd->status.inventory[i].expire_time);
			pc->setreg(sd,reference_uid(script->add_str("@inventorylist_bound"), j),sd->status.inventory[i].bound);
			j++;
		}
	}
	pc->setreg(sd,script->add_str("@inventorylist_count"),j);
	return true;
}

BUILDIN(getcartinventorylist)
{
	struct map_session_data *sd = script->rid2sd(st);
	char card_var[26];

	int i,j=0,k;
	if(!sd) return true;

	for(i=0;i<MAX_CART;i++) {
		if(sd->status.cart[i].nameid > 0 && sd->status.cart[i].amount > 0) {
			pc->setreg(sd,reference_uid(script->add_str("@cartinventorylist_id"), j),sd->status.cart[i].nameid);
			pc->setreg(sd,reference_uid(script->add_str("@cartinventorylist_amount"), j),sd->status.cart[i].amount);
			pc->setreg(sd,reference_uid(script->add_str("@cartinventorylist_equip"), j),sd->status.cart[i].equip);
			pc->setreg(sd,reference_uid(script->add_str("@cartinventorylist_refine"), j),sd->status.cart[i].refine);
			pc->setreg(sd,reference_uid(script->add_str("@cartinventorylist_identify"), j),sd->status.cart[i].identify);
			pc->setreg(sd,reference_uid(script->add_str("@cartinventorylist_attribute"), j),sd->status.cart[i].attribute);
			for (k = 0; k < MAX_SLOTS; k++) {
				sprintf(card_var, "@cartinventorylist_card%d",k+1);
				pc->setreg(sd,reference_uid(script->add_str(card_var), j),sd->status.cart[i].card[k]);
			}
			pc->setreg(sd,reference_uid(script->add_str("@cartinventorylist_expire"), j),sd->status.cart[i].expire_time);
			pc->setreg(sd,reference_uid(script->add_str("@cartinventorylist_bound"), j),sd->status.cart[i].bound);
			j++;
		}
	}
	pc->setreg(sd,script->add_str("@cartinventorylist_count"),j);
	return true;
}

BUILDIN(getskilllist)
{
	struct map_session_data *sd = script->rid2sd(st);
	int i,j=0;
	if (sd == NULL)
		return true;
	for(i=0;i<MAX_SKILL;i++) {
		if(sd->status.skill[i].id > 0 && sd->status.skill[i].lv > 0) {
			pc->setreg(sd,reference_uid(script->add_str("@skilllist_id"), j),sd->status.skill[i].id);
			pc->setreg(sd,reference_uid(script->add_str("@skilllist_lv"), j),sd->status.skill[i].lv);
			pc->setreg(sd,reference_uid(script->add_str("@skilllist_flag"), j),sd->status.skill[i].flag);
			j++;
		}
	}
	pc->setreg(sd,script->add_str("@skilllist_count"),j);
	return true;
}

BUILDIN(clearitem)
{
	struct map_session_data *sd = script->rid2sd(st);
	int i;
	if (sd == NULL)
		return true;
	for (i=0; i<MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].amount) {
			pc->delitem(sd, i, sd->status.inventory[i].amount, 0, DELITEM_NORMAL, LOG_TYPE_SCRIPT);
		}
	}
	return true;
}

/*==========================================
 * Disguise Player (returns Mob/NPC ID if success, 0 on fail)
 *------------------------------------------*/
BUILDIN(disguise)
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
BUILDIN(undisguise)
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
BUILDIN(classchange) {
	int class_,type;
	struct block_list *bl=map->id2bl(st->oid);

	if(bl==NULL) return true;

	class_=script_getnum(st,2);
	type=script_getnum(st,3);
	clif->class_change(bl,class_,type);
	return true;
}

/*==========================================
 * Display an effect
 *------------------------------------------*/
BUILDIN(misceffect)
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
BUILDIN(playbgm)
{
	struct map_session_data* sd = script->rid2sd(st);

	if (sd != NULL) {
		const char *name = script_getstr(st,2);

		clif->playBGM(sd, name);
	}

	return true;
}

int playbgm_sub(struct block_list* bl,va_list ap)
{
	const char* name = va_arg(ap,const char*);

	clif->playBGM(BL_CAST(BL_PC, bl), name);

	return 0;
}

int playbgm_foreachpc_sub(struct map_session_data* sd, va_list args)
{
	const char* name = va_arg(args, const char*);

	clif->playBGM(sd, name);
	return 0;
}

/*==========================================
 * Play a BGM on multiple client [Rikter/Yommy]
 *------------------------------------------*/
BUILDIN(playbgmall) {
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
BUILDIN(soundeffect)
{
	struct map_session_data *sd = script->rid2sd(st);
	const char* name = script_getstr(st,2);
	int type = script_getnum(st,3);

	if (sd != NULL) {
		clif->soundeffect(sd,&sd->bl,name,type);
	}
	return true;
}

int soundeffect_sub(struct block_list *bl, va_list ap)
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
BUILDIN(soundeffectall) {
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
BUILDIN(petrecovery)
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
BUILDIN(petskillattack)
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
BUILDIN(petskillsupport)
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
BUILDIN(skilleffect)
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
BUILDIN(npcskilleffect) {
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
BUILDIN(specialeffect) {
	struct block_list *bl=map->id2bl(st->oid);
	int type = script_getnum(st,2);
	enum send_target target = script_hasdata(st,3) ? (send_target)script_getnum(st,3) : AREA;

	if(bl==NULL)
		return true;

	if (script_hasdata(st,4)) {
		struct npc_data *nd = npc->name2id(script_getstr(st,4));
		if (nd != NULL)
			clif->specialeffect(&nd->bl, type, target);
	} else {
		if (target == SELF) {
			struct map_session_data *sd = script->rid2sd(st);
			if (sd != NULL)
				clif->specialeffect_single(bl,type,sd->fd);
		} else {
			clif->specialeffect(bl, type, target);
		}
	}

	return true;
}

BUILDIN(specialeffect2) {
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

/*==========================================
 * Nude [Valaris]
 *------------------------------------------*/
BUILDIN(nude)
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
BUILDIN(atcommand)
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
BUILDIN(dispbottom)
{
	struct map_session_data *sd = script->rid2sd(st);
	const char *message = script_getstr(st,2);

	if (sd == NULL)
		return true;

	if (script_hasdata(st,3)) {
		int color = script_getnum(st,3);
		clif->messagecolor_self(sd->fd, color, message);
	} else {
		clif_disp_onlyself(sd, message, (int)strlen(message));
	}

	return true;
}

/*==========================================
 * All The Players Full Recovery
 * (HP/SP full restore and resurrect if need)
 *------------------------------------------*/
BUILDIN(recovery)
{
	struct map_session_data *sd;
	struct s_mapiterator* iter;

	iter = mapit_getallusers();
	for (sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); sd = BL_UCAST(BL_PC, mapit->next(iter))) {
		if(pc_isdead(sd))
			status->revive(&sd->bl, 100, 100);
		else
			status_percent_heal(&sd->bl, 100, 100);
		clif->message(sd->fd,msg_sd(sd,880)); // "You have been recovered!"
	}
	mapit->free(iter);
	return true;
}
/*==========================================
 * Get your pet info: getpetinfo(n)
 * n -> 0:pet_id 1:pet_class 2:pet_name
 * 3:friendly 4:hungry, 5: rename flag.
 *------------------------------------------*/
BUILDIN(getpetinfo)
{
	struct map_session_data *sd = script->rid2sd(st);
	struct pet_data *pd;
	int type=script_getnum(st,2);

	if (sd == NULL || sd->pd == NULL) {
		if (type == 2)
			script_pushconststr(st,"null");
		else
			script_pushint(st,0);
		return true;
	}
	pd = sd->pd;
	switch(type) {
		case 0: script_pushint(st,pd->pet.pet_id); break;
		case 1: script_pushint(st,pd->pet.class_); break;
		case 2: script_pushstrcopy(st,pd->pet.name); break;
		case 3: script_pushint(st,pd->pet.intimate); break;
		case 4: script_pushint(st,pd->pet.hungry); break;
		case 5: script_pushint(st,pd->pet.rename_flag); break;
		default:
			script_pushint(st,0);
			break;
	}
	return true;
}

/*==========================================
 * Get your homunculus info: gethominfo(n)
 * n -> 0:hom_id 1:class 2:name
 * 3:friendly 4:hungry, 5: rename flag.
 * 6: level
 *------------------------------------------*/
BUILDIN(gethominfo)
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

/// Retrieves information about character's mercenary
/// getmercinfo <type>[,<char id>];
BUILDIN(getmercinfo)
{
	int type;
	struct map_session_data* sd;
	struct mercenary_data* md;

	type = script_getnum(st,2);

	if (script_hasdata(st,3)) {
		int char_id = script_getnum(st,3);

		if ((sd = script->charid2sd(st, char_id)) == NULL) {
			script_pushnil(st);
			return true;
		}
	} else {
		if ((sd = script->rid2sd(st)) == NULL)
			return true;
	}

	md = ( sd->status.mer_id && sd->md ) ? sd->md : NULL;

	switch( type )
	{
		case 0: script_pushint(st,md ? md->mercenary.mercenary_id : 0); break;
		case 1: script_pushint(st,md ? md->mercenary.class_ : 0); break;
		case 2:
			if( md )
				script_pushstrcopy(st,md->db->name);
			else
				script_pushconststr(st,"");
			break;
		case 3: script_pushint(st,md ? mercenary->get_faith(md) : 0); break;
		case 4: script_pushint(st,md ? mercenary->get_calls(md) : 0); break;
		case 5: script_pushint(st,md ? md->mercenary.kill_count : 0); break;
		case 6: script_pushint(st,md ? mercenary->get_lifetime(md) : 0); break;
		case 7: script_pushint(st,md ? md->db->lv : 0); break;
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
BUILDIN(checkequipedcard)
{
	int n,i,c=0;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	c = script_getnum(st,2);

	for( i=0; i<MAX_INVENTORY; i++) {
		if(sd->status.inventory[i].nameid > 0 && sd->status.inventory[i].amount && sd->inventory_data[i]) {
			if (itemdb_isspecial(sd->status.inventory[i].card[0]))
				continue;
			for(n=0;n<sd->inventory_data[i]->slot;n++) {
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

BUILDIN(__jump_zero)
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
BUILDIN(movenpc)
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
BUILDIN(message)
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

/*==========================================
 * npctalk (sends message to surrounding area)
 * usage: npctalk "<message>"{,"<npc name>"};
 *------------------------------------------*/
BUILDIN(npctalk)
{
	struct npc_data* nd;
	const char *str = script_getstr(st,2);

	if (script_hasdata(st, 3)) {
		nd = npc->name2id(script_getstr(st, 3));
	} else {
		nd = map->id2nd(st->oid);
	}

	if (nd != NULL) {
		char name[NAME_LENGTH], message[256];
		safestrncpy(name, nd->name, sizeof(name));
		strtok(name, "#"); // discard extra name identifier if present
		safesnprintf(message, sizeof(message), "%s : %s", name, str);
		clif->disp_overhead(&nd->bl, message);
	}

	return true;
}

// change npc walkspeed [Valaris]
BUILDIN(npcspeed) {
	struct npc_data* nd;
	int speed;

	speed = script_getnum(st,2);
	nd = map->id2nd(st->oid);

	if (nd != NULL) {
		unit->bl2ud2(&nd->bl); // ensure nd->ud is safe to edit
		nd->speed = speed;
		nd->ud->state.speed_changed = 1;
	}

	return true;
}
// make an npc walk to a position [Valaris]
BUILDIN(npcwalkto)
{
	struct npc_data *nd = map->id2nd(st->oid);
	int x=0,y=0;

	x=script_getnum(st,2);
	y=script_getnum(st,3);

	if (nd != NULL) {
		unit->bl2ud2(&nd->bl); // ensure nd->ud is safe to edit
		if (!nd->status.hp) {
			status_calc_npc(nd, SCO_FIRST);
		} else {
			status_calc_npc(nd, SCO_NONE);
		}
		unit->walktoxy(&nd->bl,x,y,0);
	}

	return true;
}
// stop an npc's movement [Valaris]
BUILDIN(npcstop)
{
	struct npc_data *nd = map->id2nd(st->oid);

	if (nd != NULL) {
		unit->bl2ud2(&nd->bl); // ensure nd->ud is safe to edit
		unit->stop_walking(&nd->bl, STOPWALKING_FLAG_FIXPOS|STOPWALKING_FLAG_NEXTCELL);
	}

	return true;
}

// set click npc distance [4144]
BUILDIN(setnpcdistance)
{
	struct npc_data *nd = map->id2nd(st->oid);
	if (nd == NULL)
		return false;

	nd->area_size = script_getnum(st, 2);

	return true;
}

// return current npc direction [4144]
BUILDIN(getnpcdir)
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
BUILDIN(setnpcdir)
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
BUILDIN(getnpcclass)
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
BUILDIN(getlook)
{
	int type,val = -1;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	type=script_getnum(st,2);
	switch(type) {
		case LOOK_HAIR:          val = sd->status.hair;          break; //1
		case LOOK_WEAPON:        val = sd->status.weapon;        break; //2
		case LOOK_HEAD_BOTTOM:   val = sd->status.head_bottom;   break; //3
		case LOOK_HEAD_TOP:      val = sd->status.head_top;      break; //4
		case LOOK_HEAD_MID:      val = sd->status.head_mid;      break; //5
		case LOOK_HAIR_COLOR:    val = sd->status.hair_color;    break; //6
		case LOOK_CLOTHES_COLOR: val = sd->status.clothes_color; break; //7
		case LOOK_SHIELD:        val = sd->status.shield;        break; //8
		case LOOK_SHOES:                                         break; //9
		case LOOK_ROBE:          val = sd->status.robe;          break; //12
		case LOOK_BODY2:         val=sd->status.body;            break; //13
	}

	script_pushint(st,val);
	return true;
}

/*==========================================
 *     get char save point. argument: 0- map name, 1- x, 2- y
 *------------------------------------------*/
BUILDIN(getsavepoint)
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
BUILDIN(getmapxy)
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
					sd = map->nick2sd(script_getstr(st,6));
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
					sd = map->nick2sd(script_getstr(st,6));
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
					sd = map->nick2sd(script_getstr(st,6));
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
					sd = map->nick2sd(script_getstr(st,6));
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
					sd = map->nick2sd(script_getstr(st,6));
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
	script->set_reg(st,sd,num,name,(void*)mapname,script_getref(st,2));

	//Set MapX
	num=st->stack->stack_data[st->start+3].u.num;
	name=script->get_str(script_getvarid(num));
	prefix=*name;

	if(not_server_variable(prefix))
		sd=script->rid2sd(st);
	else
		sd=NULL;
	script->set_reg(st,sd,num,name,(void*)h64BPTRSIZE(x),script_getref(st,3));

	//Set MapY
	num=st->stack->stack_data[st->start+4].u.num;
	name=script->get_str(script_getvarid(num));
	prefix=*name;

	if(not_server_variable(prefix))
		sd=script->rid2sd(st);
	else
		sd=NULL;
	script->set_reg(st,sd,num,name,(void*)h64BPTRSIZE(y),script_getref(st,4));

	//Return Success value
	script_pushint(st,0);
	return true;
}

/*==========================================
 * Allows player to write NPC logs (i.e. Bank NPC, etc) [Lupus]
 *------------------------------------------*/
BUILDIN(logmes)
{
	const char *str;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	str = script_getstr(st,2);
	logs->npc(sd,str);
	return true;
}

BUILDIN(summon)
{
	int class_, timeout=0;
	const char *str,*event="";
	struct mob_data *md;
	int64 tick = timer->gettick();
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	str    = script_getstr(st,2);
	class_ = script_getnum(st,3);
	if( script_hasdata(st,4) )
		timeout=script_getnum(st,4);
	if( script_hasdata(st,5) ) {
		event=script_getstr(st,5);
		script->check_event(st, event);
	}

	clif->skill_poseffect(&sd->bl,AM_CALLHOMUN,1,sd->bl.x,sd->bl.y,tick);

	md = mob->once_spawn_sub(&sd->bl, sd->bl.m, sd->bl.x, sd->bl.y, str, class_, event, SZ_SMALL, AI_NONE);
	if (md) {
		md->master_id=sd->bl.id;
		md->special_state.ai = AI_ATTACK;
		if( md->deletetimer != INVALID_TIMER )
			timer->delete(md->deletetimer, mob->timer_delete);
		md->deletetimer = timer->add(tick+(timeout>0?timeout*1000:60000),mob->timer_delete,md->bl.id,0);
		mob->spawn (md); //Now it is ready for spawning.
		clif->specialeffect(&md->bl,344,AREA);
		sc_start4(NULL, &md->bl, SC_MODECHANGE, 100, 1, 0, MD_AGGRESSIVE, 0, 60000);
	}
	return true;
}

/*==========================================
 * Checks whether it is daytime/nighttime
 *------------------------------------------*/
BUILDIN(isnight) {
	script_pushint(st,(map->night_flag == 1));
	return true;
}

/*================================================
 * Check how many items/cards in the list are
 * equipped - used for 2/15's cards patch [celest]
 *------------------------------------------------*/
BUILDIN(isequippedcnt)
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
BUILDIN(isequipped)
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
BUILDIN(cardscnt)
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
BUILDIN(getrefine)
{
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	script_pushint(st,sd->status.inventory[status->current_equip_item_index].refine);
	return true;
}

/*=======================================================
 * Day/Night controls
 *-------------------------------------------------------*/
BUILDIN(night) {
	if (map->night_flag != 1) pc->map_night_timer(pc->night_timer_tid, 0, 0, 1);
	return true;
}
BUILDIN(day) {
	if (map->night_flag != 0) pc->map_day_timer(pc->day_timer_tid, 0, 0, 1);
	return true;
}

//=======================================================
// Unequip [Spectre]
//-------------------------------------------------------
BUILDIN(unequip)
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

BUILDIN(equip)
{
	int nameid=0,i;
	struct item_data *item_data;
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return false;

	nameid=script_getnum(st,2);
	if((item_data = itemdb->exists(nameid)) == NULL)
	{
		ShowError("wrong item ID : equipitem(%i)\n",nameid);
		return false;
	}
	ARR_FIND( 0, MAX_INVENTORY, i, sd->status.inventory[i].nameid == nameid && sd->status.inventory[i].equip == 0 );
	if( i < MAX_INVENTORY )
		pc->equipitem(sd,i,item_data->equip);

	return true;
}

BUILDIN(autoequip)
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
BUILDIN(equip2)
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

	ref    = script_getnum(st,3);
	attr   = script_getnum(st,4);
	c0     = (short)script_getnum(st,5);
	c1     = (short)script_getnum(st,6);
	c2     = (short)script_getnum(st,7);
	c3     = (short)script_getnum(st,8);

	ARR_FIND( 0, MAX_INVENTORY, i,( sd->status.inventory[i].equip == 0 &&
									sd->status.inventory[i].nameid == nameid &&
									sd->status.inventory[i].refine == ref &&
									sd->status.inventory[i].attribute == attr &&
									sd->status.inventory[i].card[0] == c0 &&
									sd->status.inventory[i].card[1] == c1 &&
									sd->status.inventory[i].card[2] == c2 &&
									sd->status.inventory[i].card[3] == c3 ) );

	if( i < MAX_INVENTORY ) {
		script_pushint(st,1);
		pc->equipitem(sd,i,item_data->equip);
	}
	else
		script_pushint(st,0);

	return true;
}

BUILDIN(setbattleflag)
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

BUILDIN(getbattleflag)
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
BUILDIN(getstrlen)
{

	const char *str = script_getstr(st,2);
	int len = (str) ? (int)strlen(str) : 0;

	script_pushint(st,len);
	return true;
}

//=======================================================
// isalpha [Valaris]
//-------------------------------------------------------
BUILDIN(charisalpha)
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
BUILDIN(charisupper)
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
BUILDIN(charislower)
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
BUILDIN(charat) {
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
// setchar <string>, <char>, <index>
//-------------------------------------------------------
BUILDIN(setchar)
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
BUILDIN(insertchar)
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
BUILDIN(delchar)
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
BUILDIN(strtoupper)
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
BUILDIN(strtolower)
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
BUILDIN(substr)
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
BUILDIN(explode)
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
			script->set_reg(st, sd, reference_uid(id, start + k), name, (void*)temp, reference_getref(data));
			k++;
			j = 0;
		} else {
			temp[j++] = str[i];
		}
	}
	//set last string
	temp[j] = '\0';
	script->set_reg(st, sd, reference_uid(id, start + k), name, (void*)temp, reference_getref(data));

	aFree(temp);

	script_pushint(st, k + 1);
	return true;
}

//=======================================================
// implode <string_array>
// implode <string_array>, <glue>
//-------------------------------------------------------
BUILDIN(implode)
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
			temp = (char*) script->get_val2(st, reference_uid(id, i), reference_getref(data));
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
			temp = (char*) script->get_val2(st, reference_uid(id, i), reference_getref(data));
			len = strlen(temp);
			memcpy(&output[k], temp, len);
			k += len;
			if(glue_len != 0) {
				memcpy(&output[k], glue, glue_len);
				k += glue_len;
			}
			script_removetop(st, -1, 0);
		}
		temp = (char*) script->get_val2(st, reference_uid(id, array_size), reference_getref(data));
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
BUILDIN(sprintf) {
	unsigned int argc = 0, arg = 0;
	const char* format;
	char* p;
	char* q;
	char* buf  = NULL;
	char* buf2 = NULL;
	struct script_data* data;
	size_t len, buf2_len = 0;
	StringBuf final_buf;

	// Fetch init data
	format = script_getstr(st, 2);
	argc = script_lastdata(st)-2;
	len = strlen(format);

	// Skip parsing, where no parsing is required.
	if(len==0) {
		script_pushconststr(st,"");
		return true;
	}

	// Pessimistic alloc
	CREATE(buf, char, len+1);

	// Need not be parsed, just solve stuff like %%.
	if(argc==0) {
		memcpy(buf,format,len+1);
		script_pushstrcopy(st, buf);
		aFree(buf);
		return true;
	}

	safestrncpy(buf, format, len+1);

	// Issue sprintf for each parameter
	StrBuf->Init(&final_buf);
	q = buf;
	while((p = strchr(q, '%'))!=NULL) {
		if(p!=q) {
			len = p-q+1;
			if(buf2_len<len) {
				RECREATE(buf2, char, len);
				buf2_len = len;
			}
			safestrncpy(buf2, q, len);
			StrBuf->AppendStr(&final_buf, buf2);
			q = p;
		}
		p = q+1;
		if(*p=='%') {  // %%
			StrBuf->AppendStr(&final_buf, "%");
			q+=2;
			continue;
		}
		if(*p=='n') {  // %n
			ShowWarning("buildin_sprintf: Format %%n not supported! Skipping...\n");
			script->reportsrc(st);
			q+=2;
			continue;
		}
		if(arg>=argc) {
			ShowError("buildin_sprintf: Not enough arguments passed!\n");
			aFree(buf);
			if(buf2) aFree(buf2);
			StrBuf->Destroy(&final_buf);
			script_pushconststr(st,"");
			return false;
		}
		if((p = strchr(q+1, '%'))==NULL) {
			p = strchr(q, 0);  // EOS
		}
		len = p-q+1;
		if(buf2_len<len) {
			RECREATE(buf2, char, len);
			buf2_len = len;
		}
		safestrncpy(buf2, q, len);
		q = p;

		// Note: This assumes the passed value being the correct
		// type to the current format specifier. If not, the server
		// probably crashes or returns anything else, than expected,
		// but it would behave in normal code the same way so it's
		// the scripter's responsibility.
		data = script_getdata(st, arg+3);
		if(data_isstring(data)) {  // String
			StrBuf->Printf(&final_buf, buf2, script_getstr(st, arg+3));
		} else if(data_isint(data)) {  // Number
			StrBuf->Printf(&final_buf, buf2, script_getnum(st, arg+3));
		} else if(data_isreference(data)) {  // Variable
			char* name = reference_getname(data);
			if(name[strlen(name)-1]=='$') {  // var Str
				StrBuf->Printf(&final_buf, buf2, script_getstr(st, arg+3));
			} else {  // var Int
				StrBuf->Printf(&final_buf, buf2, script_getnum(st, arg+3));
			}
		} else {  // Unsupported type
			ShowError("buildin_sprintf: Unknown argument type!\n");
			aFree(buf);
			if(buf2) aFree(buf2);
			StrBuf->Destroy(&final_buf);
			script_pushconststr(st,"");
			return false;
		}
		arg++;
	}

	// Append anything left
	if(*q) {
		StrBuf->AppendStr(&final_buf, q);
	}

	// Passed more, than needed
	if(arg<argc) {
		ShowWarning("buildin_sprintf: Unused arguments passed.\n");
		script->reportsrc(st);
	}

	script_pushstrcopy(st, StrBuf->Value(&final_buf));

	aFree(buf);
	if(buf2) aFree(buf2);
	StrBuf->Destroy(&final_buf);

	return true;
}

//=======================================================
// sscanf(<str>, <format>, ...);
// Implements C sscanf.
//-------------------------------------------------------
BUILDIN(sscanf) {
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
			script->set_reg(st, sd, reference_uid( reference_getid(data), reference_getindex(data) ), buf_p, (void *)(ref_str), reference_getref(data));
		} else {  // Number
			if(sscanf(str, buf, &ref_int)==0) {
				break;
			}
			script->set_reg(st, sd, reference_uid( reference_getid(data), reference_getindex(data) ), buf_p, (void *)h64BPTRSIZE(ref_int), reference_getref(data));
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
BUILDIN(strpos) {
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
BUILDIN(replacestr)
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
BUILDIN(countstr)
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
BUILDIN(setnpcdisplay) {
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

BUILDIN(atoi) {
	const char *value;
	value = script_getstr(st,2);
	script_pushint(st,atoi(value));
	return true;
}

BUILDIN(axtoi) {
	const char *hex = script_getstr(st,2);
	long value = strtol(hex, NULL, 16);
#if LONG_MAX > INT_MAX || LONG_MIN < INT_MIN
	value = cap_value(value, INT_MIN, INT_MAX);
#endif
	script_pushint(st, (int)value);
	return true;
}

BUILDIN(strtol) {
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
BUILDIN(compare)
{
	const char *message;
	const char *cmpstring;
	message = script_getstr(st,2);
	cmpstring = script_getstr(st,3);
	script_pushint(st,(stristr(message,cmpstring) != NULL));
	return true;
}

BUILDIN(strcmp)
{
	const char *str1 = script_getstr(st,2);
	const char *str2 = script_getstr(st,3);
	script_pushint(st,strcmp(str1, str2));
	return true;
}

// List of mathematics commands --->

BUILDIN(log10)
{
	double i, a;
	i = script_getnum(st,2);
	a = log10(i);
	script_pushint(st,(int)a);
	return true;
}

BUILDIN(sqrt) //[zBuffer]
{
	double i, a;
	i = script_getnum(st,2);
	a = sqrt(i);
	script_pushint(st,(int)a);
	return true;
}

BUILDIN(pow) //[zBuffer]
{
	double i, a, b;
	a = script_getnum(st,2);
	b = script_getnum(st,3);
	i = pow(a,b);
	script_pushint(st,(int)i);
	return true;
}

BUILDIN(distance) //[zBuffer]
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

BUILDIN(min)
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

BUILDIN(max)
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

BUILDIN(md5)
{
	const char *tmpstr;
	char *md5str;

	tmpstr = script_getstr(st,2);
	md5str = (char *)aMalloc((32+1)*sizeof(char));
	MD5_String(tmpstr, md5str);
	script_pushstr(st, md5str);
	return true;
}

BUILDIN(swap)
{
	struct map_session_data *sd = NULL;
	struct script_data *data1, *data2;
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

	if (is_string_variable(varname1)) {
		const char *value1, *value2;

		value1 = script_getstr(st,2);
		value2 = script_getstr(st,3);

		if (strcmpi(value1, value2)) {
			script->set_reg(st, sd, uid1, varname1, (void*)(value2), script_getref(st,3));
			script->set_reg(st, sd, uid2, varname2, (void*)(value1), script_getref(st,2));
		}
	}
	else {
		int value1, value2;

		value1 = script_getnum(st,2);
		value2 = script_getnum(st,3);

		if (value1 != value2) {
			script->set_reg(st, sd, uid1, varname1, (void*)h64BPTRSIZE(value2), script_getref(st,3));
			script->set_reg(st, sd, uid2, varname2, (void*)h64BPTRSIZE(value1), script_getref(st,2));
		}
	}
	return true;
}

// [zBuffer] List of dynamic var commands --->

BUILDIN(setd)
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

	if( is_string_variable(varname) ) {
		script->setd_sub(st, sd, varname, elem, (void *)script_getstr(st, 3), NULL);
	} else {
		script->setd_sub(st, sd, varname, elem, (void *)h64BPTRSIZE(script_getnum(st, 3)), NULL);
	}

	return true;
}

int buildin_query_sql_sub(struct script_state* st, Sql* handle)
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
BUILDIN(query_sql) {
	return script->buildin_query_sql_sub(st, map->mysql_handle);
}

BUILDIN(query_logsql) {
	if( !logs->config.sql_logs ) {// logs->mysql_handle == NULL
		ShowWarning("buildin_query_logsql: SQL logs are disabled, query '%s' will not be executed.\n", script_getstr(st,2));
		script_pushint(st,-1);
		return false;
	}
	return script->buildin_query_sql_sub(st, logs->mysql_handle);
}

//Allows escaping of a given string.
BUILDIN(escape_sql)
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

BUILDIN(getd) {
	char varname[100];
	const char *buffer;
	int elem;

	buffer = script_getstr(st, 2);

	if (sscanf(buffer, "%99[^[][%d]", varname, &elem) < 2)
		elem = 0;

	// Push the 'pointer' so it's more flexible [Lance]
	script->push_val(st->stack, C_NAME, reference_uid(script->add_str(varname), elem),NULL);

	return true;
}

// <--- [zBuffer] List of dynamic var commands
// Pet stat [Lance]
BUILDIN(petstat)
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

BUILDIN(callshop)
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

BUILDIN(npcshopitem)
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

BUILDIN(npcshopadditem)
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

BUILDIN(npcshopdelitem)
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
		if (n < size) {
			memmove(&nd->u.shop.shop_item[n], &nd->u.shop.shop_item[n+1], sizeof(nd->u.shop.shop_item[0])*(size-n));
			size--;
		}
	}

	RECREATE(nd->u.shop.shop_item, struct npc_item_list, size);
	nd->u.shop.count = size;

	script_pushint(st,1);
	return true;
}

//Sets a script to attach to a shop npc.
BUILDIN(npcshopattach) {
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
BUILDIN(setitemscript)
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
BUILDIN(addmonsterdrop) {
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
BUILDIN(delmonsterdrop) {
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
BUILDIN(getmonsterinfo)
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
		default: script_pushint(st,-1); //wrong Index
	}
	return true;
}

BUILDIN(checkvending) // check vending [Nab4]
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
BUILDIN(checkchatting) {
	struct map_session_data *sd = NULL;

	if (script_hasdata(st,2))
		sd = script->nick2sd(st, script_getstr(st,2));
	else
		sd = script->rid2sd(st);

	if (sd != NULL)
		script_pushint(st,(sd->chatID != 0));
	else
		script_pushint(st,0);

	return true;
}

BUILDIN(checkidle) {
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

BUILDIN(searchitem)
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
		count = itemdb->search_name_array(items, ARRAYLENGTH(items), itemname, 0);
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
		void* v = (void*)h64BPTRSIZE((int)items[i]->nameid);
		script->set_reg(st, sd, reference_uid(id, start), name, v, reference_getref(data));
	}

	script_pushint(st, count);
	return true;
}

// [zBuffer] List of player cont commands --->
BUILDIN(rid2name)
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

BUILDIN(pcblockmove) {
	int id, flag;
	struct map_session_data *sd = NULL;

	id = script_getnum(st,2);
	flag = script_getnum(st,3);

	if (id != 0)
		sd = script->id2sd(st, id);
	else
		sd = script->rid2sd(st);

	if (sd != NULL)
		sd->state.blockedmove = flag > 0;

	return true;
}

BUILDIN(pcfollow)
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

BUILDIN(pcstopfollow)
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

BUILDIN(getunittype) {
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

/// Makes the unit walk to target position or target id
/// Returns if it was successfull
///
/// unitwalk(<unit_id>,<x>,<y>) -> <bool>
/// unitwalk(<unit_id>,<target_id>) -> <bool>
BUILDIN(unitwalk) {
	struct block_list* bl;

	bl = map->id2bl(script_getnum(st,2));
	if( bl == NULL ) {
		script_pushint(st, 0);
		return true;
	}

	if( bl->type == BL_NPC ) {
		unit->bl2ud2(bl); // ensure the ((struct npc_data*)bl)->ud is safe to edit
	}
	if( script_hasdata(st,4) ) {
		int x = script_getnum(st,3);
		int y = script_getnum(st,4);
		script_pushint(st, unit->walktoxy(bl,x,y,0));// We'll use harder calculations.
	} else {
		int target_id = script_getnum(st,3);
		script_pushint(st, unit->walktobl(bl,map->id2bl(target_id),1,1));
	}

	return true;
}

/// Kills the unit
///
/// unitkill <unit_id>;
BUILDIN(unitkill)
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
BUILDIN(unitwarp) {
	int unit_id;
	int mapid;
	short x;
	short y;
	struct block_list* bl;
	const char *mapname;

	unit_id = script_getnum(st,2);
	mapname = script_getstr(st, 3);
	x = (short)script_getnum(st,4);
	y = (short)script_getnum(st,5);

	if (!unit_id) //Warp the script's runner
		bl = map->id2bl(st->rid);
	else
		bl = map->id2bl(unit_id);

	if( strcmp(mapname,"this") == 0 )
		mapid = bl?bl->m:-1;
	else
		mapid = map->mapname2mapid(mapname);

	if( mapid >= 0 && bl != NULL ) {
		unit->bl2ud2(bl); // ensure ((struct npc_data *)bl)->ud is safe to edit
		script_pushint(st, unit->warp(bl,mapid,x,y,CLR_OUTSIGHT));
	} else {
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
BUILDIN(unitattack) {
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
BUILDIN(unitstop) {
	int unit_id;
	struct block_list* bl;

	unit_id = script_getnum(st,2);

	bl = map->id2bl(unit_id);
	if( bl != NULL ) {
		unit->bl2ud2(bl); // ensure ((struct npc_data *)bl)->ud is safe to edit
		unit->stop_attack(bl);
		unit->stop_walking(bl, STOPWALKING_FLAG_NEXTCELL);
		if( bl->type == BL_MOB )
			BL_UCAST(BL_MOB, bl)->target_id = 0;
	}

	return true;
}

/// Makes the unit say the message
///
/// unittalk <unit_id>,"<message>";
BUILDIN(unittalk) {
	int unit_id;
	const char* message;
	struct block_list* bl;

	unit_id = script_getnum(st,2);
	message = script_getstr(st, 3);

	bl = map->id2bl(unit_id);
	if( bl != NULL ) {
		struct StringBuf sbuf;
		StrBuf->Init(&sbuf);
		StrBuf->Printf(&sbuf, "%s : %s", status->get_name(bl), message);
		clif->disp_overhead(bl, StrBuf->Value(&sbuf));
		StrBuf->Destroy(&sbuf);
	}

	return true;
}

/// Makes the unit do an emotion
///
/// unitemote <unit_id>,<emotion>;
///
/// @see e_* in const.txt
BUILDIN(unitemote) {
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
BUILDIN(unitskilluseid) {
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
		}
		unit->skilluse_id(bl, target_id, skill_id, skill_lv);
	}

	return true;
}

/// Makes the unit cast the skill on the target position.
///
/// unitskillusepos <unit_id>,<skill_id>,<skill_lv>,<target_x>,<target_y>;
/// unitskillusepos <unit_id>,"<skill name>",<skill_lv>,<target_x>,<target_y>;
BUILDIN(unitskillusepos) {
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
		}
		unit->skilluse_pos(bl, skill_x, skill_y, skill_id, skill_lv);
	}

	return true;
}

// <--- [zBuffer] List of mob control commands

/// Pauses the execution of the script, detaching the player
///
/// sleep <mili seconds>;
BUILDIN(sleep)
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
BUILDIN(sleep2) {
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
BUILDIN(awake) {
	DBIterator *iter;
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
BUILDIN(getvariableofnpc)
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

/// Opens a warp portal.
/// Has no "portal opening" effect/sound, it opens the portal immediately.
///
/// warpportal <source x>,<source y>,"<target map>",<target x>,<target y>;
///
/// @author blackhole89
BUILDIN(warpportal) {
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

BUILDIN(openmail)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;

	mail->openmail(sd);

	return true;
}

BUILDIN(openauction)
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
BUILDIN(checkcell) {
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
BUILDIN(setcell) {
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
BUILDIN(mercenary_create)
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

BUILDIN(mercenary_heal) {
	struct map_session_data *sd = script->rid2sd(st);
	int hp, sp;

	if( sd == NULL || sd->md == NULL )
		return true;
	hp = script_getnum(st,2);
	sp = script_getnum(st,3);

	status->heal(&sd->md->bl, hp, sp, 0);
	return true;
}

BUILDIN(mercenary_sc_start) {
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

BUILDIN(mercenary_get_calls) {
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

BUILDIN(mercenary_set_calls) {
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

BUILDIN(mercenary_get_faith) {
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

BUILDIN(mercenary_set_faith) {
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
BUILDIN(readbook)
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

BUILDIN(questinfo)
{
	struct npc_data *nd = map->id2nd(st->oid);
	int quest_id, icon;
	struct questinfo qi;

	if( nd == NULL || nd->bl.m == -1 )
		return true;

	quest_id = script_getnum(st, 2);
	icon = script_getnum(st, 3);

	#if PACKETVER >= 20120410
		if(icon < 0 || (icon > 8 && icon != 9999) || icon == 7)
			icon = 9999; // Default to nothing if icon id is invalid.
	#else
		if(icon < 0 || icon > 7)
			icon = 0;
		else
			icon = icon + 1;
	#endif

	qi.quest_id = quest_id;
	qi.icon = (unsigned char)icon;
	qi.nd = nd;

	if (script_hasdata(st, 4)) {
		int color = script_getnum(st, 4);
		if (color < 0 || color > 3) {
			ShowWarning("buildin_questinfo: invalid color '%d', changing to 0\n",color);
			script->reportfunc(st);
			color = 0;
		}
		qi.color = (unsigned char)color;
	}

	qi.hasJob = false;

	if (script_hasdata(st, 5)) {
		int job = script_getnum(st, 5);

		if (!pc->db_checkid(job)) {
			ShowError("buildin_questinfo: Nonexistant Job Class.\n");
		} else {
			qi.hasJob = true;
			qi.job = (unsigned short)job;
		}
	}

	map->add_questinfo(nd->bl.m,&qi);

	return true;
}

BUILDIN(setquest)
{
	unsigned short i;
	int quest_id;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	quest_id = script_getnum(st, 2);

	quest->add(sd, quest_id);

	// If questinfo is set, remove quest bubble once quest is set.
	for(i = 0; i < map->list[sd->bl.m].qi_count; i++) {
		struct questinfo *qi = &map->list[sd->bl.m].qi_data[i];
		if( qi->quest_id == quest_id ) {
#if PACKETVER >= 20120410
			clif->quest_show_event(sd, &qi->nd->bl, 9999, 0);
#else
			clif->quest_show_event(sd, &qi->nd->bl, 0, 0);
#endif
		}
	}

	return true;
}

BUILDIN(erasequest)
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

BUILDIN(completequest)
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

BUILDIN(changequest)
{
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	quest->change(sd, script_getnum(st, 2),script_getnum(st, 3));
	return true;
}

BUILDIN(questactive)
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

BUILDIN(questprogress)
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

BUILDIN(showevent)
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

	#if PACKETVER >= 20120410
		if(icon < 0 || (icon > 8 && icon != 9999) || icon == 7)
			icon = 9999; // Default to nothing if icon id is invalid.
	#else
		if(icon < 0 || icon > 7)
			icon = 0;
		else
			icon = icon + 1;
	#endif

	clif->quest_show_event(sd, &nd->bl, icon, color);
	return true;
}

/*==========================================
 * BattleGround System
 *------------------------------------------*/
BUILDIN(waitingroom2bg) {
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
			mapreg->setreg(reference_uid(script->add_str("$@arenamembers"), i), sd->bl.id);
		else
			mapreg->setreg(reference_uid(script->add_str("$@arenamembers"), i), 0);
	}

	mapreg->setreg(script->add_str("$@arenamembersnum"), i);
	script_pushint(st,bg_id);
	return true;
}

BUILDIN(waitingroom2bg_single) {
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

BUILDIN(bg_team_setxy)
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

BUILDIN(bg_warp)
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

BUILDIN(bg_monster)
{
	int class_ = 0, x = 0, y = 0, bg_id = 0;
	const char *str, *mapname, *evt="";

	bg_id   = script_getnum(st,2);
	mapname = script_getstr(st,3);
	x       = script_getnum(st,4);
	y       = script_getnum(st,5);
	str     = script_getstr(st,6);
	class_  = script_getnum(st,7);
	if( script_hasdata(st,8) ) evt = script_getstr(st,8);
	script->check_event(st, evt);
	script_pushint(st, mob->spawn_bg(mapname,x,y,str,class_,evt,bg_id));
	return true;
}

BUILDIN(bg_monster_set_team)
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
	clif->charnameack(0, &md->bl);

	return true;
}

BUILDIN(bg_leave)
{
	struct map_session_data *sd = script->rid2sd(st);
	if( sd == NULL || !sd->bg_id )
		return true;

	bg->team_leave(sd,BGTL_LEFT);
	return true;
}

BUILDIN(bg_destroy)
{
	int bg_id = script_getnum(st,2);
	bg->team_delete(bg_id);
	return true;
}

BUILDIN(bg_getareausers)
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

BUILDIN(bg_updatescore) {
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

BUILDIN(bg_get_data)
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

BUILDIN(instance_create) {
	const char *name;
	int owner_id, res;
	int type = IOT_PARTY;

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
	if( res == -4 ) { // Already exists
		script_pushint(st, -1);
		return true;
	} else if( res < 0 ) {
		const char *err;
		switch(res) {
			case -3: err = "No free instances"; break;
			case -2: err = "Invalid party ID"; break;
			case -1: err = "Invalid type"; break;
			default: err = "Unknown"; break;
		}
		ShowError("buildin_instance_create: %s [%d].\n", err, res);
		script_pushint(st, -2);
		return true;
	}

	script_pushint(st, res);
	return true;
}

BUILDIN(instance_destroy) {
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

BUILDIN(instance_attachmap)
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

BUILDIN(instance_detachmap) {
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

BUILDIN(instance_attach)
{
	int instance_id = script_getnum(st, 2);

	if( !instance->valid(instance_id) )
		return true;

	st->instance_id = instance_id;
	return true;
}

BUILDIN(instance_id) {
	script_pushint(st, st->instance_id);
	return true;
}

BUILDIN(instance_set_timeout)
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

BUILDIN(instance_init) {
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

BUILDIN(instance_announce) {
	int         instance_id = script_getnum(st,2);
	const char *mes         = script_getstr(st,3);
	int         flag        = script_getnum(st,4);
	const char *fontColor   = script_hasdata(st,5) ? script_getstr(st,5) : NULL;
	int         fontType    = script_hasdata(st,6) ? script_getnum(st,6) : 0x190; // default fontType (FW_NORMAL)
	int         fontSize    = script_hasdata(st,7) ? script_getnum(st,7) : 12;    // default fontSize
	int         fontAlign   = script_hasdata(st,8) ? script_getnum(st,8) : 0;     // default fontAlign
	int         fontY       = script_hasdata(st,9) ? script_getnum(st,9) : 0;     // default fontY

	int i;

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
		                  mes, strlen(mes)+1, flag&BC_COLOR_MASK, fontColor, fontType, fontSize, fontAlign, fontY);

	return true;
}

BUILDIN(instance_npcname) {
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

BUILDIN(has_instance) {
	struct map_session_data *sd;
	const char *str;
	int16 m;
	int instance_id = -1;
	bool type = strcmp(script->getfuncname(st),"has_instance2") == 0 ? true : false;

	str = script_getstr(st, 2);

	if( (m = map->mapname2mapid(str)) < 0 ) {
		if( type )
			script_pushint(st, -1);
		else
			script_pushconststr(st, "");
		return true;
	}

	if( script_hasdata(st, 3) )
		instance_id = script_getnum(st, 3);
	else if( st->instance_id >= 0 )
		instance_id = st->instance_id;
	else if( (sd = script->rid2sd(st)) != NULL ) {
		struct party_data *p;
		int i = 0, j = 0;
		if( sd->instances ) {
			for( i = 0; i < sd->instances; i++ ) {
				if( sd->instance[i] >= 0 ) {
					ARR_FIND(0, instance->list[sd->instance[i]].num_map, j, map->list[instance->list[sd->instance[i]].map[j]].instance_src_map == m);
					if( j != instance->list[sd->instance[i]].num_map )
						break;
				}
			}
			if( i != sd->instances )
				instance_id = sd->instance[i];
		}
		if (instance_id == -1 && sd->status.party_id && (p = party->search(sd->status.party_id)) != NULL && p->instances) {
			for( i = 0; i < p->instances; i++ ) {
				if( p->instance[i] >= 0 ) {
					ARR_FIND(0, instance->list[p->instance[i]].num_map, j, map->list[instance->list[p->instance[i]].map[j]].instance_src_map == m);
					if( j != instance->list[p->instance[i]].num_map )
						break;
				}
			}
			if( i != p->instances )
				instance_id = p->instance[i];
		}
		if( instance_id == -1 && sd->guild && sd->guild->instances ) {
			for( i = 0; i < sd->guild->instances; i++ ) {
				if( sd->guild->instance[i] >= 0 ) {
					ARR_FIND(0, instance->list[sd->guild->instance[i]].num_map, j, map->list[instance->list[sd->guild->instance[i]].map[j]].instance_src_map == m);
					if( j != instance->list[sd->guild->instance[i]].num_map )
						break;
				}
			}
			if( i != sd->guild->instances )
				instance_id = sd->guild->instance[i];
		}
	}

	if( !instance->valid(instance_id) || (m = instance->map2imap(m, instance_id)) < 0 ) {
		if( type )
			script_pushint(st, -1);
		else
			script_pushconststr(st, "");
		return true;
	}

	if( type )
		script_pushint(st, instance_id);
	else
		script_pushconststr(st, map->list[m].name);
	return true;
}

int buildin_instance_warpall_sub(struct block_list *bl, va_list ap)
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
BUILDIN(instance_warpall) {
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
BUILDIN(instance_check_party)
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
BUILDIN(instance_check_guild)
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
BUILDIN(setfont)
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

int buildin_mobuseskill_sub(struct block_list *bl, va_list ap)
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
BUILDIN(areamobuseskill) {
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

BUILDIN(progressbar)
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

BUILDIN(pushpc)
{
	uint8 dir;
	int cells, dx, dy;
	struct map_session_data* sd;

	if((sd = script->rid2sd(st))==NULL)
	{
		return true;
	}

	dir = script_getnum(st,2);
	cells     = script_getnum(st,3);

	if(dir>7)
	{
		ShowWarning("buildin_pushpc: Invalid direction %d specified.\n", dir);
		script->reportsrc(st);

		dir%= 8;  // trim spin-over
	}

	if(!cells)
	{// zero distance
		return true;
	}
	else if(cells<0)
	{// pushing backwards
		dir = (dir+4)%8;  // turn around
		cells     = -cells;
	}

	dx = dirx[dir];
	dy = diry[dir];

	unit->blown(&sd->bl, dx, dy, cells, 0);
	return true;
}

/// Invokes buying store preparation window
/// buyingstore <slots>;
BUILDIN(buyingstore)
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
BUILDIN(searchstores)
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
BUILDIN(showdigit)
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
BUILDIN(makerune)
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
BUILDIN(hascashmount)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;
	if( sd->sc.data[SC_ALL_RIDING] )
		script_pushint(st,1);
	else
		script_pushint(st,0);
	return true;
}

/**
 * setcashmount() returns 1 on success or 0 otherwise
 *
 * - Toggles cash mounts on a player when he can mount
 * - Will fail if the player is already riding a standard mount e.g. dragon, peco, wug, mado, etc.
 * - Will unmount the player is he is already mounting a cash mount
 **/
BUILDIN(setcashmount)
{
	struct map_session_data *sd = script->rid2sd(st);
	if (sd == NULL)
		return true;
	if (pc_hasmount(sd)) {
		clif->msgtable(sd, MSG_REINS_CANT_USE_MOUNTED);
		script_pushint(st,0);//can't mount with one of these
	} else {
		if (sd->sc.data[SC_ALL_RIDING])
			status_change_end(&sd->bl, SC_ALL_RIDING, INVALID_TIMER);
		else
			sc_start(NULL, &sd->bl, SC_ALL_RIDING, 100, 25, INFINITE_DURATION);
		script_pushint(st,1);//in both cases, return 1.
	}
	return true;
}

/**
 * Retrieves quantity of arguments provided to callfunc/callsub.
 * getargcount() -> amount of arguments received in a function
 **/
BUILDIN(getargcount) {
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
BUILDIN(getcharip) {
	struct map_session_data* sd = NULL;

	/* check if a character name is specified */
	if (script_hasdata(st, 2)) {
		if (script_isstringtype(st, 2)) {
			sd = map->nick2sd(script_getstr(st, 2));
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

	/* check for IP */
	if (!sockt->session[sd->fd]->client_addr) {
		script_pushconststr(st, "");
		return true;
	}

	/* return the client ip_addr converted for output */
	if (sd && sd->fd && sockt->session[sd->fd])
	{
		/* initiliaze */
		const char *ip_addr = NULL;
		uint32 ip;

		/* set ip, ip_addr and convert to ip and push str */
		ip = sockt->session[sd->fd]->client_addr;
		ip_addr = sockt->ip2str(ip, NULL);
		script_pushstrcopy(st, ip_addr);
	}

	return true;
}
/**
 * is_function(<function name>) -> 1 if function exists, 0 otherwise
 **/
BUILDIN(is_function) {
	const char* str = script_getstr(st,2);

	if( strdb_exists(script->userfunc_db, str) )
		script_pushint(st,1);
	else
		script_pushint(st,0);

	return true;
}
/**
 * freeloop(<toggle>) -> toggles this script instance's looping-check ability
 **/
BUILDIN(freeloop) {

	if( script_getnum(st,2) )
		st->freeloop = 1;
	else
		st->freeloop = 0;

	script_pushint(st, st->freeloop);

	return true;
}

BUILDIN(sit) {
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

BUILDIN(stand) {
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

BUILDIN(issit) {
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

/**
 * @commands (script based)
 **/
BUILDIN(bindatcmd) {
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
	}

	return true;
}

BUILDIN(unbindatcmd) {
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

BUILDIN(useatcmd) {
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

BUILDIN(checkre)
{
	int num;

	num=script_getnum(st,2);
	switch(num) {
		case 0:
#ifdef RENEWAL
			script_pushint(st, 1);
#else
			script_pushint(st, 0);
#endif
			break;
		case 1:
#ifdef RENEWAL_CAST
			script_pushint(st, 1);
#else
			script_pushint(st, 0);
#endif
			break;
		case 2:
#ifdef RENEWAL_DROP
			script_pushint(st, 1);
#else
			script_pushint(st, 0);
#endif
			break;
		case 3:
#ifdef RENEWAL_EXP
			script_pushint(st, 1);
#else
			script_pushint(st, 0);
#endif
			break;
		case 4:
#ifdef RENEWAL_LVDMG
			script_pushint(st, 1);
#else
			script_pushint(st, 0);
#endif
			break;
		case 5:
#ifdef RENEWAL_EDP
			script_pushint(st, 1);
#else
			script_pushint(st, 0);
#endif
			break;
		case 6:
#ifdef RENEWAL_ASPD
			script_pushint(st, 1);
#else
			script_pushint(st, 0);
#endif
			break;
		default:
			ShowWarning("buildin_checkre: unknown parameter.\n");
			break;
	}
	return true;
}

/* getrandgroupitem <container_item_id>,<quantity> */
BUILDIN(getrandgroupitem) {
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
						map->addflooritem(&sd->bl, &it, get_count, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0);
				}
			}
		}

		script_pushint(st, 0);
	}

	return true;
}

/* cleanmap <map_name>;
 * cleanarea <map_name>, <x0>, <y0>, <x1>, <y1>; */
int script_cleanfloor_sub(struct block_list *bl, va_list ap) {
	nullpo_ret(bl);
	map->clearflooritem(bl);

	return 0;
}

BUILDIN(cleanmap)
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
BUILDIN(npcskill)
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
BUILDIN(montransform) {
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
struct script_queue *script_hqueue_get(int idx)
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
int script_hqueue_create(void)
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
BUILDIN(queue)
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
BUILDIN(queuesize)
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
bool script_hqueue_add(int idx, int var)
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
BUILDIN(queueadd)
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
bool script_hqueue_remove(int idx, int var)
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
BUILDIN(queueremove)
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
 * When the option value isn't provided, the option is removed.
 *
 * Returns 1 (true) on success, 0 (false) on failure.
 *
 * The optionType is one of:
 * - QUEUEOPT_DEATH
 * - QUEUEOPT_LOGOUT
 * - QUEUEOPT_MAPCHANGE
 *
 * When the QUEUEOPT_MAPCHANGE event is triggered, it sets a temporary
 * character variable \c @Queue_Destination_Map$ with the destination map name.
 *
 * @code{.herc}
 *    queueopt(.@queue_id, optionType, <optional val>);
 * @endcode
 */
BUILDIN(queueopt)
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
bool script_hqueue_del(int idx)
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
BUILDIN(queuedel)
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
bool script_hqueue_clear(int idx)
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
BUILDIN(queueiterator)
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
BUILDIN(qiget)
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
BUILDIN(qicheck)
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
BUILDIN(qiclear)
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
BUILDIN(packageitem) {
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
BUILDIN(bg_create_team) {
	const char *map_name, *ev = "", *dev = "";//ev and dev will be dropped.
	int x, y, map_index = 0, bg_id;

	map_name = script_getstr(st,2);
	if( strcmp(map_name,"-") != 0 ) {
		map_index = script->mapindexname2id(st,map_name);
		if( map_index == 0 ) { // Invalid Map
			script_pushint(st,0);
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
BUILDIN(bg_join_team) {
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
BUILDIN(countbound)
{
	int i, type, j=0, k=0;
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	type = script_hasdata(st,2)?script_getnum(st,2):0;

	for(i=0;i<MAX_INVENTORY;i++) {
		if(sd->status.inventory[i].nameid > 0 && (
			(!type && sd->status.inventory[i].bound > 0) ||
			(type && sd->status.inventory[i].bound == type)
		)) {
			pc->setreg(sd,reference_uid(script->add_str("@bound_items"), k),sd->status.inventory[i].nameid);
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
BUILDIN(checkbound)
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

	ARR_FIND( 0, MAX_INVENTORY, i, (sd->status.inventory[i].nameid == nameid &&
			( sd->status.inventory[i].refine == (script_hasdata(st,4)? script_getnum(st,4) : sd->status.inventory[i].refine) ) &&
			( sd->status.inventory[i].attribute == (script_hasdata(st,5)? script_getnum(st,5) : sd->status.inventory[i].attribute) ) &&
			( sd->status.inventory[i].card[0] == (script_hasdata(st,6)? script_getnum(st,6) : sd->status.inventory[i].card[0]) ) &&
			( sd->status.inventory[i].card[1] == (script_hasdata(st,7)? script_getnum(st,7) : sd->status.inventory[i].card[1]) ) &&
			( sd->status.inventory[i].card[2] == (script_hasdata(st,8)? script_getnum(st,8) : sd->status.inventory[i].card[2]) ) &&
			( sd->status.inventory[i].card[3] == (script_hasdata(st,9)? script_getnum(st,9) : sd->status.inventory[i].card[3]) ) &&
			((sd->status.inventory[i].bound > 0 && !bound_type) || sd->status.inventory[i].bound == bound_type )) );

	if( i < MAX_INVENTORY ){
		script_pushint(st, sd->status.inventory[i].bound);
		return true;
	} else
		script_pushint(st,0);

	return true;
}

/* bg_match_over( arena_name {, optional canceled } ) */
/* returns 0 when successful, 1 otherwise */
BUILDIN(bg_match_over) {
	bool canceled = script_hasdata(st,3) ? true : false;
	struct bg_arena *arena = bg->name2arena((const char*)script_getstr(st, 2));

	if( arena ) {
		bg->match_over(arena,canceled);
		script_pushint(st, 0);
	} else
		script_pushint(st, 1);

	return true;
}

BUILDIN(instance_mapname) {
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
BUILDIN(instance_set_respawn) {
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
BUILDIN(openshop)
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
/**
 * @call sellitem <Item_ID>,{,price{,qty}};
 *
 * adds <Item_ID> (or modifies if present) to shop
 * if price not provided (or -1) uses the item's value_sell
 **/
BUILDIN(sellitem) {
	struct npc_data *nd;
	struct item_data *it;
	int i = 0, id = script_getnum(st,2);
	int value = 0;
	int qty = 0;

	if( !(nd = map->id2nd(st->oid)) ) {
		ShowWarning("buildin_sellitem: trying to run without a proper NPC!\n");
		return false;
	} else if ( !(it = itemdb->exists(id)) ) {
		ShowWarning("buildin_sellitem: unknown item id '%d'!\n",id);
		return false;
	}

	value = script_hasdata(st,3) ? script_getnum(st, 3) : it->value_buy;
	if( value == -1 )
		value = it->value_buy;

	if( !nd->u.scr.shop )
		npc->trader_update(nd->src_id?nd->src_id:nd->bl.id);
	else {/* no need to run this if its empty */
		for( i = 0; i < nd->u.scr.shop->items; i++ ) {
			if( nd->u.scr.shop->item[i].nameid == id )
				break;
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

	if( i != nd->u.scr.shop->items ) {
		nd->u.scr.shop->item[i].value = value;
		nd->u.scr.shop->item[i].qty   = qty;
		if( nd->u.scr.shop->type == NST_MARKET ) /* has been manually updated, make it reflect on sql */
			npc->market_tosql(nd,i);
	} else {
		for( i = 0; i < nd->u.scr.shop->items; i++ ) {
			if( nd->u.scr.shop->item[i].nameid == 0 )
				break;
		}

		if( i == nd->u.scr.shop->items ) {
			if( nd->u.scr.shop->items == USHRT_MAX ) {
				ShowWarning("buildin_sellitem: Can't add %s (%s/%s), shop list is full!\n", it->name, nd->exname, nd->path);
				return false;
			}
			i = nd->u.scr.shop->items;
			RECREATE(nd->u.scr.shop->item, struct npc_item_list, ++nd->u.scr.shop->items);
		}

		nd->u.scr.shop->item[i].nameid = it->nameid;
		nd->u.scr.shop->item[i].value  = value;
		nd->u.scr.shop->item[i].qty    = qty;
	}

	return true;
}
/**
 * @call stopselling <Item_ID>;
 *
 * removes <Item_ID> from the current npc shop
 *
 * @return 1 on success, 0 otherwise
 **/
BUILDIN(stopselling) {
	struct npc_data *nd;
	int i, id = script_getnum(st,2);

	if( !(nd = map->id2nd(st->oid)) || !nd->u.scr.shop ) {
		ShowWarning("buildin_stopselling: trying to run without a proper NPC!\n");
		return false;
	}

	for( i = 0; i < nd->u.scr.shop->items; i++ ) {
		if( nd->u.scr.shop->item[i].nameid == id )
			break;
	}

	if( i != nd->u.scr.shop->items ) {
		int cursor;

		if( nd->u.scr.shop->type == NST_MARKET )
			npc->market_delfromsql(nd,i);

		nd->u.scr.shop->item[i].nameid = 0;
		nd->u.scr.shop->item[i].value  = 0;
		nd->u.scr.shop->item[i].qty    = 0;

		for( i = 0, cursor = 0; i < nd->u.scr.shop->items; i++ ) {
			if( nd->u.scr.shop->item[i].nameid == 0 )
				continue;

			if( cursor != i ) {
				nd->u.scr.shop->item[cursor].nameid = nd->u.scr.shop->item[i].nameid;
				nd->u.scr.shop->item[cursor].value  = nd->u.scr.shop->item[i].value;
				nd->u.scr.shop->item[cursor].qty    = nd->u.scr.shop->item[i].qty;
			}

			cursor++;
		}

		script_pushint(st, 1);
	} else
		script_pushint(st, 0);

	return true;
}
/**
 * @call setcurrency <Val1>{,<Val2>};
 *
 * updates currently-attached player shop currency
 **/
/* setcurrency(<Val1>,{<Val2>}) */
BUILDIN(setcurrency)
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
BUILDIN(tradertype) {
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
		npc->market_delfromsql(nd,USHRT_MAX);
	}

#if PACKETVER < 20131223
	if( type == NST_MARKET ) {
		ShowWarning("buildin_tradertype: NST_MARKET is only available with PACKETVER 20131223 or newer!\n");
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
BUILDIN(purchaseok) {
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
BUILDIN(shopcount) {
	struct npc_data *nd;
	int id = script_getnum(st, 2);
	unsigned short i;

	if( !(nd = map->id2nd(st->oid)) ) {
		ShowWarning("buildin_shopcount(%d): trying to run without a proper NPC!\n",id);
		return false;
	} else if ( !nd->u.scr.shop || !nd->u.scr.shop->items ) {
		ShowWarning("buildin_shopcount(%d): trying to use without any items!\n",id);
		return false;
	} else if ( nd->u.scr.shop->type != NST_MARKET ) {
		ShowWarning("buildin_shopcount(%d): trying to use on a non-NST_MARKET shop!\n",id);
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
BUILDIN(channelmes)
{
	struct map_session_data *sd = script->rid2sd(st);
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

/** By Cydh
Display script message
showscript "<message>"{,<GID>};
*/
BUILDIN(showscript) {
	struct block_list *bl = NULL;
	const char *msg = script_getstr(st, 2);
	int id = 0;

	if (script_hasdata(st, 3)) {
		id = script_getnum(st, 3);
		bl = map->id2bl(id);
	}
	else {
		bl = st->rid ? map->id2bl(st->rid) : map->id2bl(st->oid);
	}

	if (!bl) {
		ShowError("buildin_showscript: Script not attached. (id=%d, rid=%d, oid=%d)\n", id, st->rid, st->oid);
		script_pushint(st, 0);
		return false;
	}

	clif->ShowScript(bl, msg);

	script_pushint(st, 1);

	return true;
}

BUILDIN(mergeitem)
{
	struct map_session_data *sd = script->rid2sd(st);

	if (sd == NULL)
		return true;

	clif->openmergeitem(sd->fd, sd);

	return true;
}
/** place holder for the translation macro **/
BUILDIN(_) {
	return true;
}

// declarations that were supposed to be exported from npc_chat.c
BUILDIN(defpattern);
BUILDIN(activatepset);
BUILDIN(deactivatepset);
BUILDIN(deletepset);

BUILDIN(pcre_match) {
	const char *input = script_getstr(st, 2);
	const char *regex = script_getstr(st, 3);

	script->op_2str(st, C_RE_EQ, input, regex);
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
bool script_add_builtin(const struct script_function *buildin, bool override) {
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
		else if( strcmp(buildin->name, "select") == 0 ) script->buildin_select_offset = script->buildin_count;
		else if( strcmp(buildin->name, "_") == 0 ) script->buildin_lang_macro_offset = script->buildin_count;

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

bool script_hp_add(char *name, char *args, bool (*func)(struct script_state *st), bool isDeprecated) {
	struct script_function buildin;
	buildin.name = name;
	buildin.arg = args;
	buildin.func = func;
	buildin.deprecated = isDeprecated;
	return script->add_builtin(&buildin, true);
}

void script_run_use_script(struct map_session_data *sd, struct item_data *data, int oid) __attribute__((nonnull (1)));

/**
 * Run use script for item.
 *
 * @param sd    player session data. Must be correct and checked before.
 * @param n     item index in inventory. Must be correct and checked before.
 * @param oid   npc id. Can be also 0 or fake npc id.
 */
void script_run_use_script(struct map_session_data *sd, struct item_data *data, int oid)
{
	script->current_item_id = data->nameid;
	script->run(data->script, 0, sd->bl.id, oid);
	script->current_item_id = 0;
}

void script_run_item_equip_script(struct map_session_data *sd, struct item_data *data, int oid) __attribute__((nonnull (1, 2)));

/**
 * Run item equip script for item.
 *
 * @param sd    player session data. Must be correct and checked before.
 * @param data  equipped item data. Must be correct and checked before.
 * @param oid   npc id. Can be also 0 or fake npc id.
 */
void script_run_item_equip_script(struct map_session_data *sd, struct item_data *data, int oid)
{
	script->current_item_id = data->nameid;
	script->run(data->equip_script, 0, sd->bl.id, oid);
	script->current_item_id = 0;
}

void script_run_item_unequip_script(struct map_session_data *sd, struct item_data *data, int oid) __attribute__((nonnull (1, 2)));

/**
 * Run item unequip script for item.
 *
 * @param sd    player session data. Must be correct and checked before.
 * @param data  unequipped item data. Must be correct and checked before.
 * @param oid   npc id. Can be also 0 or fake npc id.
 */
void script_run_item_unequip_script(struct map_session_data *sd, struct item_data *data, int oid)
{
	script->current_item_id = data->nameid;
	script->run(data->unequip_script, 0, sd->bl.id, oid);
	script->current_item_id = 0;
}

#define BUILDIN_DEF(x,args) { buildin_ ## x , #x , args, false }
#define BUILDIN_DEF2(x,x2,args) { buildin_ ## x , x2 , args, false }
#define BUILDIN_DEF_DEPRECATED(x,args) { buildin_ ## x , #x , args, true }
#define BUILDIN_DEF2_DEPRECATED(x,x2,args) { buildin_ ## x , x2 , args, true }
void script_parse_builtin(void) {
	struct script_function BUILDIN[] = {
		/* Commands for internal use by the script engine */
		BUILDIN_DEF(__jump_zero,"il"),
		BUILDIN_DEF(__setr,"rv?"),

		// NPC interaction
		BUILDIN_DEF(mes,"s*"),
		BUILDIN_DEF(next,""),
		BUILDIN_DEF(close,""),
		BUILDIN_DEF(close2,""),
		BUILDIN_DEF(menu,"sl*"),
		BUILDIN_DEF(select,"s*"), //for future jA script compatibility
		BUILDIN_DEF(prompt,"s*"),
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
		BUILDIN_DEF(warpparty,"siii?"), // [Fredzilla] [Paradox924X]
		BUILDIN_DEF(warpguild,"siii"), // [Fredzilla]
		BUILDIN_DEF(setlook,"ii"),
		BUILDIN_DEF(changelook,"ii"), // Simulates but don't Store it
		BUILDIN_DEF2(__setr,"set","rv"),
		BUILDIN_DEF(setarray,"rv*"),
		BUILDIN_DEF(cleararray,"rvi"),
		BUILDIN_DEF(copyarray,"rri"),
		BUILDIN_DEF(getarraysize,"r"),
		BUILDIN_DEF(deletearray,"r?"),
		BUILDIN_DEF(getelementofarray,"ri"),
		BUILDIN_DEF(getitem,"vi?"),
		BUILDIN_DEF(rentitem,"vi"),
		BUILDIN_DEF(getitem2,"viiiiiiii?"),
		BUILDIN_DEF(getnameditem,"vv"),
		BUILDIN_DEF2(grouprandomitem,"groupranditem","i"),
		BUILDIN_DEF(makeitem,"visii"),
		BUILDIN_DEF(delitem,"vi?"),
		BUILDIN_DEF(delitem2,"viiiiiiii?"),
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
		BUILDIN_DEF(checkweight,"vi*"),
		BUILDIN_DEF(checkweight2,"rr"),
		BUILDIN_DEF(readparam,"i?"),
		BUILDIN_DEF(getcharid,"i?"),
		BUILDIN_DEF(getnpcid,"i?"),
		BUILDIN_DEF(getpartyname,"i"),
		BUILDIN_DEF(getpartymember,"i?"),
		BUILDIN_DEF(getpartyleader,"i?"),
		BUILDIN_DEF(getguildname,"i"),
		BUILDIN_DEF(getguildmaster,"i"),
		BUILDIN_DEF(getguildmasterid,"i"),
		BUILDIN_DEF(getguildmember,"i?"),
		BUILDIN_DEF(strcharinfo,"i"),
		BUILDIN_DEF(strnpcinfo,"i"),
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
		BUILDIN_DEF(getequiprefinerycnt,"i"),
		BUILDIN_DEF(getequipweaponlv,"i"),
		BUILDIN_DEF(getequippercentrefinery,"i"),
		BUILDIN_DEF(successrefitem,"i?"),
		BUILDIN_DEF(failedrefitem,"i"),
		BUILDIN_DEF(downrefitem,"i?"),
		BUILDIN_DEF(statusup,"i"),
		BUILDIN_DEF(statusup2,"ii"),
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
		BUILDIN_DEF(getgroupid,""),
		BUILDIN_DEF(end,""),
		BUILDIN_DEF(checkoption,"i"),
		BUILDIN_DEF(setoption,"i?"),
		BUILDIN_DEF(setcart,"?"),
		BUILDIN_DEF(checkcart,""),
		BUILDIN_DEF(setfalcon,"?"),
		BUILDIN_DEF(checkfalcon,""),
		BUILDIN_DEF(setmount,"?"),
		BUILDIN_DEF(checkmount,""),
		BUILDIN_DEF(checkwug,""),
		BUILDIN_DEF(savepoint,"sii"),
		BUILDIN_DEF(gettimetick,"i"),
		BUILDIN_DEF(gettime,"i"),
		BUILDIN_DEF(gettimestr,"si"),
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
		BUILDIN_DEF(clone,"siisi????"),
		BUILDIN_DEF(doevent,"s"),
		BUILDIN_DEF(donpcevent,"s"),
		BUILDIN_DEF(addtimer,"is"),
		BUILDIN_DEF(deltimer,"s"),
		BUILDIN_DEF(addtimercount,"si"),
		BUILDIN_DEF(initnpctimer,"??"),
		BUILDIN_DEF(stopnpctimer,"??"),
		BUILDIN_DEF(startnpctimer,"??"),
		BUILDIN_DEF(setnpctimer,"i?"),
		BUILDIN_DEF(getnpctimer,"i?"),
		BUILDIN_DEF(attachnpctimer,"?"), // attached the player id to the npc timer [Celest]
		BUILDIN_DEF(detachnpctimer,"?"), // detached the player id from the npc timer [Celest]
		BUILDIN_DEF(playerattached,""), // returns id of the current attached player. [Skotlex]
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
		BUILDIN_DEF(sc_start,"iii???"),
		BUILDIN_DEF2(sc_start,"sc_start2","iiii???"),
		BUILDIN_DEF2(sc_start,"sc_start4","iiiiii???"),
		BUILDIN_DEF(sc_end,"i?"),
		BUILDIN_DEF(getstatus, "i?"),
		BUILDIN_DEF(getscrate,"ii?"),
		BUILDIN_DEF(debugmes,"v"),
		BUILDIN_DEF2(catchpet,"pet","i"),
		BUILDIN_DEF2(birthpet,"bpet",""),
		BUILDIN_DEF(resetlvl,"i"),
		BUILDIN_DEF(resetstatus,""),
		BUILDIN_DEF(resetskill,""),
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
		BUILDIN_DEF(classchange,"ii"),
		BUILDIN_DEF(misceffect,"i"),
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
		BUILDIN_DEF(specialeffect,"i??"), // npc skill effect [Valaris]
		BUILDIN_DEF(specialeffect2,"i??"), // skill effect on players[Valaris]
		BUILDIN_DEF(nude,""), // nude command [Valaris]
		BUILDIN_DEF(mapwarp,"ssii??"), // Added by RoVeRT
		BUILDIN_DEF(atcommand,"s"), // [MouseJstr]
		BUILDIN_DEF2(atcommand,"charcommand","s"), // [MouseJstr]
		BUILDIN_DEF(movenpc,"sii?"), // [MouseJstr]
		BUILDIN_DEF(message,"vs"), // [MouseJstr]
		BUILDIN_DEF(npctalk,"s?"), // [Valaris]
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
		BUILDIN_DEF(checkoption1,"i"),
		BUILDIN_DEF(checkoption2,"i"),
		BUILDIN_DEF(guildgetexp,"i"),
		BUILDIN_DEF(guildchangegm,"is"),
		BUILDIN_DEF(logmes,"s"), //this command actls as MES but rints info into LOG file either SQL/TXT [Lupus]
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
		BUILDIN_DEF(pcre_match,"ss"),
		BUILDIN_DEF(dispbottom,"s?"), //added from jA [Lupus]
		BUILDIN_DEF(getusersname,""),
		BUILDIN_DEF(recovery,""),
		BUILDIN_DEF(getpetinfo,"i"),
		BUILDIN_DEF(gethominfo,"i"),
		BUILDIN_DEF(getmercinfo,"i?"),
		BUILDIN_DEF(checkequipedcard,"i"),
		BUILDIN_DEF(globalmes,"s?"), //end jA addition
		BUILDIN_DEF(unequip,"i"), // unequip command [Spectre]
		BUILDIN_DEF(getstrlen,"s"), //strlen [Valaris]
		BUILDIN_DEF(charisalpha,"si"), //isalpha [Valaris]
		BUILDIN_DEF(charat,"si"),
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
		BUILDIN_DEF(getiteminfo,"ii"), //[Lupus] returns Items Buy / sell Price, etc info
		BUILDIN_DEF(setiteminfo,"iii"), //[Lupus] set Items Buy / sell Price, etc info
		BUILDIN_DEF(getequipcardid,"ii"), //[Lupus] returns CARD ID or other info from CARD slot N of equipped item
		// List of mathematics commands --->
		BUILDIN_DEF(log10,"i"),
		BUILDIN_DEF(sqrt,"i"), //[zBuffer]
		BUILDIN_DEF(pow,"ii"), //[zBuffer]
		BUILDIN_DEF(distance,"iiii"), //[zBuffer]
		// <--- List of mathematics commands
		BUILDIN_DEF(min, "i*"),
		BUILDIN_DEF(max, "i*"),
		BUILDIN_DEF(md5,"s"),
		BUILDIN_DEF(swap,"rr"),
		// [zBuffer] List of dynamic var commands --->
		BUILDIN_DEF(getd,"s"),
		BUILDIN_DEF(setd,"sv"),
		// <--- [zBuffer] List of dynamic var commands
		BUILDIN_DEF(petstat,"i"),
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
		BUILDIN_DEF(pcblockmove,"ii"),
		// <--- [zBuffer] List of player cont commands
		// [zBuffer] List of mob control commands --->
		BUILDIN_DEF(getunittype,"i"),
		BUILDIN_DEF(unitwalk,"ii?"),
		BUILDIN_DEF(unitkill,"i"),
		BUILDIN_DEF(unitwarp,"isii"),
		BUILDIN_DEF(unitattack,"iv?"),
		BUILDIN_DEF(unitstop,"i"),
		BUILDIN_DEF(unittalk,"is"),
		BUILDIN_DEF(unitemote,"ii"),
		BUILDIN_DEF(unitskilluseid,"ivi?"), // originally by Qamera [Celest]
		BUILDIN_DEF(unitskillusepos,"iviii"), // [Celest]
		// <--- [zBuffer] List of mob control commands
		BUILDIN_DEF(sleep,"i"),
		BUILDIN_DEF(sleep2,"i"),
		BUILDIN_DEF(awake,"s"),
		BUILDIN_DEF(getvariableofnpc,"rs"),
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
		BUILDIN_DEF(areamobuseskill,"siiiiviiiii"),
		BUILDIN_DEF(progressbar,"si"),
		BUILDIN_DEF(pushpc,"ii"),
		BUILDIN_DEF(buyingstore,"i"),
		BUILDIN_DEF(searchstores,"ii"),
		BUILDIN_DEF(showdigit,"i?"),
		// WoE SE
		BUILDIN_DEF(agitstart2,""),
		BUILDIN_DEF(agitend2,""),
		BUILDIN_DEF(agitcheck2,""),
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
		BUILDIN_DEF(useatcmd, "s"),

		/**
		 * Item bound [Xantara] [Akinari] [Mhalicot/Hercules]
		 **/
		BUILDIN_DEF2(getitem,"getitembound","vii?"),
		BUILDIN_DEF2(getitem2,"getitembound2","viiiiiiiii?"),
		BUILDIN_DEF(countbound, "?"),
		BUILDIN_DEF(checkbound, "i???????"),

		//Quest Log System [Inkfish]
		BUILDIN_DEF(questinfo, "ii??"),
		BUILDIN_DEF(setquest, "i"),
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
		BUILDIN_DEF(sellitem,"i??"),
		BUILDIN_DEF(stopselling,"i"),
		BUILDIN_DEF(setcurrency,"i?"),
		BUILDIN_DEF(tradertype,"i"),
		BUILDIN_DEF(purchaseok,""),
		BUILDIN_DEF(shopcount, "i"),

		BUILDIN_DEF(channelmes, "ss"),
		BUILDIN_DEF(showscript, "s?"),
		BUILDIN_DEF(mergeitem,""),
		BUILDIN_DEF(_,"s"),
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

void script_label_add(int key, int pos) {
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
void script_hardcoded_constants(void)
{
	script->constdb_comment("Boolean");
	script->set_constant("true", 1, false, false);
	script->set_constant("false", 0, false, false);

	script->constdb_comment("Server defines");
	script->set_constant("PACKETVER",PACKETVER,false, false);
	script->set_constant("MAX_LEVEL",MAX_LEVEL,false, false);
	script->set_constant("MAX_STORAGE",MAX_STORAGE,false, false);
	script->set_constant("MAX_GUILD_STORAGE",MAX_GUILD_STORAGE,false, false);
	script->set_constant("MAX_CART",MAX_INVENTORY,false, false);
	script->set_constant("MAX_INVENTORY",MAX_INVENTORY,false, false);
	script->set_constant("MAX_ZENY",MAX_ZENY,false, false);
	script->set_constant("MAX_BG_MEMBERS",MAX_BG_MEMBERS,false, false);
	script->set_constant("MAX_CHAT_USERS",MAX_CHAT_USERS,false, false);
	script->set_constant("MAX_REFINE",MAX_REFINE,false, false);

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
unsigned short script_mapindexname2id (struct script_state *st, const char* name) {
	unsigned short index;

	if( !(index=mapindex->name2id(name)) ) {
		script->reportsrc(st);
		return 0;
	}
	return index;
}

void script_defaults(void) {
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

	script->buf = NULL;
	script->pos = 0, script->size = 0;

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
	script->get_val_scope_str = get_val_npcscope_str;
	script->get_val_npc_str = get_val_npcscope_str;
	script->get_val_instance_str = get_val_instance_str;
	script->get_val_ref_num = get_val_npcscope_num;
	script->get_val_scope_num = get_val_npcscope_num;
	script->get_val_npc_num = get_val_npcscope_num;
	script->get_val_instance_num = get_val_instance_num;
	script->push_str = push_str;
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
	script->get_str = script_get_str;
	script->search_str = script_search_str;
	script->setd_sub = setd_sub;
	script->attach_state = script_attach_state;

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
	script->parse_expr = parse_expr;
	script->parse_line = parse_line;
	script->read_constdb = read_constdb;
	script->constdb_comment = script_constdb_comment;
	script->load_parameters = script_load_parameters;
	script->print_line = script_print_line;
	script->errorwarning_sub = script_errorwarning_sub;
	script->set_reg = set_reg;
	script->set_reg_ref_str = set_reg_npcscope_str;
	script->set_reg_scope_str = set_reg_npcscope_str;
	script->set_reg_npc_str = set_reg_npcscope_str;
	script->set_reg_instance_str = set_reg_instance_str;
	script->set_reg_ref_num = set_reg_npcscope_num;
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
	script->config.warn_func_mismatch_argtypes = 1;
	script->config.warn_func_mismatch_paramnum = 1;
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
	script->load_translation = script_load_translation;
	script->translation_db_destroyer = script_translation_db_destroyer;
	script->clear_translations = script_clear_translations;
	script->parse_cleanup_timer = script_parse_cleanup_timer;
	script->add_language = script_add_language;
	script->get_translation_file_name = script_get_translation_file_name;
	script->parser_clean_leftovers = script_parser_clean_leftovers;

	script->run_use_script = script_run_use_script;
	script->run_item_equip_script = script_run_item_equip_script;
	script->run_item_unequip_script = script_run_item_unequip_script;
}
