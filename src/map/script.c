// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

//#define DEBUG_DISP
//#define DEBUG_DISASM
//#define DEBUG_RUN
//#define DEBUG_HASH
//#define DEBUG_DUMP_STACK

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/md5calc.h"
#include "../common/nullpo.h"
#include "../common/random.h"
#include "../common/showmsg.h"
#include "../common/socket.h"	// usage: getcharip
#include "../common/strlib.h"
#include "../common/timer.h"
#include "../common/utils.h"

#include "map.h"
#include "path.h"
#include "clif.h"
#include "chrif.h"
#include "itemdb.h"
#include "pc.h"
#include "status.h"
#include "storage.h"
#include "mob.h"
#include "npc.h"
#include "pet.h"
#include "mapreg.h"
#include "homunculus.h"
#include "instance.h"
#include "mercenary.h"
#include "intif.h"
#include "skill.h"
#include "status.h"
#include "chat.h"
#include "battle.h"
#include "battleground.h"
#include "party.h"
#include "guild.h"
#include "atcommand.h"
#include "log.h"
#include "unit.h"
#include "pet.h"
#include "mail.h"
#include "script.h"
#include "quest.h"
#include "elemental.h"
#include "../config/core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef WIN32
	#include <sys/time.h>
#endif
#include <time.h>
#include <setjmp.h>
#include <errno.h>

#define FETCH(n, t) \
		if( script_hasdata(st,n) ) \
			(t)=script_getnum(st,n);

/// Maximum amount of elements in script arrays
#define SCRIPT_MAX_ARRAYSIZE 128

#define SCRIPT_BLOCK_SIZE 512
enum { LABEL_NEXTLINE=1,LABEL_START };

/// temporary buffer for passing around compiled bytecode
/// @see add_scriptb, set_label, parse_script
static unsigned char* script_buf = NULL;
static int script_pos = 0, script_size = 0;

static inline int GETVALUE(const unsigned char* buf, int i)
{
	return (int)MakeDWord(MakeWord(buf[i], buf[i+1]), MakeWord(buf[i+2], 0));
}
static inline void SETVALUE(unsigned char* buf, int i, int n)
{
	buf[i]   = GetByte(n, 0);
	buf[i+1] = GetByte(n, 1);
	buf[i+2] = GetByte(n, 2);
}

// String buffer structures.
// str_data stores string information
static struct str_data_struct {
	enum c_op type;
	int str;
	int backpatch;
	int label;
	bool (*func)(struct script_state *st);
	int val;
	int next;
} *str_data = NULL;
static int str_data_size = 0; // size of the data
static int str_num = LABEL_START; // next id to be assigned

// str_buf holds the strings themselves
static char *str_buf;
static int str_size = 0; // size of the buffer
static int str_pos = 0; // next position to be assigned


// Using a prime number for SCRIPT_HASH_SIZE should give better distributions
#define SCRIPT_HASH_SIZE 1021
int str_hash[SCRIPT_HASH_SIZE];
// Specifies which string hashing method to use
//#define SCRIPT_HASH_DJB2
//#define SCRIPT_HASH_SDBM
#define SCRIPT_HASH_ELF

static DBMap* scriptlabel_db=NULL; // const char* label_name -> int script_pos
static DBMap* userfunc_db=NULL; // const char* func_name -> struct script_code*
static int parse_options=0;
DBMap* script_get_label_db(void){ return scriptlabel_db; }
DBMap* script_get_userfunc_db(void){ return userfunc_db; }

// important buildin function references for usage in scripts
static int buildin_set_ref = 0;
static int buildin_callsub_ref = 0;
static int buildin_callfunc_ref = 0;
static int buildin_getelementofarray_ref = 0;

// Caches compiled autoscript item code.
// Note: This is not cleared when reloading itemdb.
static DBMap* autobonus_db=NULL; // char* script -> char* bytecode

struct Script_Config script_config = {
	1, // warn_func_mismatch_argtypes
	1, 65535, 2048, //warn_func_mismatch_paramnum/check_cmdcount/check_gotocount
	0, INT_MAX, // input_min_value/input_max_value
	"OnPCDieEvent", //die_event_name
	"OnPCKillEvent", //kill_pc_event_name
	"OnNPCKillEvent", //kill_mob_event_name
	"OnPCLoginEvent", //login_event_name
	"OnPCLogoutEvent", //logout_event_name
	"OnPCLoadMapEvent", //loadmap_event_name
	"OnPCBaseLvUpEvent", //baselvup_event_name
	"OnPCJobLvUpEvent", //joblvup_event_name
	"OnTouch_",	//ontouch_name (runs on first visible char to enter area, picks another char if the first char leaves)
	"OnTouch",	//ontouch2_name (run whenever a char walks into the OnTouch area)
};

static jmp_buf     error_jump;
static char*       error_msg;
static const char* error_pos;
static int         error_report; // if the error should produce output

// for advanced scripting support ( nested if, switch, while, for, do-while, function, etc )
// [Eoe / jA 1080, 1081, 1094, 1164]
enum curly_type {
	TYPE_NULL = 0,
	TYPE_IF,
	TYPE_SWITCH,
	TYPE_WHILE,
	TYPE_FOR,
	TYPE_DO,
	TYPE_USERFUNC,
	TYPE_ARGLIST // function argument list
};

enum e_arglist
{
	ARGLIST_UNDEFINED = 0,
	ARGLIST_NO_PAREN  = 1,
	ARGLIST_PAREN     = 2,
};

static struct {
	struct {
		enum curly_type type;
		int index;
		int count;
		int flag;
		struct linkdb_node *case_label;
	} curly[256];		// Information right parenthesis
	int curly_count;	// The number of right brackets
	int index;			// Number of the syntax used in the script
} syntax;

const char* parse_curly_close(const char* p);
const char* parse_syntax_close(const char* p);
const char* parse_syntax_close_sub(const char* p,int* flag);
const char* parse_syntax(const char* p);
static int parse_syntax_for_flag = 0;

extern int current_equip_item_index; //for New CARDS Scripts. It contains Inventory Index of the EQUIP_SCRIPT caller item. [Lupus]
int potion_flag=0; //For use on Alchemist improved potions/Potion Pitcher. [Skotlex]
int potion_hp=0, potion_per_hp=0, potion_sp=0, potion_per_sp=0;
int potion_target=0;


c_op get_com(unsigned char *script,int *pos);
int get_num(unsigned char *script,int *pos);

static struct linkdb_node* sleep_db;// int oid -> struct script_state*

/*==========================================
 * (Only those needed) local declaration prototype
 *------------------------------------------*/
const char* parse_subexpr(const char* p,int limit);
int run_func(struct script_state *st);

enum {
	MF_NOMEMO,	//0
	MF_NOTELEPORT,
	MF_NOSAVE,
	MF_NOBRANCH,
	MF_NOPENALTY,
	MF_NOZENYPENALTY,
	MF_PVP,
	MF_PVP_NOPARTY,
	MF_PVP_NOGUILD,
	MF_GVG,
	MF_GVG_NOPARTY,	//10
	MF_NOTRADE,
	MF_NOSKILL,
	MF_NOWARP,
	MF_PARTYLOCK,
	MF_NOICEWALL,
	MF_SNOW,
	MF_FOG,
	MF_SAKURA,
	MF_LEAVES,
	/* 21 - 22 free */
	MF_CLOUDS = 23,
	MF_CLOUDS2,
	MF_FIREWORKS,
	MF_GVG_CASTLE,
	MF_GVG_DUNGEON,
	MF_NIGHTENABLED,
	MF_NOBASEEXP,
	MF_NOJOBEXP,	//30
	MF_NOMOBLOOT,
	MF_NOMVPLOOT,
	MF_NORETURN,
	MF_NOWARPTO,
	MF_NIGHTMAREDROP,
	MF_ZONE,
	MF_NOCOMMAND,
	MF_NODROP,
	MF_JEXP,
	MF_BEXP,	//40
	MF_NOVENDING,
	MF_LOADEVENT,
	MF_NOCHAT,
	MF_NOEXPPENALTY,
	MF_GUILDLOCK,
	MF_TOWN,
	MF_AUTOTRADE,
	MF_ALLOWKS,
	MF_MONSTER_NOTELEPORT,
	MF_PVP_NOCALCRANK,	//50
	MF_BATTLEGROUND,
	MF_RESET
};

const char* script_op2name(int op)
{
#define RETURN_OP_NAME(type) case type: return #type
	switch( op )
	{
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

	default:
		ShowDebug("script_op2name: unexpected op=%d\n", op);
		return "???";
	}
#undef RETURN_OP_NAME
}

#ifdef DEBUG_DUMP_STACK
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
		ShowMessage("\t[%d] %s", i, script_op2name(data->type));
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
			ShowMessage(" \"%s\" (id=%d ref=%p subtype=%s)\n", reference_getname(data), data->u.num, data->ref, script_op2name(str_data[data->u.num].type));
			break;

		case C_RETINFO:
			{
				struct script_retinfo* ri = data->u.ri;
				ShowMessage(" %p {var_function=%p, script=%p, pos=%d, nargs=%d, defsp=%d}\n", ri, ri->var_function, ri->script, ri->pos, ri->nargs, ri->defsp);
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

	if( st->oid == 0 )
		return; //Can't report source.

	bl = map_id2bl(st->oid);
	if( bl == NULL )
		return;

	switch( bl->type ) {
		case BL_NPC:
			if( bl->m >= 0 )
				ShowDebug("Source (NPC): %s at %s (%d,%d)\n", ((struct npc_data *)bl)->name, map[bl->m].name, bl->x, bl->y);
			else
				ShowDebug("Source (NPC): %s (invisible/not on a map)\n", ((struct npc_data *)bl)->name);
			break;
		default:
			if( bl->m >= 0 )
				ShowDebug("Source (Non-NPC type %d): name %s at %s (%d,%d)\n", bl->type, status_get_name(bl), map[bl->m].name, bl->x, bl->y);
			else
				ShowDebug("Source (Non-NPC type %d): name %s (invisible/not on a map)\n", bl->type, status_get_name(bl));
			break;
	}
}

/// Reports on the console information about the script data.
static void script_reportdata(struct script_data* data)
{
	if( data == NULL )
		return;
	switch( data->type ) {
		case C_NOP:// no value
			ShowDebug("Data: nothing (nil)\n");
			break;
		case C_INT:// number
			ShowDebug("Data: number value=%d\n", data->u.num);
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
				if( not_array_variable(*name) )
					ShowDebug("Data: variable name='%s'\n", name);
				else
					ShowDebug("Data: variable name='%s' index=%d\n", name, reference_getindex(data));
			} else if( reference_toconstant(data) ) {// constant
				ShowDebug("Data: constant name='%s' value=%d\n", reference_getname(data), reference_getconstant(data));
			} else if( reference_toparam(data) ) {// param
				ShowDebug("Data: param name='%s' type=%d\n", reference_getname(data), reference_getparamtype(data));
			} else {// ???
				ShowDebug("Data: reference name='%s' type=%s\n", reference_getname(data), script_op2name(data->type));
				ShowDebug("Please report this!!! - str_data.type=%s\n", script_op2name(str_data[reference_getid(data)].type));
			}
			break;
		case C_POS:// label
			ShowDebug("Data: label pos=%d\n", data->u.num);
			break;
		default:
			ShowDebug("Data: %s\n", script_op2name(data->type));
			break;
	}
}


/// Reports on the console information about the current built-in function.
static void script_reportfunc(struct script_state* st)
{
	int i, params, id;
	struct script_data* data;

	if( !script_hasdata(st,0) )
	{// no stack
		return;
	}

	data = script_getdata(st,0);

	if( !data_isreference(data) || str_data[reference_getid(data)].type != C_FUNC )
	{// script currently not executing a built-in function or corrupt stack
		return;
	}

	id     = reference_getid(data);
	params = script_lastdata(st)-1;

	if( params > 0 )
	{
		ShowDebug("Function: %s (%d parameter%s):\n", get_str(id), params, ( params == 1 ) ? "" : "s");

		for( i = 2; i <= script_lastdata(st); i++ )
		{
			script_reportdata(script_getdata(st,i));
		}
	}
	else
	{
		ShowDebug("Function: %s (no parameters)\n", get_str(id));
	}
}


/*==========================================
 * Output error message
 *------------------------------------------*/
static void disp_error_message2(const char *mes,const char *pos,int report)
{
	error_msg = aStrdup(mes);
	error_pos = pos;
	error_report = report;
	longjmp( error_jump, 1 );
}
#define disp_error_message(mes,pos) disp_error_message2(mes,pos,1)

/// Checks event parameter validity
static void check_event(struct script_state *st, const char *evt)
{
	if( evt && evt[0] && !stristr(evt, "::On") )
	{
		ShowWarning("NPC event parameter deprecated! Please use 'NPCNAME::OnEVENT' instead of '%s'.\n", evt);
		script_reportsrc(st);
	}
}

/*==========================================
 * Hashes the input string
 *------------------------------------------*/
static unsigned int calc_hash(const char* p)
{
	unsigned int h;

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
	while( *p ){
		unsigned int g;
		h = ( h << 4 ) + ((unsigned char)TOLOWER(*p++));
		g = h & 0xF0000000;
		if( g )
		{
			h ^= g >> 24;
			h &= ~g;
		}
	}
#else // athena hash
	h = 0;
	while( *p )
		h = ( h << 1 ) + ( h >> 3 ) + ( h >> 5 ) + ( h >> 8 ) + (unsigned char)TOLOWER(*p++);
#endif

	return h % SCRIPT_HASH_SIZE;
}


/*==========================================
 * str_data manipulation functions
 *------------------------------------------*/

/// Looks up string using the provided id.
const char* get_str(int id)
{
	Assert( id >= LABEL_START && id < str_size );
	return str_buf+str_data[id].str;
}

/// Returns the uid of the string, or -1.
static int search_str(const char* p)
{
	int i;

	for( i = str_hash[calc_hash(p)]; i != 0; i = str_data[i].next )
		if( strcasecmp(get_str(i),p) == 0 )
			return i;

	return -1;
}

/// Stores a copy of the string and returns its id.
/// If an identical string is already present, returns its id instead.
int add_str(const char* p)
{
	int i, h;
	int len;

	h = calc_hash(p);

	if( str_hash[h] == 0 )
	{// empty bucket, add new node here
		str_hash[h] = str_num;
	}
	else
	{// scan for end of list, or occurence of identical string
		for( i = str_hash[h]; ; i = str_data[i].next )
		{
			if( strcasecmp(get_str(i),p) == 0 )
				return i; // string already in list
			if( str_data[i].next == 0 )
				break; // reached the end
		}

		// append node to end of list
		str_data[i].next = str_num;
	}

	// grow list if neccessary
	if( str_num >= str_data_size )
	{
		str_data_size += 128;
		RECREATE(str_data,struct str_data_struct,str_data_size);
		memset(str_data + (str_data_size - 128), '\0', 128);
	}

	len=(int)strlen(p);

	// grow string buffer if neccessary
	while( str_pos+len+1 >= str_size )
	{
		str_size += 256;
		RECREATE(str_buf,char,str_size);
		memset(str_buf + (str_size - 256), '\0', 256);
	}

	safestrncpy(str_buf+str_pos, p, len+1);
	str_data[str_num].type = C_NOP;
	str_data[str_num].str = str_pos;
	str_data[str_num].next = 0;
	str_data[str_num].func = NULL;
	str_data[str_num].backpatch = -1;
	str_data[str_num].label = -1;
	str_pos += len+1;

	return str_num++;
}


/// Appends 1 byte to the script buffer.
static void add_scriptb(int a)
{
	if( script_pos+1 >= script_size )
	{
		script_size += SCRIPT_BLOCK_SIZE;
		RECREATE(script_buf,unsigned char,script_size);
	}
	script_buf[script_pos++] = (uint8)(a);
}

/// Appends a c_op value to the script buffer.
/// The value is variable-length encoded into 8-bit blocks.
/// The encoding scheme is ( 01?????? )* 00??????, LSB first.
/// All blocks but the last hold 7 bits of data, topmost bit is always 1 (carries).
static void add_scriptc(int a)
{
	while( a >= 0x40 )
	{
		add_scriptb((a&0x3f)|0x40);
		a = (a - 0x40) >> 6;
	}

	add_scriptb(a);
}

/// Appends an integer value to the script buffer.
/// The value is variable-length encoded into 8-bit blocks.
/// The encoding scheme is ( 11?????? )* 10??????, LSB first.
/// All blocks but the last hold 7 bits of data, topmost bit is always 1 (carries).
static void add_scripti(int a)
{
	while( a >= 0x40 )
	{
		add_scriptb((a&0x3f)|0xc0);
		a = (a - 0x40) >> 6;
	}
	add_scriptb(a|0x80);
}

/// Appends a str_data object (label/function/variable/integer) to the script buffer.

///
/// @param l The id of the str_data entry
// Maximum up to 16M
static void add_scriptl(int l)
{
	int backpatch = str_data[l].backpatch;

	switch(str_data[l].type){
		case C_POS:
		case C_USERFUNC_POS:
			add_scriptc(C_POS);
			add_scriptb(str_data[l].label);
			add_scriptb(str_data[l].label>>8);
			add_scriptb(str_data[l].label>>16);
			break;
		case C_NOP:
		case C_USERFUNC:
			// Embedded data backpatch there is a possibility of label
			add_scriptc(C_NAME);
			str_data[l].backpatch = script_pos;
			add_scriptb(backpatch);
			add_scriptb(backpatch>>8);
			add_scriptb(backpatch>>16);
			break;
		case C_INT:
			add_scripti(abs(str_data[l].val));
			if( str_data[l].val < 0 ) //Notice that this is negative, from jA (Rayce)
				add_scriptc(C_NEG);
			break;
		default: // assume C_NAME
			add_scriptc(C_NAME);
			add_scriptb(l);
			add_scriptb(l>>8);
			add_scriptb(l>>16);
			break;
	}
}

/*==========================================
 * Resolve the label
 *------------------------------------------*/
void set_label(int l,int pos, const char* script_pos)
{
	int i,next;

	if(str_data[l].type==C_INT || str_data[l].type==C_PARAM || str_data[l].type==C_FUNC)
	{	//Prevent overwriting constants values, parameters and built-in functions [Skotlex]
		disp_error_message("set_label: invalid label name",script_pos);
		return;
	}
	if(str_data[l].label!=-1){
		disp_error_message("set_label: dup label ",script_pos);
		return;
	}
	str_data[l].type=(str_data[l].type == C_USERFUNC ? C_USERFUNC_POS : C_POS);
	str_data[l].label=pos;
	for(i=str_data[l].backpatch;i>=0 && i!=0x00ffffff;){
		next=GETVALUE(script_buf,i);
		script_buf[i-1]=(str_data[l].type == C_USERFUNC ? C_USERFUNC_POS : C_POS);
		SETVALUE(script_buf,i,pos);
		i=next;
	}
}

/// Skips spaces and/or comments.
const char* skip_space(const char* p)
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
				if( *p == '\0' )
					return p;//disp_error_message("script:skip_space: end of file while parsing block comment. expected "CL_BOLD"*/"CL_NORM, p);
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
/// A word consists of undercores and/or alfanumeric characters,
/// and valid variable prefixes/postfixes.
static
const char* skip_word(const char* p)
{
	// prefix
	switch( *p )
	{
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

	while( ISALNUM(*p) || *p == '_' )
		++p;

	// postfix
	if( *p == '$' )// string
		p++;

	return p;
}

/// Adds a word to str_data.
/// @see skip_word
/// @see add_str
static
int add_word(const char* p)
{
	char* word;
	int len;
	int i;

	// Check for a word
	len = skip_word(p) - p;
	if( len == 0 )
		disp_error_message("script:add_word: invalid word. A word consists of undercores and/or alfanumeric characters, and valid variable prefixes/postfixes.", p);

	// Duplicate the word
	word = (char*)aMalloc(len+1);
	memcpy(word, p, len);
	word[len] = 0;

	// add the word
	i = add_str(word);
	aFree(word);
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
	int func;

	func = add_word(p);
	if( str_data[func].type == C_FUNC ){
		char argT = 0;
		// buildin function
		add_scriptl(func);
		add_scriptc(C_ARG);
		arg = script->buildin[str_data[func].val];
		if( !arg ) arg = &argT;
	} else if( str_data[func].type == C_USERFUNC || str_data[func].type == C_USERFUNC_POS ){
		// script defined function
		add_scriptl(buildin_callsub_ref);
		add_scriptc(C_ARG);
		add_scriptl(func);
		arg = script->buildin[str_data[buildin_callsub_ref].val];
		if( *arg == 0 )
			disp_error_message("parse_callfunc: callsub has no arguments, please review it's definition",p);
		if( *arg != '*' )
			++arg; // count func as argument
	} else {
#ifdef SCRIPT_CALLFUNC_CHECK
		const char* name = get_str(func);
		if( !is_custom && strdb_get(userfunc_db, name) == NULL ) {
#endif
			disp_error_message("parse_line: expect command, missing function name or calling undeclared function",p);
#ifdef SCRIPT_CALLFUNC_CHECK
		} else {;
			add_scriptl(buildin_callfunc_ref);
			add_scriptc(C_ARG);
			add_scriptc(C_STR);
			while( *name ) add_scriptb(*name ++);
			add_scriptb(0);
			arg = script->buildin[str_data[buildin_callfunc_ref].val];
			if( *arg != '*' ) ++ arg;
		}
#endif
	}

	p = skip_word(p);
	p = skip_space(p);
	syntax.curly[syntax.curly_count].type = TYPE_ARGLIST;
	syntax.curly[syntax.curly_count].count = 0;
	if( *p == ';' )
	{// <func name> ';'
		syntax.curly[syntax.curly_count].flag = ARGLIST_NO_PAREN;
	} else if( *p == '(' && *(p2=skip_space(p+1)) == ')' )
	{// <func name> '(' ')'
		syntax.curly[syntax.curly_count].flag = ARGLIST_PAREN;
		p = p2;
	/*
	} else if( 0 && require_paren && *p != '(' )
	{// <func name>
		syntax.curly[syntax.curly_count].flag = ARGLIST_NO_PAREN;
	*/
	} else {// <func name> <arg list>
		if( require_paren ){
			if( *p != '(' )
				disp_error_message("need '('",p);
			++p; // skip '('
			syntax.curly[syntax.curly_count].flag = ARGLIST_PAREN;
		} else if( *p == '(' ){
			syntax.curly[syntax.curly_count].flag = ARGLIST_UNDEFINED;
		} else {
			syntax.curly[syntax.curly_count].flag = ARGLIST_NO_PAREN;
		}
		++syntax.curly_count;
		while( *arg ) {
			p2=parse_subexpr(p,-1);
			if( p == p2 )
				break; // not an argument
			if( *arg != '*' )
				++arg; // next argument

			p=skip_space(p2);
			if( *arg == 0 || *p != ',' )
				break; // no more arguments
			++p; // skip comma
		}
		--syntax.curly_count;
	}
	if( arg && *arg && *arg != '?' && *arg != '*' )
		disp_error_message2("parse_callfunc: not enough arguments, expected ','", p, script_config.warn_func_mismatch_paramnum);
	if( syntax.curly[syntax.curly_count].type != TYPE_ARGLIST )
		disp_error_message("parse_callfunc: DEBUG last curly is not an argument list",p);
	if( syntax.curly[syntax.curly_count].flag == ARGLIST_PAREN ){
		if( *p != ')' )
			disp_error_message("parse_callfunc: expected ')' to close argument list",p);
		++p;
	}
	add_scriptc(C_FUNC);
	return p;
}

/// Processes end of logical script line.
/// @param first When true, only fix up scheduling data is initialized
/// @param p Script position for error reporting in set_label
static void parse_nextline(bool first, const char* p)
{
	if( !first )
	{
		add_scriptc(C_EOL);  // mark end of line for stack cleanup
		set_label(LABEL_NEXTLINE, script_pos, p);  // fix up '-' labels
	}

	// initialize data for new '-' label fix up scheduling
	str_data[LABEL_NEXTLINE].type      = C_NOP;
	str_data[LABEL_NEXTLINE].backpatch = -1;
	str_data[LABEL_NEXTLINE].label     = -1;
}

/// Parse a variable assignment using the direct equals operator
/// @param p script position where the function should run from
/// @return NULL if not a variable assignment, the new position otherwise
const char* parse_variable(const char* p) {
	int i, j, word;
	c_op type = C_NOP;
	const char *p2 = NULL;
	const char *var = p;

	// skip the variable where applicable
	p = skip_word(p);
	p = skip_space(p);

	if( p == NULL ) {// end of the line or invalid buffer
		return NULL;
	}

	if( *p == '[' ) {// array variable so process the array as appropriate
		for( p2 = p, i = 0, j = 1; p; ++ i ) {
			if( *p ++ == ']' && --(j) == 0 ) break;
			if( *p == '[' ) ++ j;
		}

		if( !(p = skip_space(p)) ) {// end of line or invalid characters remaining
			disp_error_message("Missing right expression or closing bracket for variable.", p);
		}
	}

	if( type == C_NOP &&
	!( ( p[0] == '=' && p[1] != '=' && (type = C_EQ) ) // =
	|| ( p[0] == '+' && p[1] == '=' && (type = C_ADD) ) // +=
	|| ( p[0] == '-' && p[1] == '=' && (type = C_SUB) ) // -=
	|| ( p[0] == '^' && p[1] == '=' && (type = C_XOR) ) // ^=
	|| ( p[0] == '|' && p[1] == '=' && (type = C_OR ) ) // |=
	|| ( p[0] == '&' && p[1] == '=' && (type = C_AND) ) // &=
	|| ( p[0] == '*' && p[1] == '=' && (type = C_MUL) ) // *=
	|| ( p[0] == '/' && p[1] == '=' && (type = C_DIV) ) // /=
	|| ( p[0] == '%' && p[1] == '=' && (type = C_MOD) ) // %=
	|| ( p[0] == '~' && p[1] == '=' && (type = C_NOT) ) // ~=
	|| ( p[0] == '+' && p[1] == '+' && (type = C_ADD_PP) ) // ++
	|| ( p[0] == '-' && p[1] == '-' && (type = C_SUB_PP) ) // --
	|| ( p[0] == '<' && p[1] == '<' && p[2] == '=' && (type = C_L_SHIFT) ) // <<=
	|| ( p[0] == '>' && p[1] == '>' && p[2] == '=' && (type = C_R_SHIFT) ) // >>=
	) )
	{// failed to find a matching operator combination so invalid
		return NULL;
	}

	switch( type ) {
		case C_EQ: {// incremental modifier
			p = skip_space( &p[1] );
		}
		break;

		case C_L_SHIFT:
		case C_R_SHIFT: {// left or right shift modifier
			p = skip_space( &p[3] );
		}
		break;

		default: {// normal incremental command
			p = skip_space( &p[2] );
		}
	}

	if( p == NULL ) {// end of line or invalid buffer
		return NULL;
	}

	// push the set function onto the stack
	add_scriptl(buildin_set_ref);
	add_scriptc(C_ARG);

	// always append parenthesis to avoid errors
	syntax.curly[syntax.curly_count].type = TYPE_ARGLIST;
	syntax.curly[syntax.curly_count].count = 0;
	syntax.curly[syntax.curly_count].flag = ARGLIST_PAREN;

	// increment the total curly count for the position in the script
	++ syntax.curly_count;

	// parse the variable currently being modified
	word = add_word(var);

	if( str_data[word].type == C_FUNC || str_data[word].type == C_USERFUNC || str_data[word].type == C_USERFUNC_POS )
	{// cannot assign a variable which exists as a function or label
		disp_error_message("Cannot modify a variable which has the same name as a function or label.", p);
	}

	if( p2 ) {// process the variable index
		const char* p3 = NULL;

		// push the getelementofarray method into the stack
		add_scriptl(buildin_getelementofarray_ref);
		add_scriptc(C_ARG);
		add_scriptl(word);

		// process the sub-expression for this assignment
		p3 = parse_subexpr(p2 + 1, 1);
		p3 = skip_space(p3);

		if( *p3 != ']' ) {// closing parenthesis is required for this script
			disp_error_message("Missing closing ']' parenthesis for the variable assignment.", p3);
		}

		// push the closing function stack operator onto the stack
		add_scriptc(C_FUNC);
		p3 ++;
	} else {// simply push the variable or value onto the stack
		add_scriptl(word);
	}

	if( type != C_EQ )
		add_scriptc(C_REF);

	if( type == C_ADD_PP || type == C_SUB_PP ) {// incremental operator for the method
		add_scripti(1);
		add_scriptc(type == C_ADD_PP ? C_ADD : C_SUB);
	} else {// process the value as an expression
		p = parse_subexpr(p, -1);

		if( type != C_EQ )
		{// push the type of modifier onto the stack
			add_scriptc(type);
		}
	}

	// decrement the curly count for the position within the script
	-- syntax.curly_count;

	// close the script by appending the function operator
	add_scriptc(C_FUNC);

	// push the buffer from the method
	return p;
}

/*==========================================
 * Analysis section
 *------------------------------------------*/
const char* parse_simpleexpr(const char *p)
{
	int i;
	p=skip_space(p);

	if(*p==';' || *p==',')
		disp_error_message("parse_simpleexpr: unexpected expr end",p);
	if(*p=='('){
		if( (i=syntax.curly_count-1) >= 0 && syntax.curly[i].type == TYPE_ARGLIST )
			++syntax.curly[i].count;
		p=parse_subexpr(p+1,-1);
		p=skip_space(p);
		if( (i=syntax.curly_count-1) >= 0 && syntax.curly[i].type == TYPE_ARGLIST &&
				syntax.curly[i].flag == ARGLIST_UNDEFINED && --syntax.curly[i].count == 0
		){
			if( *p == ',' ){
				syntax.curly[i].flag = ARGLIST_PAREN;
				return p;
			} else
				syntax.curly[i].flag = ARGLIST_NO_PAREN;
		}
		if( *p != ')' )
			disp_error_message("parse_simpleexpr: unmatch ')'",p);
		++p;
	} else if(ISDIGIT(*p) || ((*p=='-' || *p=='+') && ISDIGIT(p[1]))){
		char *np;
		while(*p == '0' && ISDIGIT(p[1])) p++;
		i=strtoul(p,&np,0);
		add_scripti(i);
		p=np;
	} else if(*p=='"'){
		add_scriptc(C_STR);
		p++;
		while( *p && *p != '"' ){
			if( (unsigned char)p[-1] <= 0x7e && *p == '\\' ) {
				char buf[8];
				size_t len = sv->skip_escaped_c(p) - p;
				size_t n = sv->unescape_c(buf, p, len);
				if( n != 1 )
					ShowDebug("parse_simpleexpr: unexpected length %d after unescape (\"%.*s\" -> %.*s)\n", (int)n, (int)len, p, (int)n, buf);
				p += len;
				add_scriptb(*buf);
				continue;
			} else if( *p == '\n' )
				disp_error_message("parse_simpleexpr: unexpected newline @ string",p);
			add_scriptb(*p++);
		}
		if(!*p)
			disp_error_message("parse_simpleexpr: unexpected eof @ string",p);
		add_scriptb(0);
		p++;	//'"'
	} else {
		int l;
		const char* pv;

		// label , register , function etc
		if(skip_word(p)==p)
			disp_error_message("parse_simpleexpr: unexpected character",p);

		l=add_word(p);
		if( str_data[l].type == C_FUNC || str_data[l].type == C_USERFUNC || str_data[l].type == C_USERFUNC_POS)
			return parse_callfunc(p,1,0);
#ifdef SCRIPT_CALLFUNC_CHECK
		else {
			const char* name = get_str(l);
			if( strdb_get(userfunc_db,name) != NULL ) {
				return parse_callfunc(p,1,1);
			}
		}
#endif

		if( (pv = parse_variable(p)) )
		{// successfully processed a variable assignment
			return pv;
		}

		p=skip_word(p);
		if( *p == '[' ){
			// array(name[i] => getelementofarray(name,i) )
			add_scriptl(buildin_getelementofarray_ref);
			add_scriptc(C_ARG);
			add_scriptl(l);

			p=parse_subexpr(p+1,-1);
			p=skip_space(p);
			if( *p != ']' )
				disp_error_message("parse_simpleexpr: unmatch ']'",p);
			++p;
			add_scriptc(C_FUNC);
		}else
			add_scriptl(l);

	}

	return p;
}

/*==========================================
 * Analysis of the expression
 *------------------------------------------*/
const char* parse_subexpr(const char* p,int limit)
{
	int op,opl,len;
	const char* tmpp;

	p=skip_space(p);

	if( *p == '-' ){
		 tmpp = skip_space(p+1);
		if( *tmpp == ';' || *tmpp == ',' ){
			add_scriptl(LABEL_NEXTLINE);
			p++;
			return p;
		}
	}

	if((op=C_NEG,*p=='-') || (op=C_LNOT,*p=='!') || (op=C_NOT,*p=='~')){
		p=parse_subexpr(p+1,10);
		add_scriptc(op);
	} else
		p=parse_simpleexpr(p);
	p=skip_space(p);
	while((
			(op=C_OP3,opl=0,len=1,*p=='?') ||
			(op=C_ADD,opl=8,len=1,*p=='+') ||
			(op=C_SUB,opl=8,len=1,*p=='-') ||
			(op=C_MUL,opl=9,len=1,*p=='*') ||
			(op=C_DIV,opl=9,len=1,*p=='/') ||
			(op=C_MOD,opl=9,len=1,*p=='%') ||
			(op=C_LAND,opl=2,len=2,*p=='&' && p[1]=='&') ||
			(op=C_AND,opl=6,len=1,*p=='&') ||
			(op=C_LOR,opl=1,len=2,*p=='|' && p[1]=='|') ||
			(op=C_OR,opl=5,len=1,*p=='|') ||
			(op=C_XOR,opl=4,len=1,*p=='^') ||
			(op=C_EQ,opl=3,len=2,*p=='=' && p[1]=='=') ||
			(op=C_NE,opl=3,len=2,*p=='!' && p[1]=='=') ||
			(op=C_R_SHIFT,opl=7,len=2,*p=='>' && p[1]=='>') ||
			(op=C_GE,opl=3,len=2,*p=='>' && p[1]=='=') ||
			(op=C_GT,opl=3,len=1,*p=='>') ||
			(op=C_L_SHIFT,opl=7,len=2,*p=='<' && p[1]=='<') ||
			(op=C_LE,opl=3,len=2,*p=='<' && p[1]=='=') ||
			(op=C_LT,opl=3,len=1,*p=='<')) && opl>limit){
		p+=len;
		if(op == C_OP3) {
			p=parse_subexpr(p,-1);
			p=skip_space(p);
			if( *(p++) != ':')
				disp_error_message("parse_subexpr: need ':'", p-1);
			p=parse_subexpr(p,-1);
		} else {
			p=parse_subexpr(p,opl);
		}
		add_scriptc(op);
		p=skip_space(p);
	}

	return p;  /* return first untreated operator */
}

/*==========================================
 * Evaluation of the expression
 *------------------------------------------*/
const char* parse_expr(const char *p)
{
	switch(*p){
	case ')': case ';': case ':': case '[': case ']':
	case '}':
		disp_error_message("parse_expr: unexpected char",p);
	}
	p=parse_subexpr(p,-1);
	return p;
}

/*==========================================
 * Analysis of the line
 *------------------------------------------*/
const char* parse_line(const char* p)
{
	const char* p2;

	p=skip_space(p);
	if(*p==';') {
		//Close decision for if(); for(); while();
		p = parse_syntax_close(p + 1);
		return p;
	}
	if(*p==')' && parse_syntax_for_flag)
		return p+1;

	p = skip_space(p);
	if(p[0] == '{') {
		syntax.curly[syntax.curly_count].type  = TYPE_NULL;
		syntax.curly[syntax.curly_count].count = -1;
		syntax.curly[syntax.curly_count].index = -1;
		syntax.curly_count++;
		return p + 1;
	} else if(p[0] == '}') {
		return parse_curly_close(p);
	}

	// Syntax-related processing
	p2 = parse_syntax(p);
	if(p2 != NULL)
		return p2;

	// attempt to process a variable assignment
	p2 = parse_variable(p);

	if( p2 != NULL )
	{// variable assignment processed so leave the method
		return parse_syntax_close(p2 + 1);
	}

	p = parse_callfunc(p,0,0);
	p = skip_space(p);

	if(parse_syntax_for_flag) {
		if( *p != ')' )
			disp_error_message("parse_line: need ')'",p);
	} else {
		if( *p != ';' )
			disp_error_message("parse_line: need ';'",p);
	}

	//Binding decision for if(), for(), while()
	p = parse_syntax_close(p+1);

	return p;
}

// { ... } Closing process
const char* parse_curly_close(const char* p)
{
	if(syntax.curly_count <= 0) {
		disp_error_message("parse_curly_close: unexpected string",p);
		return p + 1;
	} else if(syntax.curly[syntax.curly_count-1].type == TYPE_NULL) {
		syntax.curly_count--;
		//Close decision  if, for , while
		p = parse_syntax_close(p + 1);
		return p;
	} else if(syntax.curly[syntax.curly_count-1].type == TYPE_SWITCH) {
		//Closing switch()
		int pos = syntax.curly_count-1;
		char label[256];
		int l;
		// Remove temporary variables
		sprintf(label,"set $@__SW%x_VAL,0;",syntax.curly[pos].index);
		syntax.curly[syntax.curly_count++].type = TYPE_NULL;
		parse_line(label);
		syntax.curly_count--;

		// Go to the end pointer unconditionally
		sprintf(label,"goto __SW%x_FIN;",syntax.curly[pos].index);
		syntax.curly[syntax.curly_count++].type = TYPE_NULL;
		parse_line(label);
		syntax.curly_count--;

		// You are here labeled
		sprintf(label,"__SW%x_%x",syntax.curly[pos].index,syntax.curly[pos].count);
		l=add_str(label);
		set_label(l,script_pos, p);

		if(syntax.curly[pos].flag) {
			//Exists default
			sprintf(label,"goto __SW%x_DEF;",syntax.curly[pos].index);
			syntax.curly[syntax.curly_count++].type = TYPE_NULL;
			parse_line(label);
			syntax.curly_count--;
		}

		// Label end
		sprintf(label,"__SW%x_FIN",syntax.curly[pos].index);
		l=add_str(label);
		set_label(l,script_pos, p);
		linkdb_final(&syntax.curly[pos].case_label);	// free the list of case label
		syntax.curly_count--;
		//Closing decision if, for , while
		p = parse_syntax_close(p + 1);
		return p;
	} else {
		disp_error_message("parse_curly_close: unexpected string",p);
		return p + 1;
	}
}

// Syntax-related processing
//	 break, case, continue, default, do, for, function,
//	 if, switch, while ? will handle this internally.
const char* parse_syntax(const char* p)
{
	const char *p2 = skip_word(p);

	switch(*p) {
	case 'B':
	case 'b':
		if(p2 - p == 5 && !strncasecmp(p,"break",5)) {
			// break Processing
			char label[256];
			int pos = syntax.curly_count - 1;
			while(pos >= 0) {
				if(syntax.curly[pos].type == TYPE_DO) {
					sprintf(label,"goto __DO%x_FIN;",syntax.curly[pos].index);
					break;
				} else if(syntax.curly[pos].type == TYPE_FOR) {
					sprintf(label,"goto __FR%x_FIN;",syntax.curly[pos].index);
					break;
				} else if(syntax.curly[pos].type == TYPE_WHILE) {
					sprintf(label,"goto __WL%x_FIN;",syntax.curly[pos].index);
					break;
				} else if(syntax.curly[pos].type == TYPE_SWITCH) {
					sprintf(label,"goto __SW%x_FIN;",syntax.curly[pos].index);
					break;
				}
				pos--;
			}
			if(pos < 0) {
				disp_error_message("parse_syntax: unexpected 'break'",p);
			} else {
				syntax.curly[syntax.curly_count++].type = TYPE_NULL;
				parse_line(label);
				syntax.curly_count--;
			}
			p = skip_space(p2);
			if(*p != ';')
				disp_error_message("parse_syntax: need ';'",p);
			// Closing decision if, for , while
			p = parse_syntax_close(p + 1);
			return p;
		}
		break;
	case 'c':
	case 'C':
		if(p2 - p == 4 && !strncasecmp(p,"case",4)) {
			//Processing case
			int pos = syntax.curly_count-1;
			if(pos < 0 || syntax.curly[pos].type != TYPE_SWITCH) {
				disp_error_message("parse_syntax: unexpected 'case' ",p);
				return p+1;
			} else {
				char label[256];
				int  l,v;
				char *np;
				if(syntax.curly[pos].count != 1) {
					//Jump for FALLTHRU
					sprintf(label,"goto __SW%x_%xJ;",syntax.curly[pos].index,syntax.curly[pos].count);
					syntax.curly[syntax.curly_count++].type = TYPE_NULL;
					parse_line(label);
					syntax.curly_count--;

					// You are here labeled
					sprintf(label,"__SW%x_%x",syntax.curly[pos].index,syntax.curly[pos].count);
					l=add_str(label);
					set_label(l,script_pos, p);
				}
				//Decision statement switch
				p = skip_space(p2);
				if(p == p2) {
					disp_error_message("parse_syntax: expect space ' '",p);
				}
				// check whether case label is integer or not
				v = strtol(p,&np,0);
				if(np == p) { //Check for constants
					p2 = skip_word(p);
					v = p2-p; // length of word at p2
					memcpy(label,p,v);
					label[v]='\0';
					if( !script_get_constant(label, &v) )
						disp_error_message("parse_syntax: 'case' label not integer",p);
					p = skip_word(p);
				} else { //Numeric value
					if((*p == '-' || *p == '+') && ISDIGIT(p[1]))	// pre-skip because '-' can not skip_word
						p++;
					p = skip_word(p);
					if(np != p)
						disp_error_message("parse_syntax: 'case' label not integer",np);
				}
				p = skip_space(p);
				if(*p != ':')
					disp_error_message("parse_syntax: expect ':'",p);
				sprintf(label,"if(%d != $@__SW%x_VAL) goto __SW%x_%x;",
					v,syntax.curly[pos].index,syntax.curly[pos].index,syntax.curly[pos].count+1);
				syntax.curly[syntax.curly_count++].type = TYPE_NULL;
				// Bad I do not parse twice
				p2 = parse_line(label);
				parse_line(p2);
				syntax.curly_count--;
				if(syntax.curly[pos].count != 1) {
					// Label after the completion of FALLTHRU
					sprintf(label,"__SW%x_%xJ",syntax.curly[pos].index,syntax.curly[pos].count);
					l=add_str(label);
					set_label(l,script_pos,p);
				}
				// check duplication of case label [Rayce]
				if(linkdb_search(&syntax.curly[pos].case_label, (void*)__64BPTRSIZE(v)) != NULL)
					disp_error_message("parse_syntax: dup 'case'",p);
				linkdb_insert(&syntax.curly[pos].case_label, (void*)__64BPTRSIZE(v), (void*)1);

				sprintf(label,"set $@__SW%x_VAL,0;",syntax.curly[pos].index);
				syntax.curly[syntax.curly_count++].type = TYPE_NULL;

				parse_line(label);
				syntax.curly_count--;
				syntax.curly[pos].count++;
			}
			return p + 1;
		} else if(p2 - p == 8 && !strncasecmp(p,"continue",8)) {
			// Processing continue
			char label[256];
			int pos = syntax.curly_count - 1;
			while(pos >= 0) {
				if(syntax.curly[pos].type == TYPE_DO) {
					sprintf(label,"goto __DO%x_NXT;",syntax.curly[pos].index);
					syntax.curly[pos].flag = 1; //Flag put the link for continue
					break;
				} else if(syntax.curly[pos].type == TYPE_FOR) {
					sprintf(label,"goto __FR%x_NXT;",syntax.curly[pos].index);
					break;
				} else if(syntax.curly[pos].type == TYPE_WHILE) {
					sprintf(label,"goto __WL%x_NXT;",syntax.curly[pos].index);
					break;
				}
				pos--;
			}
			if(pos < 0) {
				disp_error_message("parse_syntax: unexpected 'continue'",p);
			} else {
				syntax.curly[syntax.curly_count++].type = TYPE_NULL;
				parse_line(label);
				syntax.curly_count--;
			}
			p = skip_space(p2);
			if(*p != ';')
				disp_error_message("parse_syntax: need ';'",p);
			//Closing decision if, for , while
			p = parse_syntax_close(p + 1);
			return p;
		}
		break;
	case 'd':
	case 'D':
		if(p2 - p == 7 && !strncasecmp(p,"default",7)) {
			// Switch - default processing
			int pos = syntax.curly_count-1;
			if(pos < 0 || syntax.curly[pos].type != TYPE_SWITCH) {
				disp_error_message("parse_syntax: unexpected 'default'",p);
			} else if(syntax.curly[pos].flag) {
				disp_error_message("parse_syntax: dup 'default'",p);
			} else {
				char label[256];
				int l;
				// Put the label location
				p = skip_space(p2);
				if(*p != ':') {
					disp_error_message("parse_syntax: need ':'",p);
				}
				sprintf(label,"__SW%x_%x",syntax.curly[pos].index,syntax.curly[pos].count);
				l=add_str(label);
				set_label(l,script_pos,p);

				// Skip to the next link w/o condition
				sprintf(label,"goto __SW%x_%x;",syntax.curly[pos].index,syntax.curly[pos].count+1);
				syntax.curly[syntax.curly_count++].type = TYPE_NULL;
				parse_line(label);
				syntax.curly_count--;

				// The default label
				sprintf(label,"__SW%x_DEF",syntax.curly[pos].index);
				l=add_str(label);
				set_label(l,script_pos,p);

				syntax.curly[syntax.curly_count - 1].flag = 1;
				syntax.curly[pos].count++;
			}
			return p + 1;
		} else if(p2 - p == 2 && !strncasecmp(p,"do",2)) {
			int l;
			char label[256];
			p=skip_space(p2);

			syntax.curly[syntax.curly_count].type  = TYPE_DO;
			syntax.curly[syntax.curly_count].count = 1;
			syntax.curly[syntax.curly_count].index = syntax.index++;
			syntax.curly[syntax.curly_count].flag  = 0;
			// Label of the (do) form here
			sprintf(label,"__DO%x_BGN",syntax.curly[syntax.curly_count].index);
			l=add_str(label);
			set_label(l,script_pos,p);
			syntax.curly_count++;
			return p;
		}
		break;
	case 'f':
	case 'F':
		if(p2 - p == 3 && !strncasecmp(p,"for",3)) {
			int l;
			char label[256];
			int  pos = syntax.curly_count;
			syntax.curly[syntax.curly_count].type  = TYPE_FOR;
			syntax.curly[syntax.curly_count].count = 1;
			syntax.curly[syntax.curly_count].index = syntax.index++;
			syntax.curly[syntax.curly_count].flag  = 0;
			syntax.curly_count++;

			p=skip_space(p2);

			if(*p != '(')
				disp_error_message("parse_syntax: need '('",p);
			p++;

			// Execute the initialization statement
			syntax.curly[syntax.curly_count++].type = TYPE_NULL;
			p=parse_line(p);
			syntax.curly_count--;

			// Form the start of label decision
			sprintf(label,"__FR%x_J",syntax.curly[pos].index);
			l=add_str(label);
			set_label(l,script_pos,p);

			p=skip_space(p);
			if(*p == ';') {
				// For (; Because the pattern of always true ;)
				;
			} else {
				// Skip to the end point if the condition is false
				sprintf(label,"__FR%x_FIN",syntax.curly[pos].index);
				add_scriptl(add_str("jump_zero"));
				add_scriptc(C_ARG);
				p=parse_expr(p);
				p=skip_space(p);
				add_scriptl(add_str(label));
				add_scriptc(C_FUNC);
			}
			if(*p != ';')
				disp_error_message("parse_syntax: need ';'",p);
			p++;

			// Skip to the beginning of the loop
			sprintf(label,"goto __FR%x_BGN;",syntax.curly[pos].index);
			syntax.curly[syntax.curly_count++].type = TYPE_NULL;
			parse_line(label);
			syntax.curly_count--;

			// Labels to form the next loop
			sprintf(label,"__FR%x_NXT",syntax.curly[pos].index);
			l=add_str(label);
			set_label(l,script_pos,p);

			// Process the next time you enter the loop
			// A ')' last for; flag to be treated as'
			parse_syntax_for_flag = 1;
			syntax.curly[syntax.curly_count++].type = TYPE_NULL;
			p=parse_line(p);
			syntax.curly_count--;
			parse_syntax_for_flag = 0;

			// Skip to the determination process conditions
			sprintf(label,"goto __FR%x_J;",syntax.curly[pos].index);
			syntax.curly[syntax.curly_count++].type = TYPE_NULL;
			parse_line(label);
			syntax.curly_count--;

			// Loop start labeling
			sprintf(label,"__FR%x_BGN",syntax.curly[pos].index);
			l=add_str(label);
			set_label(l,script_pos,p);
			return p;
		}
		else if( p2 - p == 8 && strncasecmp(p,"function",8) == 0 )
		{// internal script function
			const char *func_name;

			func_name = skip_space(p2);
			p = skip_word(func_name);
			if( p == func_name )
				disp_error_message("parse_syntax:function: function name is missing or invalid", p);
			p2 = skip_space(p);
			if( *p2 == ';' )
			{// function <name> ;
				// function declaration - just register the name
				int l;
				l = add_word(func_name);
				if( str_data[l].type == C_NOP )// register only, if the name was not used by something else
					str_data[l].type = C_USERFUNC;
				else if( str_data[l].type == C_USERFUNC )
					;  // already registered
				else
					disp_error_message("parse_syntax:function: function name is invalid", func_name);

				// Close condition of if, for, while
				p = parse_syntax_close(p2 + 1);
				return p;
			}
			else if(*p2 == '{')
			{// function <name> <line/block of code>
				char label[256];
				int l;

				syntax.curly[syntax.curly_count].type  = TYPE_USERFUNC;
				syntax.curly[syntax.curly_count].count = 1;
				syntax.curly[syntax.curly_count].index = syntax.index++;
				syntax.curly[syntax.curly_count].flag  = 0;
				++syntax.curly_count;

				// Jump over the function code
				sprintf(label, "goto __FN%x_FIN;", syntax.curly[syntax.curly_count-1].index);
				syntax.curly[syntax.curly_count].type = TYPE_NULL;
				++syntax.curly_count;
				parse_line(label);
				--syntax.curly_count;

				// Set the position of the function (label)
				l=add_word(func_name);
				if( str_data[l].type == C_NOP || str_data[l].type == C_USERFUNC )// register only, if the name was not used by something else
				{
					str_data[l].type = C_USERFUNC;
					set_label(l, script_pos, p);
					if( parse_options&SCRIPT_USE_LABEL_DB )
						strdb_iput(scriptlabel_db, get_str(l), script_pos);
				}
				else
					disp_error_message("parse_syntax:function: function name is invalid", func_name);

				return skip_space(p);
			}
			else
			{
				disp_error_message("expect ';' or '{' at function syntax",p);
			}
		}
		break;
	case 'i':
	case 'I':
		if(p2 - p == 2 && !strncasecmp(p,"if",2)) {
			// If process
			char label[256];
			p=skip_space(p2);
			if(*p != '(') { //Prevent if this {} non-c syntax. from Rayce (jA)
				disp_error_message("need '('",p);
			}
			syntax.curly[syntax.curly_count].type  = TYPE_IF;
			syntax.curly[syntax.curly_count].count = 1;
			syntax.curly[syntax.curly_count].index = syntax.index++;
			syntax.curly[syntax.curly_count].flag  = 0;
			sprintf(label,"__IF%x_%x",syntax.curly[syntax.curly_count].index,syntax.curly[syntax.curly_count].count);
			syntax.curly_count++;
			add_scriptl(add_str("jump_zero"));
			add_scriptc(C_ARG);
			p=parse_expr(p);
			p=skip_space(p);
			add_scriptl(add_str(label));
			add_scriptc(C_FUNC);
			return p;
		}
		break;
	case 's':
	case 'S':
		if(p2 - p == 6 && !strncasecmp(p,"switch",6)) {
			// Processing of switch ()
			char label[256];
			p=skip_space(p2);
			if(*p != '(') {
				disp_error_message("need '('",p);
			}
			syntax.curly[syntax.curly_count].type  = TYPE_SWITCH;
			syntax.curly[syntax.curly_count].count = 1;
			syntax.curly[syntax.curly_count].index = syntax.index++;
			syntax.curly[syntax.curly_count].flag  = 0;
			sprintf(label,"$@__SW%x_VAL",syntax.curly[syntax.curly_count].index);
			syntax.curly_count++;
			add_scriptl(add_str("set"));
			add_scriptc(C_ARG);
			add_scriptl(add_str(label));
			p=parse_expr(p);
			p=skip_space(p);
			if(*p != '{') {
				disp_error_message("parse_syntax: need '{'",p);
			}
			add_scriptc(C_FUNC);
			return p + 1;
		}
		break;
	case 'w':
	case 'W':
		if(p2 - p == 5 && !strncasecmp(p,"while",5)) {
			int l;
			char label[256];
			p=skip_space(p2);
			if(*p != '(') {
				disp_error_message("need '('",p);
			}
			syntax.curly[syntax.curly_count].type  = TYPE_WHILE;
			syntax.curly[syntax.curly_count].count = 1;
			syntax.curly[syntax.curly_count].index = syntax.index++;
			syntax.curly[syntax.curly_count].flag  = 0;
			// Form the start of label decision
			sprintf(label,"__WL%x_NXT",syntax.curly[syntax.curly_count].index);
			l=add_str(label);
			set_label(l,script_pos,p);

			// Skip to the end point if the condition is false
			sprintf(label,"__WL%x_FIN",syntax.curly[syntax.curly_count].index);
			syntax.curly_count++;
			add_scriptl(add_str("jump_zero"));
			add_scriptc(C_ARG);
			p=parse_expr(p);
			p=skip_space(p);
			add_scriptl(add_str(label));
			add_scriptc(C_FUNC);
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
		p = parse_syntax_close_sub(p,&flag);
	} while(flag);
	return p;
}

// Close judgment if, for, while, of do
//	 flag == 1 : closed
//	 flag == 0 : not closed
const char* parse_syntax_close_sub(const char* p,int* flag)
{
	char label[256];
	int pos = syntax.curly_count - 1;
	int l;
	*flag = 1;

	if(syntax.curly_count <= 0) {
		*flag = 0;
		return p;
	} else if(syntax.curly[pos].type == TYPE_IF) {
		const char *bp = p;
		const char *p2;

		// if-block and else-block end is a new line
		parse_nextline(false, p);

		// Skip to the last location if
		sprintf(label,"goto __IF%x_FIN;",syntax.curly[pos].index);
		syntax.curly[syntax.curly_count++].type = TYPE_NULL;
		parse_line(label);
		syntax.curly_count--;

		// Put the label of the location
		sprintf(label,"__IF%x_%x",syntax.curly[pos].index,syntax.curly[pos].count);
		l=add_str(label);
		set_label(l,script_pos,p);

		syntax.curly[pos].count++;
		p = skip_space(p);
		p2 = skip_word(p);
		if(!syntax.curly[pos].flag && p2 - p == 4 && !strncasecmp(p,"else",4)) {
			// else  or else - if
			p = skip_space(p2);
			p2 = skip_word(p);
			if(p2 - p == 2 && !strncasecmp(p,"if",2)) {
				// else - if
				p=skip_space(p2);
				if(*p != '(') {
					disp_error_message("need '('",p);
				}
				sprintf(label,"__IF%x_%x",syntax.curly[pos].index,syntax.curly[pos].count);
				add_scriptl(add_str("jump_zero"));
				add_scriptc(C_ARG);
				p=parse_expr(p);
				p=skip_space(p);
				add_scriptl(add_str(label));
				add_scriptc(C_FUNC);
				*flag = 0;
				return p;
			} else {
				// else
				if(!syntax.curly[pos].flag) {
					syntax.curly[pos].flag = 1;
					*flag = 0;
					return p;
				}
			}
		}
		// Close if
		syntax.curly_count--;
		// Put the label of the final location
		sprintf(label,"__IF%x_FIN",syntax.curly[pos].index);
		l=add_str(label);
		set_label(l,script_pos,p);
		if(syntax.curly[pos].flag == 1) {
			// Because the position of the pointer is the same if not else for this
			return bp;
		}
		return p;
	} else if(syntax.curly[pos].type == TYPE_DO) {
		int l;
		char label[256];
		const char *p2;

		if(syntax.curly[pos].flag) {
			// (Come here continue) to form the label here
			sprintf(label,"__DO%x_NXT",syntax.curly[pos].index);
			l=add_str(label);
			set_label(l,script_pos,p);
		}

		// Skip to the end point if the condition is false
		p = skip_space(p);
		p2 = skip_word(p);
		if(p2 - p != 5 || strncasecmp(p,"while",5))
			disp_error_message("parse_syntax: need 'while'",p);

		p = skip_space(p2);
		if(*p != '(') {
			disp_error_message("need '('",p);
		}

		// do-block end is a new line
		parse_nextline(false, p);

		sprintf(label,"__DO%x_FIN",syntax.curly[pos].index);
		add_scriptl(add_str("jump_zero"));
		add_scriptc(C_ARG);
		p=parse_expr(p);
		p=skip_space(p);
		add_scriptl(add_str(label));
		add_scriptc(C_FUNC);

		// Skip to the starting point
		sprintf(label,"goto __DO%x_BGN;",syntax.curly[pos].index);
		syntax.curly[syntax.curly_count++].type = TYPE_NULL;
		parse_line(label);
		syntax.curly_count--;

		// Form label of the end point conditions
		sprintf(label,"__DO%x_FIN",syntax.curly[pos].index);
		l=add_str(label);
		set_label(l,script_pos,p);
		p = skip_space(p);
		if(*p != ';') {
			disp_error_message("parse_syntax: need ';'",p);
			return p+1;
		}
		p++;
		syntax.curly_count--;
		return p;
	} else if(syntax.curly[pos].type == TYPE_FOR) {
		// for-block end is a new line
		parse_nextline(false, p);

		// Skip to the next loop
		sprintf(label,"goto __FR%x_NXT;",syntax.curly[pos].index);
		syntax.curly[syntax.curly_count++].type = TYPE_NULL;
		parse_line(label);
		syntax.curly_count--;

		// End for labeling
		sprintf(label,"__FR%x_FIN",syntax.curly[pos].index);
		l=add_str(label);
		set_label(l,script_pos,p);
		syntax.curly_count--;
		return p;
	} else if(syntax.curly[pos].type == TYPE_WHILE) {
		// while-block end is a new line
		parse_nextline(false, p);

		// Skip to the decision while
		sprintf(label,"goto __WL%x_NXT;",syntax.curly[pos].index);
		syntax.curly[syntax.curly_count++].type = TYPE_NULL;
		parse_line(label);
		syntax.curly_count--;

		// End while labeling
		sprintf(label,"__WL%x_FIN",syntax.curly[pos].index);
		l=add_str(label);
		set_label(l,script_pos,p);
		syntax.curly_count--;
		return p;
	} else if(syntax.curly[syntax.curly_count-1].type == TYPE_USERFUNC) {
		int pos = syntax.curly_count-1;
		char label[256];
		int l;
		// Back
		sprintf(label,"return;");
		syntax.curly[syntax.curly_count++].type = TYPE_NULL;
		parse_line(label);
		syntax.curly_count--;

		// Put the label of the location
		sprintf(label,"__FN%x_FIN",syntax.curly[pos].index);
		l=add_str(label);
		set_label(l,script_pos,p);
		syntax.curly_count--;
		return p;
	} else {
		*flag = 0;
		return p;
	}
}

/// Retrieves the value of a constant.
bool script_get_constant(const char* name, int* value)
{
	int n = search_str(name);

	if( n == -1 || str_data[n].type != C_INT )
	{// not found or not a constant
		return false;
	}
	value[0] = str_data[n].val;

	return true;
}

/// Creates new constant or parameter with given value.
void script_set_constant(const char* name, int value, bool isparameter)
{
	int n = add_str(name);

	if( str_data[n].type == C_NOP )
	{// new
		str_data[n].type = isparameter ? C_PARAM : C_INT;
		str_data[n].val  = value;
	}
	else if( str_data[n].type == C_PARAM || str_data[n].type == C_INT )
	{// existing parameter or constant
		ShowError("script_set_constant: Attempted to overwrite existing %s '%s' (old value=%d, new value=%d).\n", ( str_data[n].type == C_PARAM ) ? "parameter" : "constant", name, str_data[n].val, value);
	}
	else
	{// existing name
		ShowError("script_set_constant: Invalid name for %s '%s' (already defined as %s).\n", isparameter ? "parameter" : "constant", name, script_op2name(str_data[n].type));
	}
}

/*==========================================
 * Reading constant databases
 * const.txt
 *------------------------------------------*/
static void read_constdb(void)
{
	FILE *fp;
	char line[1024],name[1024],val[1024];
	int type;

	sprintf(line, "%s/const.txt", db_path);
	fp=fopen(line, "r");
	if(fp==NULL){
		ShowError("can't read %s\n", line);
		return ;
	}
	while(fgets(line, sizeof(line), fp))
	{
		if(line[0]=='/' && line[1]=='/')
			continue;
		type=0;
		if(sscanf(line,"%[A-Za-z0-9_],%[-0-9xXA-Fa-f],%d",name,val,&type)>=2 ||
		   sscanf(line,"%[A-Za-z0-9_] %[-0-9xXA-Fa-f] %d",name,val,&type)>=2){
			script_set_constant(name, (int)strtol(val, NULL, 0), (bool)type);
		}
	}
	fclose(fp);
}

/*==========================================
 * Display emplacement line of script
 *------------------------------------------*/
static const char* script_print_line(StringBuf* buf, const char* p, const char* mark, int line)
{
	int i;
	if( p == NULL || !p[0] ) return NULL;
	if( line < 0 )
		StrBuf->Printf(buf, "*% 5d : ", -line);
	else
		StrBuf->Printf(buf, " % 5d : ", line);
	for(i=0;p[i] && p[i] != '\n';i++){
		if(p + i != mark)
			StrBuf->Printf(buf, "%c", p[i]);
		else
			StrBuf->Printf(buf, "\'%c\'", p[i]);
	}
	StrBuf->AppendStr(buf, "\n");
	return p+i+(p[i] == '\n' ? 1 : 0);
}

void script_error(const char* src, const char* file, int start_line, const char* error_msg, const char* error_pos)
{
	// Find the line where the error occurred
	int j;
	int line = start_line;
	const char *p;
	const char *linestart[5] = { NULL, NULL, NULL, NULL, NULL };
	StringBuf buf;

	for(p=src;p && *p;line++){
		const char *lineend=strchr(p,'\n');
		if(lineend==NULL || error_pos<lineend){
			break;
		}
		for( j = 0; j < 4; j++ ) {
			linestart[j] = linestart[j+1];
		}
		linestart[4] = p;
		p=lineend+1;
	}

	StrBuf->Init(&buf);
	StrBuf->AppendStr(&buf, "\a\n");
	StrBuf->Printf(&buf, "script error on %s line %d\n", file, line);
	StrBuf->Printf(&buf, "    %s\n", error_msg);
	for(j = 0; j < 5; j++ ) {
		script_print_line(&buf, linestart[j], NULL, line + j - 5);
	}
	p = script_print_line(&buf, p, error_pos, -line);
	for(j = 0; j < 5; j++) {
		p = script_print_line(&buf, p, NULL, line + j + 1 );
	}
	ShowError("%s", StrBuf->Value(&buf));
	StrBuf->Destroy(&buf);
}

/*==========================================
 * Analysis of the script
 *------------------------------------------*/
struct script_code* parse_script(const char *src,const char *file,int line,int options)
{
	const char *p,*tmpp;
	int i;
	struct script_code* code = NULL;
	char end;
	bool unresolved_names = false;

	if( src == NULL )
		return NULL;// empty script

	memset(&syntax,0,sizeof(syntax));

	script_buf=(unsigned char *)aMalloc(SCRIPT_BLOCK_SIZE*sizeof(unsigned char));
	script_pos=0;
	script_size=SCRIPT_BLOCK_SIZE;
	parse_nextline(true, NULL);

	// who called parse_script is responsible for clearing the database after using it, but just in case... lets clear it here
	if( options&SCRIPT_USE_LABEL_DB )
		db_clear(scriptlabel_db);
	parse_options = options;

	if( setjmp( error_jump ) != 0 ) {
		//Restore program state when script has problems. [from jA]
		int i;
		const int size = ARRAYLENGTH(syntax.curly);
		if( error_report )
			script_error(src,file,line,error_msg,error_pos);
		aFree( error_msg );
		aFree( script_buf );
		script_pos  = 0;
		script_size = 0;
		script_buf  = NULL;
		for(i=LABEL_START;i<str_num;i++)
			if(str_data[i].type == C_NOP) str_data[i].type = C_NAME;
		for(i=0; i<size; i++)
			linkdb_final(&syntax.curly[i].case_label);
		return NULL;
	}

	parse_syntax_for_flag=0;
	p=src;
	p=skip_space(p);
	if( options&SCRIPT_IGNORE_EXTERNAL_BRACKETS )
	{// does not require brackets around the script
		if( *p == '\0' && !(options&SCRIPT_RETURN_EMPTY_SCRIPT) )
		{// empty script and can return NULL
			aFree( script_buf );
			script_pos  = 0;
			script_size = 0;
			script_buf  = NULL;
			return NULL;
		}
		end = '\0';
	}
	else
	{// requires brackets around the script
		if( *p != '{' )
			disp_error_message("not found '{'",p);
		p = skip_space(p+1);
		if( *p == '}' && !(options&SCRIPT_RETURN_EMPTY_SCRIPT) )
		{// empty script and can return NULL
			aFree( script_buf );
			script_pos  = 0;
			script_size = 0;
			script_buf  = NULL;
			return NULL;
		}
		end = '}';
	}

	// clear references of labels, variables and internal functions
	for(i=LABEL_START;i<str_num;i++){
		if(
			str_data[i].type==C_POS || str_data[i].type==C_NAME ||
			str_data[i].type==C_USERFUNC || str_data[i].type == C_USERFUNC_POS
		){
			str_data[i].type=C_NOP;
			str_data[i].backpatch=-1;
			str_data[i].label=-1;
		}
	}

	while( syntax.curly_count != 0 || *p != end )
	{
		if( *p == '\0' )
			disp_error_message("unexpected end of script",p);
		// Special handling only label
		tmpp=skip_space(skip_word(p));
		if(*tmpp==':' && !(!strncasecmp(p,"default:",8) && p + 7 == tmpp)){
			i=add_word(p);
			set_label(i,script_pos,p);
			if( parse_options&SCRIPT_USE_LABEL_DB )
				strdb_iput(scriptlabel_db, get_str(i), script_pos);
			p=tmpp+1;
			p=skip_space(p);
			continue;
		}

		// All other lumped
		p=parse_line(p);
		p=skip_space(p);

		parse_nextline(false, p);
	}

	add_scriptc(C_NOP);

	// trim code to size
	script_size = script_pos;
	RECREATE(script_buf,unsigned char,script_pos);

	// default unknown references to variables
	for(i=LABEL_START;i<str_num;i++){
		if(str_data[i].type==C_NOP){
			int j,next;
			str_data[i].type=C_NAME;
			str_data[i].label=i;
			for(j=str_data[i].backpatch;j>=0 && j!=0x00ffffff;){
				next=GETVALUE(script_buf,j);
				SETVALUE(script_buf,j,i);
				j=next;
			}
		}
		else if( str_data[i].type == C_USERFUNC )
		{// 'function name;' without follow-up code
			ShowError("parse_script: function '%s' declared but not defined.\n", str_buf+str_data[i].str);
			unresolved_names = true;
		}
	}

	if( unresolved_names )
	{
		disp_error_message("parse_script: unresolved function references", p);
	}

#ifdef DEBUG_DISP
	for(i=0;i<script_pos;i++){
		if((i&15)==0) ShowMessage("%04x : ",i);
		ShowMessage("%02x ",script_buf[i]);
		if((i&15)==15) ShowMessage("\n");
	}
	ShowMessage("\n");
#endif
#ifdef DEBUG_DISASM
	{
		int i = 0,j;
		while(i < script_pos) {
			c_op op = get_com(script_buf,&i);

			ShowMessage("%06x %s", i, script_op2name(op));
			j = i;
			switch(op) {
			case C_INT:
				ShowMessage(" %d", get_num(script_buf,&i));
				break;
			case C_POS:
				ShowMessage(" 0x%06x", *(int*)(script_buf+i)&0xffffff);
				i += 3;
				break;
			case C_NAME:
				j = (*(int*)(script_buf+i)&0xffffff);
				ShowMessage(" %s", ( j == 0xffffff ) ? "?? unknown ??" : get_str(j));
				i += 3;
				break;
			case C_STR:
				j = strlen(script_buf + i);
				ShowMessage(" %s", script_buf + i);
				i += j+1;
				break;
			}
			ShowMessage(CL_CLL"\n");
		}
	}
#endif

	CREATE(code,struct script_code,1);
	code->script_buf  = script_buf;
	code->script_size = script_size;
	code->script_vars = idb_alloc(DB_OPT_RELEASE_DATA);
	return code;
}

/// Returns the player attached to this script, identified by the rid.
/// If there is no player attached, the script is terminated.
TBL_PC *script_rid2sd(struct script_state *st)
{
	TBL_PC *sd=map_id2sd(st->rid);
	if(!sd){
		ShowError("script_rid2sd: fatal error ! player not attached!\n");
		script_reportfunc(st);
		script_reportsrc(st);
		st->state = END;
	}
	return sd;
}

/// Dereferences a variable/constant, replacing it with a copy of the value.
///
/// @param st Script state
/// @param data Variable/constant
void get_val(struct script_state* st, struct script_data* data)
{
	const char* name;
	char prefix;
	char postfix;
	TBL_PC* sd = NULL;

	if( !data_isreference(data) )
		return;// not a variable/constant

	name = reference_getname(data);
	prefix = name[0];
	postfix = name[strlen(name) - 1];

	//##TODO use reference_tovariable(data) when it's confirmed that it works [FlavioJS]
	if( !reference_toconstant(data) && not_server_variable(prefix) )
	{
		sd = script_rid2sd(st);
		if( sd == NULL )
		{// needs player attached
			if( postfix == '$' )
			{// string variable
				ShowWarning("script:get_val: cannot access player variable '%s', defaulting to \"\"\n", name);
				data->type = C_CONSTSTR;
				data->u.str = "";
			}
			else
			{// integer variable
				ShowWarning("script:get_val: cannot access player variable '%s', defaulting to 0\n", name);
				data->type = C_INT;
				data->u.num = 0;
			}
			return;
		}
	}

	if( postfix == '$' )
	{// string variable

		switch( prefix )
		{
		case '@':
			data->u.str = pc_readregstr(sd, data->u.num);
			break;
		case '$':
			data->u.str = mapreg_readregstr(data->u.num);
			break;
		case '#':
			if( name[1] == '#' )
				data->u.str = pc_readaccountreg2str(sd, name);// global
			else
				data->u.str = pc_readaccountregstr(sd, name);// local
			break;
		case '.':
			{
				struct DBMap* n =
					data->ref      ? *data->ref:
					name[1] == '@' ?  st->stack->var_function:// instance/scope variable
					                  st->script->script_vars;// npc variable
				if( n )
					data->u.str = (char*)idb_get(n,reference_getuid(data));
				else
					data->u.str = NULL;
			}
			break;
		case '\'':
				if ( st->instance_id >= 0 ) {
					data->u.str = (char*)idb_get(instances[st->instance_id].vars,reference_getuid(data));
				} else {
					ShowWarning("script:get_val: cannot access instance variable '%s', defaulting to \"\"\n", name);
					data->u.str = NULL;
				}
			break;
		default:
			data->u.str = pc_readglobalreg_str(sd, name);
			break;
		}

		if( data->u.str == NULL || data->u.str[0] == '\0' )
		{// empty string
			data->type = C_CONSTSTR;
			data->u.str = "";
		}
		else
		{// duplicate string
			data->type = C_STR;
			data->u.str = aStrdup(data->u.str);
		}

	}
	else
	{// integer variable

		data->type = C_INT;

		if( reference_toconstant(data) )
		{
			data->u.num = reference_getconstant(data);
		}
		else if( reference_toparam(data) )
		{
			data->u.num = pc_readparam(sd, reference_getparamtype(data));
		}
		else
		switch( prefix )
		{
		case '@':
			data->u.num = pc_readreg(sd, data->u.num);
			break;
		case '$':
			data->u.num = mapreg_readreg(data->u.num);
			break;
		case '#':
			if( name[1] == '#' )
				data->u.num = pc_readaccountreg2(sd, name);// global
			else
				data->u.num = pc_readaccountreg(sd, name);// local
			break;
		case '.':
			{
				struct DBMap* n =
					data->ref      ? *data->ref:
					name[1] == '@' ?  st->stack->var_function:// instance/scope variable
					                  st->script->script_vars;// npc variable
				if( n )
					data->u.num = (int)idb_iget(n,reference_getuid(data));
				else
					data->u.num = 0;
			}
			break;
		case '\'':
				if( st->instance_id >= 0 )
					data->u.num = (int)idb_iget(instances[st->instance_id].vars,reference_getuid(data));
				else {
					ShowWarning("script:get_val: cannot access instance variable '%s', defaulting to 0\n", name);
					data->u.num = 0;
				}
			break;
		default:
			data->u.num = pc_readglobalreg(sd, name);
			break;
		}

	}

	return;
}

struct script_data* push_val2(struct script_stack* stack, enum c_op type, int val, struct DBMap** ref);

/// Retrieves the value of a reference identified by uid (variable, constant, param)
/// The value is left in the top of the stack and needs to be removed manually.
void* get_val2(struct script_state* st, int uid, struct DBMap** ref)
{
	struct script_data* data;
	push_val2(st->stack, C_NAME, uid, ref);
	data = script_getdatatop(st, -1);
	get_val(st, data);
	return (data->type == C_INT ? (void*)__64BPTRSIZE(data->u.num) : (void*)__64BPTRSIZE(data->u.str));
}

/*==========================================
 * Stores the value of a script variable
 * Return value is 0 on fail, 1 on success.
 *------------------------------------------*/
static int set_reg(struct script_state* st, TBL_PC* sd, int num, const char* name, const void* value, struct DBMap** ref)
{
	char prefix = name[0];

	if( is_string_variable(name) )
	{// string variable
		const char* str = (const char*)value;
		switch (prefix) {
		case '@':
			return pc_setregstr(sd, num, str);
		case '$':
			return mapreg_setregstr(num, str);
		case '#':
			return (name[1] == '#') ?
				pc_setaccountreg2str(sd, name, str) :
				pc_setaccountregstr(sd, name, str);
		case '.':
			{
				struct DBMap* n;
				n = (ref) ? *ref : (name[1] == '@') ? st->stack->var_function : st->script->script_vars;
				if( n ) {
					idb_remove(n, num);
					if (str[0]) idb_put(n, num, aStrdup(str));
				}
			}
			return 1;
		case '\'':
			if( st->instance_id >= 0 ) {
				idb_remove(instances[st->instance_id].vars, num);
				if( str[0] ) idb_put(instances[st->instance_id].vars, num, aStrdup(str));
			}
			return 1;
		default:
			return pc_setglobalreg_str(sd, name, str);
		}
	}
	else
	{// integer variable
		int val = (int)__64BPTRSIZE(value);
		if(str_data[num&0x00ffffff].type == C_PARAM)
		{
			if( pc_setparam(sd, str_data[num&0x00ffffff].val, val) == 0 )
			{
				if( st != NULL )
				{
					ShowError("script:set_reg: failed to set param '%s' to %d.\n", name, val);
					script_reportsrc(st);
					st->state = END;
				}
				return 0;
			}
			return 1;
		}

		switch (prefix) {
		case '@':
			return pc_setreg(sd, num, val);
		case '$':
			return mapreg_setreg(num, val);
		case '#':
			return (name[1] == '#') ?
				pc_setaccountreg2(sd, name, val) :
				pc_setaccountreg(sd, name, val);
		case '.':
			{
				struct DBMap* n;
				n = (ref) ? *ref : (name[1] == '@') ? st->stack->var_function : st->script->script_vars;
				if( n ) {
					idb_remove(n, num);
					if( val != 0 )
						idb_iput(n, num, val);
				}
			}
			return 1;
		case '\'':
			if( st->instance_id >= 0 ) {
				idb_remove(instances[st->instance_id].vars, num);
				if( val != 0 )
					idb_iput(instances[st->instance_id].vars, num, val);
			}
			return 1;
		default:
			return pc_setglobalreg(sd, name, val);
		}
	}
}

int set_var(TBL_PC* sd, char* name, void* val)
{
    return set_reg(NULL, sd, reference_uid(add_str(name),0), name, val, NULL);
}

void setd_sub(struct script_state *st, TBL_PC *sd, const char *varname, int elem, void *value, struct DBMap **ref)
{
	set_reg(st, sd, reference_uid(add_str(varname),elem), varname, value, ref);
}

/// Converts the data to a string
const char* conv_str(struct script_state* st, struct script_data* data)
{
	char* p;

	get_val(st, data);
	if( data_isstring(data) )
	{// nothing to convert
	}
	else if( data_isint(data) )
	{// int -> string
		CREATE(p, char, ITEM_NAME_LENGTH);
		snprintf(p, ITEM_NAME_LENGTH, "%d", data->u.num);
		p[ITEM_NAME_LENGTH-1] = '\0';
		data->type = C_STR;
		data->u.str = p;
	}
	else if( data_isreference(data) )
	{// reference -> string
		//##TODO when does this happen (check get_val) [FlavioJS]
		data->type = C_CONSTSTR;
		data->u.str = reference_getname(data);
	}
	else
	{// unsupported data type
		ShowError("script:conv_str: cannot convert to string, defaulting to \"\"\n");
		script_reportdata(data);
		script_reportsrc(st);
		data->type = C_CONSTSTR;
		data->u.str = "";
	}
	return data->u.str;
}

/// Converts the data to an int
int conv_num(struct script_state* st, struct script_data* data) {
	char* p;
	long num;

	get_val(st, data);
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
			script_reportdata(data);
			script_reportsrc(st);
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
		script_reportdata(data);
		script_reportsrc(st);
		data->type = C_INT;
		data->u.num = 0;
	}
#endif
	return data->u.num;
}

//
// Stack operations
//

/// Increases the size of the stack
void stack_expand(struct script_stack* stack)
{
	stack->sp_max += 64;
	stack->stack_data = (struct script_data*)aRealloc(stack->stack_data,
			stack->sp_max * sizeof(stack->stack_data[0]) );
	memset(stack->stack_data + (stack->sp_max - 64), 0,
			64 * sizeof(stack->stack_data[0]) );
}

/// Pushes a value into the stack
#define push_val(stack,type,val) push_val2(stack, type, val, NULL)

/// Pushes a value into the stack (with reference)
struct script_data* push_val2(struct script_stack* stack, enum c_op type, int val, struct DBMap** ref)
{
	if( stack->sp >= stack->sp_max )
		stack_expand(stack);
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
		stack_expand(stack);
	stack->stack_data[stack->sp].type  = type;
	stack->stack_data[stack->sp].u.str = str;
	stack->stack_data[stack->sp].ref   = NULL;
	stack->sp++;
	return &stack->stack_data[stack->sp-1];
}

/// Pushes a retinfo into the stack
struct script_data* push_retinfo(struct script_stack* stack, struct script_retinfo* ri, DBMap **ref)
{
	if( stack->sp >= stack->sp_max )
		stack_expand(stack);
	stack->stack_data[stack->sp].type = C_RETINFO;
	stack->stack_data[stack->sp].u.ri = ri;
	stack->stack_data[stack->sp].ref  = ref;
	stack->sp++;
	return &stack->stack_data[stack->sp-1];
}

/// Pushes a copy of the target position into the stack
struct script_data* push_copy(struct script_stack* stack, int pos)
{
	switch( stack->stack_data[pos].type )
	{
	case C_CONSTSTR:
		return push_str(stack, C_CONSTSTR, stack->stack_data[pos].u.str);
		break;
	case C_STR:
		return push_str(stack, C_STR, aStrdup(stack->stack_data[pos].u.str));
		break;
	case C_RETINFO:
		ShowFatalError("script:push_copy: can't create copies of C_RETINFO. Exiting...\n");
		exit(1);
		break;
	default:
		return push_val2(
			stack,stack->stack_data[pos].type,
			stack->stack_data[pos].u.num,
			stack->stack_data[pos].ref
		);
		break;
	}
}

/// Removes the values in indexes [start,end[ from the stack.
/// Adjusts all stack pointers.
void pop_stack(struct script_state* st, int start, int end)
{
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
			if( ri->var_function )
				script_free_vars(ri->var_function);
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
	     if( st->start > end )   st->start -= end - start;
	else if( st->start > start ) st->start = start;
	     if( st->end > end )   st->end -= end - start;
	else if( st->end > start ) st->end = start;
	     if( stack->defsp > end )   stack->defsp -= end - start;
	else if( stack->defsp > start ) stack->defsp = start;
	stack->sp -= end - start;
}

///
///
///

/*==========================================
 * Release script dependent variable, dependent variable of function
 *------------------------------------------*/
void script_free_vars(struct DBMap* storage)
{
	if( storage )
	{// destroy the storage construct containing the variables
		db_destroy(storage);
	}
}

void script_free_code(struct script_code* code)
{
	script_free_vars( code->script_vars );
	aFree( code->script_buf );
	aFree( code );
}

/// Creates a new script state.
///
/// @param script Script code
/// @param pos Position in the code
/// @param rid Who is running the script (attached player)
/// @param oid Where the code is being run (npc 'object')
/// @return Script state
struct script_state* script_alloc_state(struct script_code* script, int pos, int rid, int oid)
{
	struct script_state* st;
	CREATE(st, struct script_state, 1);
	st->stack = (struct script_stack*)aMalloc(sizeof(struct script_stack));
	st->stack->sp = 0;
	st->stack->sp_max = 64;
	CREATE(st->stack->stack_data, struct script_data, st->stack->sp_max);
	st->stack->defsp = st->stack->sp;
	st->stack->var_function = idb_alloc(DB_OPT_RELEASE_DATA);
	st->state = RUN;
	st->script = script;
	//st->scriptroot = script;
	st->pos = pos;
	st->rid = rid;
	st->oid = oid;
	st->sleep.timer = INVALID_TIMER;
	st->npc_item_flag = battle_config.item_enabled_npc;
	return st;
}

/// Frees a script state.
///
/// @param st Script state
void script_free_state(struct script_state* st)
{
	if(st->bk_st)
	{// backup was not restored
		ShowDebug("script_free_state: Previous script state lost (rid=%d, oid=%d, state=%d, bk_npcid=%d).\n", st->bk_st->rid, st->bk_st->oid, st->bk_st->state, st->bk_npcid);
	}
	if( st->sleep.timer != INVALID_TIMER )
		delete_timer(st->sleep.timer, run_script_timer);
	script_free_vars(st->stack->var_function);
	pop_stack(st, 0, st->stack->sp);
	aFree(st->stack->stack_data);
	aFree(st->stack);
	st->stack = NULL;
	st->pos = -1;
	aFree(st);
}

//
// Main execution unit
//
/*==========================================
 * Read command
 *------------------------------------------*/
c_op get_com(unsigned char *script,int *pos)
{
	int i = 0, j = 0;

	if(script[*pos]>=0x80){
		return C_INT;
	}
	while(script[*pos]>=0x40){
		i=script[(*pos)++]<<j;
		j+=6;
	}
	return (c_op)(i+(script[(*pos)++]<<j));
}

/*==========================================
 *  Income figures
 *------------------------------------------*/
int get_num(unsigned char *script,int *pos)
{
	int i,j;
	i=0; j=0;
	while(script[*pos]>=0xc0){
		i+=(script[(*pos)++]&0x7f)<<j;
		j+=6;
	}
	return i+((script[(*pos)++]&0x7f)<<j);
}

/*==========================================
 * Remove the value from the stack
 *------------------------------------------*/
int pop_val(struct script_state* st)
{
	if(st->stack->sp<=0)
		return 0;
	st->stack->sp--;
	get_val(st,&(st->stack->stack_data[st->stack->sp]));
	if(st->stack->stack_data[st->stack->sp].type==C_INT)
		return st->stack->stack_data[st->stack->sp].u.num;
	return 0;
}

/// Ternary operators
/// test ? if_true : if_false
void op_3(struct script_state* st, int op)
{
	struct script_data* data;
	int flag = 0;

	data = script_getdatatop(st, -3);
	get_val(st, data);

	if( data_isstring(data) )
		flag = data->u.str[0];// "" -> false
	else if( data_isint(data) )
		flag = data->u.num;// 0 -> false
	else
	{
		ShowError("script:op_3: invalid data for the ternary operator test\n");
		script_reportdata(data);
		script_reportsrc(st);
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
/// s1 ADD s2 -> s
void op_2str(struct script_state* st, int op, const char* s1, const char* s2)
{
	int a = 0;

	switch(op){
	case C_EQ: a = (strcmp(s1,s2) == 0); break;
	case C_NE: a = (strcmp(s1,s2) != 0); break;
	case C_GT: a = (strcmp(s1,s2) >  0); break;
	case C_GE: a = (strcmp(s1,s2) >= 0); break;
	case C_LT: a = (strcmp(s1,s2) <  0); break;
	case C_LE: a = (strcmp(s1,s2) <= 0); break;
	case C_ADD:
		{
			char* buf = (char *)aMalloc((strlen(s1)+strlen(s2)+1)*sizeof(char));
			strcpy(buf, s1);
			strcat(buf, s2);
			script_pushstr(st, buf);
			return;
		}
	default:
		ShowError("script:op2_str: unexpected string operator %s\n", script_op2name(op));
		script_reportsrc(st);
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
	double ret_double;

	switch( op )
	{
	case C_AND:  ret = i1 & i2;		break;
	case C_OR:   ret = i1 | i2;		break;
	case C_XOR:  ret = i1 ^ i2;		break;
	case C_LAND: ret = (i1 && i2);	break;
	case C_LOR:  ret = (i1 || i2);	break;
	case C_EQ:   ret = (i1 == i2);	break;
	case C_NE:   ret = (i1 != i2);	break;
	case C_GT:   ret = (i1 >  i2);	break;
	case C_GE:   ret = (i1 >= i2);	break;
	case C_LT:   ret = (i1 <  i2);	break;
	case C_LE:   ret = (i1 <= i2);	break;
	case C_R_SHIFT: ret = i1>>i2;	break;
	case C_L_SHIFT: ret = i1<<i2;	break;
	case C_DIV:
	case C_MOD:
		if( i2 == 0 )
		{
			ShowError("script:op_2num: division by zero detected op=%s i1=%d i2=%d\n", script_op2name(op), i1, i2);
			script_reportsrc(st);
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
		switch( op )
		{// operators that can overflow/underflow
		case C_ADD: ret = i1 + i2; ret_double = (double)i1 + (double)i2; break;
		case C_SUB: ret = i1 - i2; ret_double = (double)i1 - (double)i2; break;
		case C_MUL: ret = i1 * i2; ret_double = (double)i1 * (double)i2; break;
		default:
			ShowError("script:op_2num: unexpected number operator %s i1=%d i2=%d\n", script_op2name(op), i1, i2);
			script_reportsrc(st);
			script_pushnil(st);
			return;
		}
		if( ret_double < (double)INT_MIN )
		{
			ShowWarning("script:op_2num: underflow detected op=%s i1=%d i2=%d\n", script_op2name(op), i1, i2);
			script_reportsrc(st);
			ret = INT_MIN;
		}
		else if( ret_double > (double)INT_MAX )
		{
			ShowWarning("script:op_2num: overflow detected op=%s i1=%d i2=%d\n", script_op2name(op), i1, i2);
			script_reportsrc(st);
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

	get_val(st, left);
	get_val(st, right);

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
		op_2str(st, op, left->u.str, right->u.str);
		script_removetop(st, leftref.type == C_NOP ? -3 : -2, -1);// pop the two values before the top one

		if (leftref.type != C_NOP)
		{
			aFree(left->u.str);
			*left = leftref;
		}
	}
	else if( data_isint(left) && data_isint(right) )
	{// ii => op_2num
		int i1 = left->u.num;
		int i2 = right->u.num;

		script_removetop(st, leftref.type == C_NOP ? -2 : -1, 0);
		op_2num(st, op, i1, i2);

		if (leftref.type != C_NOP)
			*left = leftref;
	}
	else
	{// invalid argument
		ShowError("script:op_2: invalid data for operator %s\n", script_op2name(op));
		script_reportdata(left);
		script_reportdata(right);
		script_reportsrc(st);
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
	get_val(st, data);

	if( !data_isint(data) )
	{// not a number
		ShowError("script:op_1: argument is not a number (op=%s)\n", script_op2name(op));
		script_reportdata(data);
		script_reportsrc(st);
		script_pushnil(st);
		st->state = END;
		return;
	}

	i1 = data->u.num;
	script_removetop(st, -1, 0);
	switch( op )
	{
	case C_NEG: i1 = -i1; break;
	case C_NOT: i1 = ~i1; break;
	case C_LNOT: i1 = !i1; break;
	default:
		ShowError("script:op_1: unexpected operator %s i1=%d\n", script_op2name(op), i1);
		script_reportsrc(st);
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
static void script_check_buildin_argtype(struct script_state* st, int func)
{
	char type;
	int idx, invalid = 0;
	char* sf = script->buildin[str_data[func].val];

	for( idx = 2; script_hasdata(st, idx); idx++ ) {
		struct script_data* data = script_getdata(st, idx);

		type = sf[idx-2];
		
		if( type == '?' || type == '*' ) {// optional argument or unknown number of optional parameters ( no types are after this )
			break;
		} else if( type == 0 ) {// more arguments than necessary ( should not happen, as it is checked before )
			ShowWarning("Found more arguments than necessary. unexpected arg type %s\n",script_op2name(data->type));
			invalid++;
			break;
		} else {
			const char* name = NULL;

			if( data_isreference(data) )
			{// get name for variables to determine the type they refer to
				name = reference_getname(data);
			}

			switch( type ) {
				case 'v':
					if( !data_isstring(data) && !data_isint(data) && !data_isreference(data) )
					{// variant
						ShowWarning("Unexpected type for argument %d. Expected string, number or variable.\n", idx-1);
						script_reportdata(data);
						invalid++;
					}
					break;
				case 's':
					if( !data_isstring(data) && !( data_isreference(data) && is_string_variable(name) ) )
					{// string
						ShowWarning("Unexpected type for argument %d. Expected string.\n", idx-1);
						script_reportdata(data);
						invalid++;
					}
					break;
				case 'i':
					if( !data_isint(data) && !( data_isreference(data) && ( reference_toparam(data) || reference_toconstant(data) || !is_string_variable(name) ) ) )
					{// int ( params and constants are always int )
						ShowWarning("Unexpected type for argument %d. Expected number.\n", idx-1);
						script_reportdata(data);
						invalid++;
					}
					break;
				case 'r':
					if( !data_isreference(data) )
					{// variables
						ShowWarning("Unexpected type for argument %d. Expected variable, got %s.\n", idx-1,script_op2name(data->type));
						script_reportdata(data);
						invalid++;
					}
					break;
				case 'l':
					if( !data_islabel(data) && !data_isfunclabel(data) )
					{// label
						ShowWarning("Unexpected type for argument %d. Expected label, got %s\n", idx-1,script_op2name(data->type));
						script_reportdata(data);
						invalid++;
					}
					break;
			}
		}
	}

	if(invalid) {
		ShowDebug("Function: %s\n", get_str(func));
		script_reportsrc(st);
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
		script_reportsrc(st);
		return 1;
	}
	start_sp = i-1;// C_NAME of the command
	st->start = start_sp;
	st->end = end_sp;

	data = &st->stack->stack_data[st->start];
	if( data->type == C_NAME && str_data[data->u.num].type == C_FUNC )
		func = data->u.num;
	else
	{
		ShowError("script:run_func: not a buildin command.\n");
		script_reportdata(data);
		script_reportsrc(st);
		st->state = END;
		return 1;
	}

	if( script_config.warn_func_mismatch_argtypes )
	{
		script_check_buildin_argtype(st, func);
	}

	if(str_data[func].func){
		if (!(str_data[func].func(st))) //Report error
			script_reportsrc(st);
	} else {
		ShowError("script:run_func: '%s' (id=%d type=%s) has no C function. please report this!!!\n", get_str(func), func, script_op2name(str_data[func].type));
		script_reportsrc(st);
		st->state = END;
	}

	// Stack's datum are used when re-running functions [Eoe]
	if( st->state == RERUNLINE )
		return 0;

	pop_stack(st, st->start, st->end);
	if( st->state == RETFUNC )
	{// return from a user-defined function
		struct script_retinfo* ri;
		int olddefsp = st->stack->defsp;
		int nargs;

		pop_stack(st, st->stack->defsp, st->start);// pop distractions from the stack
		if( st->stack->defsp < 1 || st->stack->stack_data[st->stack->defsp-1].type != C_RETINFO )
		{
			ShowWarning("script:run_func: return without callfunc or callsub!\n");
			script_reportsrc(st);
			st->state = END;
			return 1;
		}
		script_free_vars( st->stack->var_function );

		ri = st->stack->stack_data[st->stack->defsp-1].u.ri;
		nargs = ri->nargs;
		st->pos = ri->pos;
		st->script = ri->script;
		st->stack->var_function = ri->var_function;
		st->stack->defsp = ri->defsp;
		memset(ri, 0, sizeof(struct script_retinfo));

		pop_stack(st, olddefsp-nargs-1, olddefsp);// pop arguments and retinfo

		st->state = GOTO;
	}

	return 0;
}

/*==========================================
 * script execution
 *------------------------------------------*/
void run_script(struct script_code *rootscript,int pos,int rid,int oid)
{
	struct script_state *st;

	if( rootscript == NULL || pos < 0 )
		return;

	// TODO In jAthena, this function can take over the pending script in the player. [FlavioJS]
	//      It is unclear how that can be triggered, so it needs the be traced/checked in more detail.
	// NOTE At the time of this change, this function wasn't capable of taking over the script state because st->scriptroot was never set.
	st = script_alloc_state(rootscript, pos, rid, oid);
	run_script_main(st);
}

void script_stop_sleeptimers(int id)
{
	struct script_state* st;
	for(;;)
	{
		st = (struct script_state*)linkdb_erase(&sleep_db,(void*)__64BPTRSIZE(id));
		if( st == NULL )
			break; // no more sleep timers
		script_free_state(st);
	}
}

/*==========================================
 * Delete the specified node from sleep_db
 *------------------------------------------*/
struct linkdb_node* script_erase_sleepdb(struct linkdb_node *n)
{
	struct linkdb_node *retnode;

	if( n == NULL)
		return NULL;
	if( n->prev == NULL )
		sleep_db = n->next;
	else
		n->prev->next = n->next;
	if( n->next )
		n->next->prev = n->prev;
	retnode = n->next;
	aFree( n );
	return retnode;		// The following; return retnode
}

/*==========================================
 * Timer function for sleep
 *------------------------------------------*/
int run_script_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct script_state *st     = (struct script_state *)data;
	struct linkdb_node *node    = (struct linkdb_node *)sleep_db;
	TBL_PC *sd = map_id2sd(st->rid);

	if((sd && sd->status.char_id != id) || (st->rid && !sd))
	{	//Character mismatch. Cancel execution.
		st->rid = 0;
		st->state = END;
	}
	while( node && st->sleep.timer != INVALID_TIMER ) {
		if( (int)__64BPTRSIZE(node->key) == st->oid && ((struct script_state *)node->data)->sleep.timer == st->sleep.timer ) {
			script_erase_sleepdb(node);
			st->sleep.timer = INVALID_TIMER;
			break;
		}
		node = node->next;
	}
	if(st->state != RERUNLINE)
		st->sleep.tick = 0;
	run_script_main(st);
	return 0;
}

/// Detaches script state from possibly attached character and restores it's previous script if any.
///
/// @param st Script state to detach.
/// @param dequeue_event Whether to schedule any queued events, when there was no previous script.
static void script_detach_state(struct script_state* st, bool dequeue_event)
{
	struct map_session_data* sd;

	if(st->rid && (sd = map_id2sd(st->rid))!=NULL) {
		sd->st = st->bk_st;
		sd->npc_id = st->bk_npcid;
		if(st->bk_st) {
			//Remove tag for removal.
			st->bk_st = NULL;
			st->bk_npcid = 0;
		} else if(dequeue_event) {
			/**
			 * For the Secure NPC Timeout option (check config/Secure.h) [RR]
			 **/
#ifdef SECURE_NPCTIMEOUT
			/**
			 * We're done with this NPC session, so we cancel the timer (if existent) and move on
			 **/
			if( sd->npc_idle_timer != INVALID_TIMER ) {
				delete_timer(sd->npc_idle_timer,npc_rr_secure_timeout_timer);
				sd->npc_idle_timer = INVALID_TIMER;
			}
#endif
			npc_event_dequeue(sd);
		}
	}
	else if(st->bk_st)
	{// rid was set to 0, before detaching the script state
		ShowError("script_detach_state: Found previous script state without attached player (rid=%d, oid=%d, state=%d, bk_npcid=%d)\n", st->bk_st->rid, st->bk_st->oid, st->bk_st->state, st->bk_npcid);
		script_reportsrc(st->bk_st);

		script_free_state(st->bk_st);
		st->bk_st = NULL;
	}
}

/// Attaches script state to possibly attached character and backups it's previous script, if any.
///
/// @param st Script state to attach.
static void script_attach_state(struct script_state* st)
{
	struct map_session_data* sd;

	if(st->rid && (sd = map_id2sd(st->rid))!=NULL)
	{
		if(st!=sd->st)
		{
			if(st->bk_st)
			{// there is already a backup
				ShowDebug("script_free_state: Previous script state lost (rid=%d, oid=%d, state=%d, bk_npcid=%d).\n", st->bk_st->rid, st->bk_st->oid, st->bk_st->state, st->bk_npcid);
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
			sd->npc_idle_timer = add_timer(gettick() + (SECURE_NPCTIMEOUT_INTERVAL*1000),npc_rr_secure_timeout_timer,sd->bl.id,0);
		sd->npc_idle_tick = gettick();
#endif
	}
}

/*==========================================
 * The main part of the script execution
 *------------------------------------------*/
void run_script_main(struct script_state *st)
{
	int cmdcount = script_config.check_cmdcount;
	int gotocount = script_config.check_gotocount;
	TBL_PC *sd;
	struct script_stack *stack=st->stack;
	struct npc_data *nd;

	script_attach_state(st);

	nd = map_id2nd(st->oid);
	if( nd && nd->bl.m >= 0 )
		st->instance_id = map[nd->bl.m].instance_id;
	else
		st->instance_id = -1;

	if(st->state == RERUNLINE) {
		run_func(st);
		if(st->state == GOTO)
			st->state = RUN;
	} else if(st->state != END)
		st->state = RUN;

	while(st->state == RUN)
	{
		enum c_op c = get_com(st->script->script_buf,&st->pos);
		switch(c){
		case C_EOL:
			if( stack->defsp > stack->sp )
				ShowError("script:run_script_main: unexpected stack position (defsp=%d sp=%d). please report this!!!\n", stack->defsp, stack->sp);
			else
				pop_stack(st, stack->defsp, stack->sp);// pop unused stack data. (unused return value)
			break;
		case C_INT:
			push_val(stack,C_INT,get_num(st->script->script_buf,&st->pos));
			break;
		case C_POS:
		case C_NAME:
			push_val(stack,c,GETVALUE(st->script->script_buf,st->pos));
			st->pos+=3;
			break;
		case C_ARG:
			push_val(stack,c,0);
			break;
		case C_STR:
			push_str(stack,C_CONSTSTR,(char*)(st->script->script_buf+st->pos));
			while(st->script->script_buf[st->pos++]);
			break;
		case C_FUNC:
			run_func(st);
			if(st->state==GOTO){
				st->state = RUN;
				if( !st->freeloop && gotocount>0 && (--gotocount)<=0 ){
					ShowError("run_script: infinity loop !\n");
					script_reportsrc(st);
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
			op_1(st ,c);
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
			op_2(st, c);
			break;

		case C_OP3:
			op_3(st, c);
			break;

		case C_NOP:
			st->state=END;
			break;

		default:
			ShowError("unknown command : %d @ %d\n",c,st->pos);
			st->state=END;
			break;
		}
		if( !st->freeloop && cmdcount>0 && (--cmdcount)<=0 ){
			ShowError("run_script: infinity loop !\n");
			script_reportsrc(st);
			st->state=END;
		}
	}

	if(st->sleep.tick > 0) {
		//Restore previous script
		script_detach_state(st, false);
		//Delay execution
		sd = map_id2sd(st->rid); // Get sd since script might have attached someone while running. [Inkfish]
		st->sleep.charid = sd?sd->status.char_id:0;
		st->sleep.timer  = add_timer(gettick()+st->sleep.tick,
			run_script_timer, st->sleep.charid, (intptr_t)st);
		linkdb_insert(&sleep_db, (void*)__64BPTRSIZE(st->oid), st);
	}
	else if(st->state != END && st->rid){
		//Resume later (st is already attached to player).
		if(st->bk_st) {
			ShowWarning("Unable to restore stack! Double continuation!\n");
			//Report BOTH scripts to see if that can help somehow.
			ShowDebug("Previous script (lost):\n");
			script_reportsrc(st->bk_st);
			ShowDebug("Current script:\n");
			script_reportsrc(st);

			script_free_state(st->bk_st);
			st->bk_st = NULL;
		}
	} else {
		//Dispose of script.
		if ((sd = map_id2sd(st->rid))!=NULL)
		{	//Restore previous stack and save char.
			if(sd->state.using_fake_npc){
				clif->clearunit_single(sd->npc_id, CLR_OUTSIGHT, sd->fd);
				sd->state.using_fake_npc = 0;
			}
			//Restore previous script if any.
			script_detach_state(st, true);
			if (sd->state.reg_dirty&2)
				intif_saveregistry(sd,2);
			if (sd->state.reg_dirty&1)
				intif_saveregistry(sd,1);
		}
		script_free_state(st);
		st = NULL;
	}
}

int script_config_read(char *cfgName)
{
	int i;
	char line[1024],w1[1024],w2[1024];
	FILE *fp;


	fp=fopen(cfgName,"r");
	if(fp==NULL){
		ShowError("File not found: %s\n", cfgName);
		return 1;
	}
	while(fgets(line, sizeof(line), fp))
	{
		if(line[0] == '/' && line[1] == '/')
			continue;
		i=sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if(i!=2)
			continue;

		if(strcmpi(w1,"warn_func_mismatch_paramnum")==0) {
			script_config.warn_func_mismatch_paramnum = config_switch(w2);
		}
		else if(strcmpi(w1,"check_cmdcount")==0) {
			script_config.check_cmdcount = config_switch(w2);
		}
		else if(strcmpi(w1,"check_gotocount")==0) {
			script_config.check_gotocount = config_switch(w2);
		}
		else if(strcmpi(w1,"input_min_value")==0) {
			script_config.input_min_value = config_switch(w2);
		}
		else if(strcmpi(w1,"input_max_value")==0) {
			script_config.input_max_value = config_switch(w2);
		}
		else if(strcmpi(w1,"warn_func_mismatch_argtypes")==0) {
			script_config.warn_func_mismatch_argtypes = config_switch(w2);
		}
		else if(strcmpi(w1,"import")==0){
			script_config_read(w2);
		}
		else {
			ShowWarning("Unknown setting '%s' in file %s\n", w1, cfgName);
		}
	}
	fclose(fp);

	return 0;
}

/**
 * @see DBApply
 */
static int db_script_free_code_sub(DBKey key, DBData *data, va_list ap)
{
	struct script_code *code = DB->data2ptr(data);
	if (code)
		script_free_code(code);
	return 0;
}

void script_run_autobonus(const char *autobonus, int id, int pos)
{
	struct script_code *script = (struct script_code *)strdb_get(autobonus_db, autobonus);

	if( script )
	{
		current_equip_item_index = pos;
		run_script(script,0,id,0);
	}
}

void script_add_autobonus(const char *autobonus)
{
	if( strdb_get(autobonus_db, autobonus) == NULL )
	{
		struct script_code *script = parse_script(autobonus, "autobonus", 0, 0);

		if( script )
			strdb_put(autobonus_db, autobonus, script);
	}
}


/// resets a temporary character array variable to given value
void script_cleararray_pc(struct map_session_data* sd, const char* varname, void* value)
{
	int key;
	uint8 idx;

	if( not_array_variable(varname[0]) || !not_server_variable(varname[0]) )
	{
		ShowError("script_cleararray_pc: Variable '%s' has invalid scope (char_id=%d).\n", varname, sd->status.char_id);
		return;
	}

	key = add_str(varname);

	if( is_string_variable(varname) )
	{
		for( idx = 0; idx < SCRIPT_MAX_ARRAYSIZE; idx++ )
		{
			pc_setregstr(sd, reference_uid(key, idx), (const char*)value);
		}
	}
	else
	{
		for( idx = 0; idx < SCRIPT_MAX_ARRAYSIZE; idx++ )
		{
			pc_setreg(sd, reference_uid(key, idx), (int)__64BPTRSIZE(value));
		}
	}
}


/// sets a temporary character array variable element idx to given value
/// @param refcache Pointer to an int variable, which keeps a copy of the reference to varname and must be initialized to 0. Can be NULL if only one element is set.
void script_setarray_pc(struct map_session_data* sd, const char* varname, uint8 idx, void* value, int* refcache)
{
	int key;

	if( not_array_variable(varname[0]) || !not_server_variable(varname[0]) )
	{
		ShowError("script_setarray_pc: Variable '%s' has invalid scope (char_id=%d).\n", varname, sd->status.char_id);
		return;
	}

	if( idx >= SCRIPT_MAX_ARRAYSIZE )
	{
		ShowError("script_setarray_pc: Variable '%s' has invalid index '%d' (char_id=%d).\n", varname, (int)idx, sd->status.char_id);
		return;
	}

	key = ( refcache && refcache[0] ) ? refcache[0] : add_str(varname);

	if( is_string_variable(varname) )
	{
		pc_setregstr(sd, reference_uid(key, idx), (const char*)value);
	}
	else
	{
		pc_setreg(sd, reference_uid(key, idx), (int)__64BPTRSIZE(value));
	}

	if( refcache )
	{// save to avoid repeated add_str calls
		refcache[0] = key;
	}
}
/*==========================================
 * Destructor
 *------------------------------------------*/
void do_final_script(void) {
	int i;
#ifdef DEBUG_HASH
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
			for(i=LABEL_START; i<str_num; i++) {
				unsigned int h = calc_hash(get_str(i));
				fprintf(fp,"%04d : %4u : %s\n",i,h, get_str(i));
				++count[h];
			}
			fprintf(fp,"--------------------\n\n");
			memset(count2, 0, sizeof(count2));
			for(i=0; i<SCRIPT_HASH_SIZE; i++) {
				fprintf(fp,"  hash %3d = %d\n",i,count[i]);
				if(min > count[i])
					min = count[i];		// minimun count of collision
				if(max < count[i])
					max = count[i];		// maximun count of collision
				if(count[i] == 0)
					zero++;
				++count2[count[i]];
			}
			fprintf(fp,"\n--------------------\n  items : buckets\n--------------------\n");
			for( i=min; i <= max; ++i ){
				fprintf(fp,"  %5d : %7d\n",i,count2[i]);
				mean += 1.0f*i*count2[i]/SCRIPT_HASH_SIZE; // Note: this will always result in <nr labels>/<nr buckets>
			}
			for( i=min; i <= max; ++i ){
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

	mapreg_final();

	db_destroy(scriptlabel_db);
	userfunc_db->destroy(userfunc_db, db_script_free_code_sub);
	autobonus_db->destroy(autobonus_db, db_script_free_code_sub);
	if(sleep_db) {
		struct linkdb_node *n = (struct linkdb_node *)sleep_db;
		while(n) {
			struct script_state *st = (struct script_state *)n->data;
			script_free_state(st);
			n = n->next;
		}
		linkdb_final(&sleep_db);
	}

	if (str_data)
		aFree(str_data);
	if (str_buf)
		aFree(str_buf);

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
	
	if( script->hqs ) {
		for( i = 0; i < script->hqs; i++ ) {
			if( script->hq[i].item != NULL )
				aFree(script->hq[i].item);
		}
	}
	if( script->hqis ) {
		for( i = 0; i < script->hqis; i++ ) {
			if( script->hqi[i].item != NULL )
				aFree(script->hqi[i].item);
		}
	}
	if( script->hq != NULL )
		aFree(script->hq);
	if( script->hqi != NULL )
		aFree(script->hqi);
}
/*==========================================
 * Initialization
 *------------------------------------------*/
void do_init_script(void) {
	userfunc_db = strdb_alloc(DB_OPT_DUP_KEY,0);
	scriptlabel_db = strdb_alloc(DB_OPT_DUP_KEY,50);
	autobonus_db = strdb_alloc(DB_OPT_DUP_KEY,0);
	script->parse_builtin();
	read_constdb();
	mapreg_init();
}

int script_reload() {
	int i;

	userfunc_db->clear(userfunc_db, db_script_free_code_sub);
	db_clear(scriptlabel_db);

	for( i = 0; i < atcommand->binding_count; i++ ) {
		aFree(atcommand->binding[i]);
	}

	if( atcommand->binding_count != 0 )
		aFree(atcommand->binding);

	atcommand->binding_count = 0;

	if(sleep_db) {
		struct linkdb_node *n = (struct linkdb_node *)sleep_db;
		while(n) {
			struct script_state *st = (struct script_state *)n->data;
			script_free_state(st);
			n = n->next;
		}
		linkdb_final(&sleep_db);
	}
	mapreg_reload();
	return 0;
}

//-----------------------------------------------------------------------------
// buildin functions
//

#define BUILDIN_DEF(x,args) { buildin_ ## x , #x , args }
#define BUILDIN_DEF2(x,x2,args) { buildin_ ## x , x2 , args }

/////////////////////////////////////////////////////////////////////
// NPC interaction
//

/// Appends a message to the npc dialog.
/// If a dialog doesn't exist yet, one is created.
///
/// mes "<message>";
BUILDIN(mes)
{
	TBL_PC* sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	if( !script_hasdata(st, 3) )
	{// only a single line detected in the script
		clif->scriptmes(sd, st->oid, script_getstr(st, 2));
	}
	else
	{// parse multiple lines as they exist
		int i;
		
		for( i = 2; script_hasdata(st, i); i++ )
		{
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
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
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
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	st->state = CLOSE;
	clif->scriptclose(sd, st->oid);
	return true;
}

/// Displays the button 'close' on the npc dialog.
/// The dialog is closed and the script continues when the button is pressed.
///
/// close2;
BUILDIN(close2)
{
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	st->state = STOP;
	clif->scriptclose(sd, st->oid);
	return true;
}

/// Counts the number of valid and total number of options in 'str'
/// If max_count > 0 the counting stops when that valid option is reached
/// total is incremented for each option (NULL is supported)
static int menu_countoptions(const char* str, int max_count, int* total)
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
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
#ifdef SECURE_NPCTIMEOUT
	sd->npc_idle_type = NPCT_MENU;
#endif
	
	// TODO detect multiple scripts waiting for input at the same time, and what to do when that happens
	if( sd->state.menu_or_input == 0 )
	{
		struct StringBuf buf;
		struct script_data* data;
		
		if( script_lastdata(st) % 2 == 0 )
		{// argument count is not even (1st argument is at index 2)
			ShowError("script:menu: illegal number of arguments (%d).\n", (script_lastdata(st) - 1));
			st->state = END;
			return false;
		}
		
		StrBuf->Init(&buf);
		sd->npc_menu = 0;
		for( i = 2; i < script_lastdata(st); i += 2 )
		{
			// menu options
			text = script_getstr(st, i);
			
			// target label
			data = script_getdata(st, i+1);
			if( !data_islabel(data) )
			{// not a label
				StrBuf->Destroy(&buf);
				ShowError("script:menu: argument #%d (from 1) is not a label or label not found.\n", i);
				script_reportdata(data);
				st->state = END;
				return false;
			}
			
			// append option(s)
			if( text[0] == '\0' )
				continue;// empty string, ignore
			if( sd->npc_menu > 0 )
				StrBuf->AppendStr(&buf, ":");
			StrBuf->AppendStr(&buf, text);
			sd->npc_menu += menu_countoptions(text, 0, NULL);
		}
		st->state = RERUNLINE;
		sd->state.menu_or_input = 1;
		
		/**
		 * menus beyond this length crash the client (see bugreport:6402)
		 **/
		if( StrBuf->Length(&buf) >= 2047 ) {
			struct npc_data * nd = map_id2nd(st->oid);
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
			script_reportsrc(st);
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
			sd->npc_menu -= menu_countoptions(text, sd->npc_menu, &menu);
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
			script_reportdata(script_getdata(st, i + 1));
			st->state = END;
			return false;
		}
		pc_setreg(sd, add_str("@menu"), menu);
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
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
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
			sd->npc_menu += menu_countoptions(text, 0, NULL);
		}
		
		st->state = RERUNLINE;
		sd->state.menu_or_input = 1;
		
		/**
		 * menus beyond this length crash the client (see bugreport:6402)
		 **/
		if( StrBuf->Length(&buf) >= 2047 ) {
			struct npc_data * nd = map_id2nd(st->oid);
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
			script_reportsrc(st);
		}
	} else if( sd->npc_menu == 0xff ) {// Cancel was pressed
		sd->state.menu_or_input = 0;
		st->state = END;
	} else {// return selected option
		int menu = 0;
		
		sd->state.menu_or_input = 0;
		for( i = 2; i <= script_lastdata(st); ++i ) {
			text = script_getstr(st, i);
			sd->npc_menu -= menu_countoptions(text, sd->npc_menu, &menu);
			if( sd->npc_menu <= 0 )
				break;// entry found
		}
		pc_setreg(sd, add_str("@menu"), menu);
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
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
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
			sd->npc_menu += menu_countoptions(text, 0, NULL);
		}
		
		st->state = RERUNLINE;
		sd->state.menu_or_input = 1;
		
		/**
		 * menus beyond this length crash the client (see bugreport:6402)
		 **/
		if( StrBuf->Length(&buf) >= 2047 ) {
			struct npc_data * nd = map_id2nd(st->oid);
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
			script_reportsrc(st);
		}
	}
	else if( sd->npc_menu == 0xff )
	{// Cancel was pressed
		sd->state.menu_or_input = 0;
		pc_setreg(sd, add_str("@menu"), 0xff);
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
			sd->npc_menu -= menu_countoptions(text, sd->npc_menu, &menu);
			if( sd->npc_menu <= 0 )
				break;// entry found
		}
		pc_setreg(sd, add_str("@menu"), menu);
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
		script_reportdata(script_getdata(st,2));
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
	DBMap **ref = NULL;
	
	scr = (struct script_code*)strdb_get(userfunc_db, str);
	if( !scr )
	{
		ShowError("script:callfunc: function not found! [%s]\n", str);
		st->state = END;
		return false;
	}
	
	for( i = st->start+3, j = 0; i < st->end; i++, j++ )
	{
		struct script_data* data = push_copy(st->stack,i);
		if( data_isreference(data) && !data->ref )
		{
			const char* name = reference_getname(data);
			if( name[0] == '.' ) {
				ref = (struct DBMap**)aCalloc(sizeof(struct DBMap*), 1);
				ref[0] = (name[1] == '@' ? st->stack->var_function : st->script->script_vars);
				data->ref = ref;
			}
		}
	}
	
	CREATE(ri, struct script_retinfo, 1);
	ri->script       = st->script;// script code
	ri->var_function = st->stack->var_function;// scope variables
	ri->pos          = st->pos;// script location
	ri->nargs        = j;// argument count
	ri->defsp        = st->stack->defsp;// default stack pointer
	push_retinfo(st->stack, ri, ref);
	
	st->pos = 0;
	st->script = scr;
	st->stack->defsp = st->stack->sp;
	st->state = GOTO;
	st->stack->var_function = idb_alloc(DB_OPT_RELEASE_DATA);
	
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
	DBMap **ref = NULL;
	
	if( !data_islabel(script_getdata(st,2)) && !data_isfunclabel(script_getdata(st,2)) )
	{
		ShowError("script:callsub: argument is not a label\n");
		script_reportdata(script_getdata(st,2));
		st->state = END;
		return false;
	}
	
	for( i = st->start+3, j = 0; i < st->end; i++, j++ )
	{
		struct script_data* data = push_copy(st->stack,i);
		if( data_isreference(data) && !data->ref )
		{
			const char* name = reference_getname(data);
			if( name[0] == '.' && name[1] == '@' ) {
				if ( !ref ) {
					ref = (struct DBMap**)aCalloc(sizeof(struct DBMap*), 1);
					ref[0] = st->stack->var_function;
				}
				data->ref = ref;
			}
		}
	}
	
	CREATE(ri, struct script_retinfo, 1);
	ri->script       = st->script;// script code
	ri->var_function = st->stack->var_function;// scope variables
	ri->pos          = st->pos;// script location
	ri->nargs        = j;// argument count
	ri->defsp        = st->stack->defsp;// default stack pointer
	push_retinfo(st->stack, ri, ref);
	
	st->pos = pos;
	st->stack->defsp = st->stack->sp;
	st->state = GOTO;
	st->stack->var_function = idb_alloc(DB_OPT_RELEASE_DATA);
	
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
		push_copy(st->stack, st->stack->defsp - 1 - ri->nargs + idx);
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
BUILDIN(return)
{
	if( script_hasdata(st,2) )
	{// return value
		struct script_data* data;
		script_pushcopy(st, 2);
		data = script_getdatatop(st, -1);
		if( data_isreference(data) )
		{
			const char* name = reference_getname(data);
			if( name[0] == '.' && name[1] == '@' )
			{// scope variable
				if( !data->ref || data->ref == (DBMap**)&st->stack->var_function )
					get_val(st, data);// current scope, convert to value
			}
			else if( name[0] == '.' && !data->ref )
			{// script variable, link to current script
				data->ref = &st->script->script_vars;
			}
		}
	}
	else
	{// no return value
		script_pushnil(st);
	}
	st->state = RETFUNC;
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
	int max;
	
	if( script_hasdata(st,3) )
	{// min,max
		min = script_getnum(st,2);
		max = script_getnum(st,3);
		if( max < min )
			swap(min, max);
		range = max - min + 1;
	}
	else
	{// range
		min = 0;
		range = script_getnum(st,2);
	}
	if( range <= 1 )
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
	const char* str;
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	str = script_getstr(st,2);
	x = script_getnum(st,3);
	y = script_getnum(st,4);
	
	if(strcmp(str,"Random")==0)
		ret = pc_randomwarp(sd,CLR_TELEPORT);
	else if(strcmp(str,"SavePoint")==0 || strcmp(str,"Save")==0)
		ret = pc_setpos(sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,CLR_TELEPORT);
	else
		ret = pc_setpos(sd,mapindex_name2id(str),x,y,CLR_OUTSIGHT);
	
	if( ret ) {
		ShowError("buildin_warp: moving player '%s' to \"%s\",%d,%d failed.\n", sd->status.name, str, x, y);
		script_reportsrc(st);
	}
	
	return true;
}
/*==========================================
 * Warp a specified area
 *------------------------------------------*/
static int buildin_areawarp_sub(struct block_list *bl,va_list ap)
{
	int x2,y2,x3,y3;
	unsigned int index;
	
	index = va_arg(ap,unsigned int);
	x2 = va_arg(ap,int);
	y2 = va_arg(ap,int);
	x3 = va_arg(ap,int);
	y3 = va_arg(ap,int);
	
	if(index == 0)
		pc_randomwarp((TBL_PC *)bl,CLR_TELEPORT);
	else if(x3 && y3) {
		int max, tx, ty, j = 0;
		
		// choose a suitable max number of attempts
		if( (max = (y3-y2+1)*(x3-x2+1)*3) > 1000 )
			max = 1000;
		
		// find a suitable map cell
		do {
			tx = rnd()%(x3-x2+1)+x2;
			ty = rnd()%(y3-y2+1)+y2;
			j++;
		} while( map_getcell(index,tx,ty,CELL_CHKNOPASS) && j < max );
		
		pc_setpos((TBL_PC *)bl,index,tx,ty,CLR_OUTSIGHT);
	}
	else
		pc_setpos((TBL_PC *)bl,index,x2,y2,CLR_OUTSIGHT);
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
		if( (x3 = script_getnum(st,10)) < 0 || (y3 = script_getnum(st,11)) < 0 ){
			x3 = 0;
			y3 = 0;
		} else if( x3 && y3 ) {
			// normalize x3/y3 coordinates
			if( x3 < x2 ) swap(x3,x2);
			if( y3 < y2 ) swap(y3,y2);
		}
	}
	
	if( (m = map_mapname2mapid(mapname)) < 0 )
		return true;
	
	if( strcmp(str,"Random") == 0 )
		index = 0;
	else if( !(index=mapindex_name2id(str)) )
		return true;
	
	map_foreachinarea(buildin_areawarp_sub, m,x0,y0,x1,y1, BL_PC, index,x2,y2,x3,y3);
	return true;
}

/*==========================================
 * areapercentheal <map>,<x1>,<y1>,<x2>,<y2>,<hp>,<sp>
 *------------------------------------------*/
static int buildin_areapercentheal_sub(struct block_list *bl,va_list ap)
{
	int hp, sp;
	hp = va_arg(ap, int);
	sp = va_arg(ap, int);
	pc_percentheal((TBL_PC *)bl,hp,sp);
	return 0;
}
BUILDIN(areapercentheal)
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
	
	if( (m=map_mapname2mapid(mapname))< 0)
		return true;
	
	map_foreachinarea(buildin_areapercentheal_sub,m,x0,y0,x1,y1,BL_PC,hp,sp);
	return true;
}

/*==========================================
 * warpchar [LuzZza]
 * Useful for warp one player from
 * another player npc-session.
 * Using: warpchar "mapname",x,y,Char_ID;
 *------------------------------------------*/
BUILDIN(warpchar)
{
	int x,y,a;
	const char *str;
	TBL_PC *sd;
	
	str=script_getstr(st,2);
	x=script_getnum(st,3);
	y=script_getnum(st,4);
	a=script_getnum(st,5);
	
	sd = map_charid2sd(a);
	if( sd == NULL )
		return true;
	
	if(strcmp(str, "Random") == 0)
		pc_randomwarp(sd, CLR_TELEPORT);
	else
		if(strcmp(str, "SavePoint") == 0)
			pc_setpos(sd, sd->status.save_point.map,sd->status.save_point.x, sd->status.save_point.y, CLR_TELEPORT);
		else
			pc_setpos(sd, mapindex_name2id(str), x, y, CLR_TELEPORT);
	
	return true;
}
/*==========================================
 * Warpparty - [Fredzilla] [Paradox924X]
 * Syntax: warpparty "to_mapname",x,y,Party_ID,{"from_mapname"};
 * If 'from_mapname' is specified, only the party members on that map will be warped
 *------------------------------------------*/
BUILDIN(warpparty)
{
	TBL_PC *sd = NULL;
	TBL_PC *pl_sd;
	struct party_data* p;
	int type;
	int mapindex;
	int i;
	
	const char* str = script_getstr(st,2);
	int x = script_getnum(st,3);
	int y = script_getnum(st,4);
	int p_id = script_getnum(st,5);
	const char* str2 = NULL;
	if ( script_hasdata(st,6) )
		str2 = script_getstr(st,6);
	
	p = party_search(p_id);
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
			mapindex = pl_sd->mapindex;
			x = pl_sd->bl.x;
			y = pl_sd->bl.y;
			break;
		case 4:
			mapindex = mapindex_name2id(str);
			break;
		case 2:
			//"SavePoint" uses save point of the currently attached player
			if (( sd = script_rid2sd(st) ) == NULL )
				return true;
		default:
			mapindex = 0;
			break;
	}
	
	for (i = 0; i < MAX_PARTY; i++)
	{
		if( !(pl_sd = p->data[i].sd) || pl_sd->status.party_id != p_id )
			continue;
		
		if( str2 && strcmp(str2, map[pl_sd->bl.m].name) != 0 )
			continue;
		
		if( pc_isdead(pl_sd) )
			continue;
		
		switch( type )
		{
			case 0: // Random
				if(!map[pl_sd->bl.m].flag.nowarp)
					pc_randomwarp(pl_sd,CLR_TELEPORT);
				break;
			case 1: // SavePointAll
				if(!map[pl_sd->bl.m].flag.noreturn)
					pc_setpos(pl_sd,pl_sd->status.save_point.map,pl_sd->status.save_point.x,pl_sd->status.save_point.y,CLR_TELEPORT);
				break;
			case 2: // SavePoint
				if(!map[pl_sd->bl.m].flag.noreturn)
					pc_setpos(pl_sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,CLR_TELEPORT);
				break;
			case 3: // Leader
			case 4: // m,x,y
				if(!map[pl_sd->bl.m].flag.noreturn && !map[pl_sd->bl.m].flag.nowarp)
					pc_setpos(pl_sd,mapindex,x,y,CLR_TELEPORT);
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
	TBL_PC *sd = NULL;
	TBL_PC *pl_sd;
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
	
	if( type == 2 && ( sd = script_rid2sd(st) ) == NULL )
	{// "SavePoint" uses save point of the currently attached player
		return true;
	}
	
	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); pl_sd = (TBL_PC*)mapit->next(iter) )
	{
		if( pl_sd->status.guild_id != gid )
			continue;
		
		switch( type )
		{
			case 0: // Random
				if(!map[pl_sd->bl.m].flag.nowarp)
					pc_randomwarp(pl_sd,CLR_TELEPORT);
				break;
			case 1: // SavePointAll
				if(!map[pl_sd->bl.m].flag.noreturn)
					pc_setpos(pl_sd,pl_sd->status.save_point.map,pl_sd->status.save_point.x,pl_sd->status.save_point.y,CLR_TELEPORT);
				break;
			case 2: // SavePoint
				if(!map[pl_sd->bl.m].flag.noreturn)
					pc_setpos(pl_sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,CLR_TELEPORT);
				break;
			case 3: // m,x,y
				if(!map[pl_sd->bl.m].flag.noreturn && !map[pl_sd->bl.m].flag.nowarp)
					pc_setpos(pl_sd,mapindex_name2id(str),x,y,CLR_TELEPORT);
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
	TBL_PC *sd;
	int hp,sp;
	
	sd = script_rid2sd(st);
	if (!sd) return true;
	
	hp=script_getnum(st,2);
	sp=script_getnum(st,3);
	status_heal(&sd->bl, hp, sp, 1);
	return true;
}
/*==========================================
 * Heal a player by item (get vit bonus etc)
 *------------------------------------------*/
BUILDIN(itemheal)
{
	TBL_PC *sd;
	int hp,sp;
	
	hp=script_getnum(st,2);
	sp=script_getnum(st,3);
	
	if(potion_flag==1) {
		potion_hp = hp;
		potion_sp = sp;
		return true;
	}
	
	sd = script_rid2sd(st);
	if (!sd) return true;
	pc_itemheal(sd,sd->itemid,hp,sp);
	return true;
}
/*==========================================
 *
 *------------------------------------------*/
BUILDIN(percentheal)
{
	int hp,sp;
	TBL_PC* sd;
	
	hp=script_getnum(st,2);
	sp=script_getnum(st,3);
	
	if(potion_flag==1) {
		potion_per_hp = hp;
		potion_per_sp = sp;
		return true;
	}
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
#ifdef RENEWAL
	if( sd->sc.data[SC_EXTREMITYFIST2] )
		sp = 0;
#endif
	pc_percentheal(sd,hp,sp);
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
	
	if (pcdb_checkid(job))
	{
		TBL_PC* sd;
		
		sd = script_rid2sd(st);
		if( sd == NULL )
			return true;
		
		pc_jobchange(sd, job, upper);
	}
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN(jobname)
{
	int class_=script_getnum(st,2);
	script_pushconststr(st, (char*)job_name(class_));
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
	TBL_PC* sd;
	struct script_data* data;
	int uid;
	const char* name;
	int min;
	int max;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	data = script_getdata(st,2);
	if( !data_isreference(data) ){
		ShowError("script:input: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return false;
	}
	uid = reference_getuid(data);
	name = reference_getname(data);
	min = (script_hasdata(st,3) ? script_getnum(st,3) : script_config.input_min_value);
	max = (script_hasdata(st,4) ? script_getnum(st,4) : script_config.input_max_value);
	
#ifdef SECURE_NPCTIMEOUT
	sd->npc_idle_type = NPCT_WAIT;
#endif
	
	if( !sd->state.menu_or_input )
	{	// first invocation, display npc input box
		sd->state.menu_or_input = 1;
		st->state = RERUNLINE;
		if( is_string_variable(name) )
			clif->scriptinputstr(sd,st->oid);
		else
			clif->scriptinput(sd,st->oid);
	}
	else
	{	// take received text/value and store it in the designated variable
		sd->state.menu_or_input = 0;
		if( is_string_variable(name) )
		{
			int len = (int)strlen(sd->npc_str);
			set_reg(st, sd, uid, name, (void*)sd->npc_str, script_getref(st,2));
			script_pushint(st, (len > max ? 1 : len < min ? -1 : 0));
		}
		else
		{
			int amount = sd->npc_amount;
			set_reg(st, sd, uid, name, (void*)__64BPTRSIZE(cap_value(amount,min,max)), script_getref(st,2));
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
BUILDIN(set)
{
	TBL_PC* sd = NULL;
	struct script_data* data;
	//struct script_data* datavalue;
	int num;
	const char* name;
	char prefix;
	
	data = script_getdata(st,2);
	//datavalue = script_getdata(st,3);
	if( !data_isreference(data) )
	{
		ShowError("script:set: not a variable\n");
		script_reportdata(script_getdata(st,2));
		st->state = END;
		return false;
	}
	
	num = reference_getuid(data);
	name = reference_getname(data);
	prefix = *name;
	
	if( not_server_variable(prefix) )
	{
		sd = script_rid2sd(st);
		if( sd == NULL )
		{
			ShowError("script:set: no player attached for player variable '%s'\n", name);
			return true;
		}
	}
	
#if 0
	if( data_isreference(datavalue) )
	{// the value being referenced is a variable
		const char* namevalue = reference_getname(datavalue);
		
		if( !not_array_variable(*namevalue) )
		{// array variable being copied into another array variable
			if( sd == NULL && not_server_variable(*namevalue) && !(sd = script_rid2sd(st)) )
			{// player must be attached in order to copy a player variable
				ShowError("script:set: no player attached for player variable '%s'\n", namevalue);
				return true;
			}
			
			if( is_string_variable(namevalue) != is_string_variable(name) )
			{// non-matching array value types
				ShowWarning("script:set: two array variables do not match in type.\n");
				return true;
			}
			
			// push the maximum number of array values to the stack
			push_val(st->stack, C_INT, SCRIPT_MAX_ARRAYSIZE);
			
			// call the copy array method directly
			return buildin_copyarray(st);
		}
	}
#endif
	
	if( is_string_variable(name) )
		set_reg(st,sd,num,name,(void*)script_getstr(st,3),script_getref(st,2));
	else
		set_reg(st,sd,num,name,(void*)__64BPTRSIZE(script_getnum(st,3)),script_getref(st,2));
	
	// return a copy of the variable reference
	script_pushcopy(st,2);
	
	return true;
}

/////////////////////////////////////////////////////////////////////
/// Array variables
///

/// Returns the size of the specified array
static int32 getarraysize(struct script_state* st, int32 id, int32 idx, int isstring, struct DBMap** ref)
{
	int32 ret = idx;
	
	if( isstring )
	{
		for( ; idx < SCRIPT_MAX_ARRAYSIZE; ++idx )
		{
			char* str = (char*)get_val2(st, reference_uid(id, idx), ref);
			if( str && *str )
				ret = idx + 1;
			script_removetop(st, -1, 0);
		}
	}
	else
	{
		for( ; idx < SCRIPT_MAX_ARRAYSIZE; ++idx )
		{
			int32 num = (int32)__64BPTRSIZE(get_val2(st, reference_uid(id, idx), ref));
			if( num )
				ret = idx + 1;
			script_removetop(st, -1, 0);
		}
	}
	return ret;
}

/// Sets values of an array, from the starting index.
/// ex: setarray arr[1],1,2,3;
///
/// setarray <array variable>,<value1>{,<value2>...};
BUILDIN(setarray)
{
	struct script_data* data;
	const char* name;
	int32 start;
	int32 end;
	int32 id;
	int32 i;
	TBL_PC* sd = NULL;
	
	data = script_getdata(st, 2);
	if( !data_isreference(data) )
	{
		ShowError("script:setarray: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return false;// not a variable
	}
	
	id = reference_getid(data);
	start = reference_getindex(data);
	name = reference_getname(data);
	if( not_array_variable(*name) )
	{
		ShowError("script:setarray: illegal scope\n");
		script_reportdata(data);
		st->state = END;
		return false;// not supported
	}
	
	if( not_server_variable(*name) )
	{
		sd = script_rid2sd(st);
		if( sd == NULL )
			return true;// no player attached
	}
	
	end = start + script_lastdata(st) - 2;
	if( end > SCRIPT_MAX_ARRAYSIZE )
		end = SCRIPT_MAX_ARRAYSIZE;
	
	if( is_string_variable(name) )
	{// string array
		for( i = 3; start < end; ++start, ++i )
			set_reg(st, sd, reference_uid(id, start), name, (void*)script_getstr(st,i), reference_getref(data));
	}
	else
	{// int array
		for( i = 3; start < end; ++start, ++i )
			set_reg(st, sd, reference_uid(id, start), name, (void*)__64BPTRSIZE(script_getnum(st,i)), reference_getref(data));
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
	int32 start;
	int32 end;
	int32 id;
	void* v;
	TBL_PC* sd = NULL;
	
	data = script_getdata(st, 2);
	if( !data_isreference(data) )
	{
		ShowError("script:cleararray: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return false;// not a variable
	}
	
	id = reference_getid(data);
	start = reference_getindex(data);
	name = reference_getname(data);
	if( not_array_variable(*name) )
	{
		ShowError("script:cleararray: illegal scope\n");
		script_reportdata(data);
		st->state = END;
		return false;// not supported
	}
	
	if( not_server_variable(*name) )
	{
		sd = script_rid2sd(st);
		if( sd == NULL )
			return true;// no player attached
	}
	
	if( is_string_variable(name) )
		v = (void*)script_getstr(st, 3);
	else
		v = (void*)__64BPTRSIZE(script_getnum(st, 3));
	
	end = start + script_getnum(st, 4);
	if( end > SCRIPT_MAX_ARRAYSIZE )
		end = SCRIPT_MAX_ARRAYSIZE;
	
	for( ; start < end; ++start )
		set_reg(st, sd, reference_uid(id, start), name, v, script_getref(st,2));
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
	int32 count;
	TBL_PC* sd = NULL;
	
	data1 = script_getdata(st, 2);
	data2 = script_getdata(st, 3);
	if( !data_isreference(data1) || !data_isreference(data2) )
	{
		ShowError("script:copyarray: not a variable\n");
		script_reportdata(data1);
		script_reportdata(data2);
		st->state = END;
		return false;// not a variable
	}
	
	id1 = reference_getid(data1);
	id2 = reference_getid(data2);
	idx1 = reference_getindex(data1);
	idx2 = reference_getindex(data2);
	name1 = reference_getname(data1);
	name2 = reference_getname(data2);
	if( not_array_variable(*name1) || not_array_variable(*name2) )
	{
		ShowError("script:copyarray: illegal scope\n");
		script_reportdata(data1);
		script_reportdata(data2);
		st->state = END;
		return false;// not supported
	}
	
	if( is_string_variable(name1) != is_string_variable(name2) )
	{
		ShowError("script:copyarray: type mismatch\n");
		script_reportdata(data1);
		script_reportdata(data2);
		st->state = END;
		return false;// data type mismatch
	}
	
	if( not_server_variable(*name1) || not_server_variable(*name2) )
	{
		sd = script_rid2sd(st);
		if( sd == NULL )
			return true;// no player attached
	}
	
	count = script_getnum(st, 4);
	if( count > SCRIPT_MAX_ARRAYSIZE - idx1 )
		count = SCRIPT_MAX_ARRAYSIZE - idx1;
	if( count <= 0 || (id1 == id2 && idx1 == idx2) )
		return true;// nothing to copy
	
	if( id1 == id2 && idx1 > idx2 )
	{// destination might be overlapping the source - copy in reverse order
		for( i = count - 1; i >= 0; --i )
		{
			v = get_val2(st, reference_uid(id2, idx2 + i), reference_getref(data2));
			set_reg(st, sd, reference_uid(id1, idx1 + i), name1, v, reference_getref(data1));
			script_removetop(st, -1, 0);
		}
	}
	else
	{// normal copy
		for( i = 0; i < count; ++i )
		{
			if( idx2 + i < SCRIPT_MAX_ARRAYSIZE )
			{
				v = get_val2(st, reference_uid(id2, idx2 + i), reference_getref(data2));
				set_reg(st, sd, reference_uid(id1, idx1 + i), name1, v, reference_getref(data1));
				script_removetop(st, -1, 0);
			}
			else// out of range - assume ""/0
				set_reg(st, sd, reference_uid(id1, idx1 + i), name1, (is_string_variable(name1)?(void*)"":(void*)0), reference_getref(data1));
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
	const char* name;
	
	data = script_getdata(st, 2);
	if( !data_isreference(data) )
	{
		ShowError("script:getarraysize: not a variable\n");
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return false;// not a variable
	}
	
	name = reference_getname(data);
	if( not_array_variable(*name) )
	{
		ShowError("script:getarraysize: illegal scope\n");
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return false;// not supported
	}
	
	script_pushint(st, getarraysize(st, reference_getid(data), reference_getindex(data), is_string_variable(name), reference_getref(data)));
	return true;
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
	int start;
	int end;
	int id;
	TBL_PC *sd = NULL;
	
	data = script_getdata(st, 2);
	if( !data_isreference(data) )
	{
		ShowError("script:deletearray: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return false;// not a variable
	}
	
	id = reference_getid(data);
	start = reference_getindex(data);
	name = reference_getname(data);
	if( not_array_variable(*name) )
	{
		ShowError("script:deletearray: illegal scope\n");
		script_reportdata(data);
		st->state = END;
		return false;// not supported
	}
	
	if( not_server_variable(*name) )
	{
		sd = script_rid2sd(st);
		if( sd == NULL )
			return true;// no player attached
	}
	
	end = SCRIPT_MAX_ARRAYSIZE;
	
	if( start >= end )
		return true;// nothing to free
	
	if( script_hasdata(st,3) )
	{
		int count = script_getnum(st, 3);
		if( count > end - start )
			count = end - start;
		if( count <= 0 )
			return true;// nothing to free
		
		// move rest of the elements backward
		for( ; start + count < end; ++start )
		{
			void* v = get_val2(st, reference_uid(id, start + count), reference_getref(data));
			set_reg(st, sd, reference_uid(id, start), name, v, reference_getref(data));
			script_removetop(st, -1, 0);
		}
	}
	
	// clear the rest of the array
	if( is_string_variable(name) )
	{
		for( ; start < end; ++start )
			set_reg(st, sd, reference_uid(id, start), name, (void *)"", reference_getref(data));
	}
	else
	{
		for( ; start < end; ++start )
			set_reg(st, sd, reference_uid(id, start), name, (void*)0, reference_getref(data));
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
	const char* name;
	int32 id;
	int i;
	
	data = script_getdata(st, 2);
	if( !data_isreference(data) )
	{
		ShowError("script:getelementofarray: not a variable\n");
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return false;// not a variable
	}
	
	id = reference_getid(data);
	name = reference_getname(data);
	if( not_array_variable(*name) )
	{
		ShowError("script:getelementofarray: illegal scope\n");
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return false;// not supported
	}
	
	i = script_getnum(st, 3);
	if( i < 0 || i >= SCRIPT_MAX_ARRAYSIZE )
	{
		ShowWarning("script:getelementofarray: index out of range (%d)\n", i);
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return false;// out of range
	}
	
	push_val2(st->stack, C_NAME, reference_uid(id, i), reference_getref(data));
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
	TBL_PC* sd;
	
	type=script_getnum(st,2);
	val=script_getnum(st,3);
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	pc_changelook(sd,type,val);
	
	return true;
}

BUILDIN(changelook)
{ // As setlook but only client side
	int type,val;
	TBL_PC* sd;
	
	type=script_getnum(st,2);
	val=script_getnum(st,3);
	
	sd = script_rid2sd(st);
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
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
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
	TBL_PC* sd;
	
	type=script_getnum(st,2);
	x=script_getnum(st,3);
	y=script_getnum(st,4);
	id=script_getnum(st,5);
	color=script_getnum(st,6);
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	clif->viewpoint(sd,st->oid,type,x,y,id,color);
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN(countitem)
{
	int nameid, i;
	int count = 0;
	struct item_data* id = NULL;
	struct script_data* data;
	
	TBL_PC* sd = script_rid2sd(st);
	if (!sd) {
		script_pushint(st,0);
		return true;
	}
	
	data = script_getdata(st,2);
	get_val(st, data);  // convert into value in case of a variable
	
	if( data_isstring(data) )
	{// item name
		id = itemdb_searchname(script->conv_str(st, data));
	}
	else
	{// item id
		id = itemdb_exists(script->conv_num(st, data));
	}
	
	if( id == NULL )
	{
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
 * countitem2(nameID,Identified,Refine,Attribute,Card0,Card1,Card2,Card3)	[Lupus]
 *	returns number of items that meet the conditions
 *------------------------------------------*/
BUILDIN(countitem2)
{
	int nameid, iden, ref, attr, c1, c2, c3, c4;
	int count = 0;
	int i;
	struct item_data* id = NULL;
	struct script_data* data;
	
	TBL_PC* sd = script_rid2sd(st);
	if (!sd) {
		script_pushint(st,0);
		return true;
	}
	
	data = script_getdata(st,2);
	get_val(st, data);  // convert into value in case of a variable
	
	if( data_isstring(data) )
	{// item name
		id = itemdb_searchname(script->conv_str(st, data));
	}
	else
	{// item id
		id = itemdb_exists(script->conv_num(st, data));
	}
	
	if( id == NULL )
	{
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
 *	0 : fail
 *	1 : success (npc side only)
 *------------------------------------------*/
BUILDIN(checkweight)
{
	int nameid, amount, slots, amount2=0;
	unsigned int weight=0, i, nbargs;
	struct item_data* id = NULL;
	struct map_session_data* sd;
	struct script_data* data;
	
	if( ( sd = script_rid2sd(st) ) == NULL ){
		return true;
	}
	nbargs = script_lastdata(st)+1;
	if(nbargs%2){
	    ShowError("buildin_checkweight: Invalid nb of args should be a multiple of 2.\n");
	    script_pushint(st,0);
	    return false;
	}
	slots = pc_inventoryblank(sd); //nb of empty slot
	
	for(i=2; i<nbargs; i=i+2){
	    data = script_getdata(st,i);
	    get_val(st, data);  // convert into value in case of a variable
	    if( data_isstring(data) ){// item name
		    id = itemdb_searchname(script->conv_str(st, data));
	    } else {// item id
		    id = itemdb_exists(script->conv_num(st, data));
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
		
	    switch( pc_checkadditem(sd, nameid, amount) )
	    {
		    case ADDITEM_EXIST:
			    // item is already in inventory, but there is still space for the requested amount
			    break;
		    case ADDITEM_NEW:
			    if( itemdb_isstackable(nameid) ) {// stackable
				    amount2++;
				    if( slots < amount2 ) {
					    script_pushint(st,0);
					    return true;
				    }
			    }
			    else {// non-stackable
				    amount2 += amount;
				    if( slots < amount2){
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
	int32 nameid=-1, amount=-1;
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
	
	TBL_PC *sd = script_rid2sd(st);
	nullpo_retr(1,sd);
	
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
	
	if( not_array_variable(*name_it) || not_array_variable(*name_nb))
	{
		ShowError("script:checkweight2: illegal scope\n");
		script_pushint(st,0);
		return false;// not supported
	}
	if(is_string_variable(name_it) || is_string_variable(name_nb)){
		ShowError("script:checkweight2: illegal type, need int\n");
		script_pushint(st,0);
		return false;// not supported
	}
	nb_it = getarraysize(st, id_it, idx_it, 0, reference_getref(data_it));
	nb_nb = getarraysize(st, id_nb, idx_nb, 0, reference_getref(data_nb));
	if(nb_it != nb_nb){
		ShowError("Size mistmatch: nb_it=%d, nb_nb=%d\n",nb_it,nb_nb);
		fail = 1;
	}
	
	slots = pc_inventoryblank(sd);
	for(i=0; i<nb_it; i++){
		nameid = (int32)__64BPTRSIZE(get_val2(st,reference_uid(id_it,idx_it+i),reference_getref(data_it)));
	    script_removetop(st, -1, 0);
	    amount = (int32)__64BPTRSIZE(get_val2(st,reference_uid(id_nb,idx_nb+i),reference_getref(data_nb)));
	    script_removetop(st, -1, 0);
	    if(fail) continue; //cpntonie to depop rest
		
		if(itemdb_exists(nameid) == NULL ){
			ShowError("buildin_checkweight2: Invalid item '%d'.\n", nameid);
			fail=1;
			continue;
		}
		if(amount < 0 ){
			ShowError("buildin_checkweight2: Invalid amount '%d'.\n", amount);
			fail = 1;
			continue;
		}
	    weight += itemdb_weight(nameid)*amount;
	    if( weight + sd->weight > sd->max_weight ){
			fail = 1;
			continue;
	    }
	    switch( pc_checkadditem(sd, nameid, amount) ) {
		    case ADDITEM_EXIST:
				// item is already in inventory, but there is still space for the requested amount
			    break;
		    case ADDITEM_NEW:
			    if( itemdb_isstackable(nameid) ){// stackable
				    amount2++;
				    if( slots < amount2 )
					    fail = 1;
			    }
			    else {// non-stackable
				    amount2 += amount;
				    if( slots < amount2 ){
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
 *------------------------------------------*/
BUILDIN(getitem)
{
	int nameid,amount,get_count,i,flag = 0;
	struct item it;
	TBL_PC *sd;
	struct script_data *data;
	struct item_data *item_data;
	
	data=script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) )
	{// "<item name>"
		const char *name=script->conv_str(st,data);
		if( (item_data = itemdb_searchname(name)) == NULL ){
			ShowError("buildin_getitem: Nonexistant item %s requested.\n", name);
			return false; //No item created.
		}
		nameid=item_data->nameid;
	} else if( data_isint(data) ) {// <item id>
		nameid=script->conv_num(st,data);
		//Violet Box, Blue Box, etc - random item pick
		if( nameid < 0 ) {
			nameid = -nameid;
			flag = 1;
		}
		if( nameid <= 0 || !(item_data = itemdb_exists(nameid)) ){
			ShowError("buildin_getitem: Nonexistant item %d requested.\n", nameid);
			return false; //No item created.
		}
	} else {
		ShowError("buildin_getitem: invalid data type for argument #1 (%d).", data->type);
		return false;
	}
	
	// <amount>
	if( (amount=script_getnum(st,3)) <= 0)
		return true; //return if amount <=0, skip the useles iteration
	
	memset(&it,0,sizeof(it));
	it.nameid=nameid;
	if(!flag)
		it.identify=1;
	else
		it.identify=itemdb_isidentified2(item_data);
	
	if( script_hasdata(st,4) )
		sd=map_id2sd(script_getnum(st,4)); // <Account ID>
	else
		sd=script_rid2sd(st); // Attached player
	
	if( sd == NULL ) // no target
		return true;
	
	//Check if it's stackable.
	if (!itemdb_isstackable(nameid))
		get_count = 1;
	else
		get_count = amount;
	
	for (i = 0; i < amount; i += get_count)
	{
		// if not pet egg
		if (!pet_create_egg(sd, nameid))
		{
			if ((flag = pc_additem(sd, &it, get_count, LOG_TYPE_SCRIPT)))
			{
				clif->additem(sd, 0, 0, flag);
				if( pc_candrop(sd,&it) )
					map_addflooritem(&it,get_count,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
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
	int nameid,amount,get_count,i,flag = 0;
	int iden,ref,attr,c1,c2,c3,c4;
	struct item_data *item_data;
	struct item item_tmp;
	TBL_PC *sd;
	struct script_data *data;
	
	if( script_hasdata(st,11) )
		sd=map_id2sd(script_getnum(st,11)); // <Account ID>
	else
		sd=script_rid2sd(st); // Attached player
	
	if( sd == NULL ) // no target
		return true;
	
	data=script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) ){
		const char *name=script->conv_str(st,data);
		struct item_data *item_data = itemdb_searchname(name);
		if( item_data )
			nameid=item_data->nameid;
		else
			nameid=UNKNOWN_ITEM_ID;
	}else
		nameid=script->conv_num(st,data);
	
	amount=script_getnum(st,3);
	iden=script_getnum(st,4);
	ref=script_getnum(st,5);
	attr=script_getnum(st,6);
	c1=(short)script_getnum(st,7);
	c2=(short)script_getnum(st,8);
	c3=(short)script_getnum(st,9);
	c4=(short)script_getnum(st,10);
	
	if(nameid<0) { // Invalide nameid
		nameid = -nameid;
		flag = 1;
	}
	
	if(nameid > 0) {
		memset(&item_tmp,0,sizeof(item_tmp));
		item_data=itemdb_exists(nameid);
		if (item_data == NULL)
			return -1;
		if(item_data->type==IT_WEAPON || item_data->type==IT_ARMOR){
			if(ref > MAX_REFINE) ref = MAX_REFINE;
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
		item_tmp.card[0]=(short)c1;
		item_tmp.card[1]=(short)c2;
		item_tmp.card[2]=(short)c3;
		item_tmp.card[3]=(short)c4;
		
		//Check if it's stackable.
		if (!itemdb_isstackable(nameid))
			get_count = 1;
		else
			get_count = amount;
		
		for (i = 0; i < amount; i += get_count)
		{
			// if not pet egg
			if (!pet_create_egg(sd, nameid))
			{
				if ((flag = pc_additem(sd, &item_tmp, get_count, LOG_TYPE_SCRIPT)))
				{
					clif->additem(sd, 0, 0, flag);
					if( pc_candrop(sd,&item_tmp) )
						map_addflooritem(&item_tmp,get_count,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
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
BUILDIN(rentitem)
{
	struct map_session_data *sd;
	struct script_data *data;
	struct item it;
	int seconds;
	int nameid = 0, flag;
	
	data = script_getdata(st,2);
	get_val(st,data);
	
	if( (sd = script_rid2sd(st)) == NULL )
		return true;
	
	if( data_isstring(data) )
	{
		const char *name = script->conv_str(st,data);
		struct item_data *itd = itemdb_searchname(name);
		if( itd == NULL )
		{
			ShowError("buildin_rentitem: Nonexistant item %s requested.\n", name);
			return false;
		}
		nameid = itd->nameid;
	}
	else if( data_isint(data) )
	{
		nameid = script->conv_num(st,data);
		if( nameid <= 0 || !itemdb_exists(nameid) )
		{
			ShowError("buildin_rentitem: Nonexistant item %d requested.\n", nameid);
			return false;
		}
	}
	else
	{
		ShowError("buildin_rentitem: invalid data type for argument #1 (%d).\n", data->type);
		return false;
	}
	
	seconds = script_getnum(st,3);
	memset(&it, 0, sizeof(it));
	it.nameid = nameid;
	it.identify = 1;
	it.expire_time = (unsigned int)(time(NULL) + seconds);
	
	if( (flag = pc_additem(sd, &it, 1, LOG_TYPE_SCRIPT)) )
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
BUILDIN(getnameditem)
{
	int nameid;
	struct item item_tmp;
	TBL_PC *sd, *tsd;
	struct script_data *data;
	
	sd = script_rid2sd(st);
	if (sd == NULL)
	{	//Player not attached!
		script_pushint(st,0);
		return true;
	}
	
	data=script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) ){
		const char *name=script->conv_str(st,data);
		struct item_data *item_data = itemdb_searchname(name);
		if( item_data == NULL)
		{	//Failed
			script_pushint(st,0);
			return true;
		}
		nameid = item_data->nameid;
	}else
		nameid = script->conv_num(st,data);
	
	if(!itemdb_exists(nameid)/* || itemdb_isstackable(nameid)*/)
	{	//Even though named stackable items "could" be risky, they are required for certain quests.
		script_pushint(st,0);
		return true;
	}
	
	data=script_getdata(st,3);
	get_val(st,data);
	if( data_isstring(data) )	//Char Name
		tsd=map_nick2sd(script->conv_str(st,data));
	else	//Char Id was given
		tsd=map_charid2sd(script->conv_num(st,data));
	
	if( tsd == NULL )
	{	//Failed
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
	if(pc_additem(sd,&item_tmp,1,LOG_TYPE_SCRIPT)) {
		script_pushint(st,0);
		return true;	//Failed to add item, we will not drop if they don't fit
	}
	
	script_pushint(st,1);
	return true;
}

/*==========================================
 * gets a random item ID from an item group [Skotlex]
 * groupranditem group_num
 *------------------------------------------*/
BUILDIN(grouprandomitem)
{
	int group;
	
	group = script_getnum(st,2);
	script_pushint(st,itemdb_searchrandomid(group));
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN(makeitem)
{
	int nameid,amount,flag = 0;
	int x,y,m;
	const char *mapname;
	struct item item_tmp;
	struct script_data *data;
	struct item_data *item_data;

	data=script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) ){
		const char *name=script->conv_str(st,data);
		if( (item_data = itemdb_searchname(name)) )
			nameid=item_data->nameid;
		else
			nameid=UNKNOWN_ITEM_ID;
	} else {
		nameid=script->conv_num(st,data);
		if( nameid <= 0 || !(item_data = itemdb_exists(nameid)) ){
			ShowError("makeitem: Nonexistant item %d requested.\n", nameid);
			return false; //No item created.
		}
	}
	amount=script_getnum(st,3);
	mapname	=script_getstr(st,4);
	x	=script_getnum(st,5);
	y	=script_getnum(st,6);
	
	if(strcmp(mapname,"this")==0)
	{
		TBL_PC *sd;
		sd = script_rid2sd(st);
		if (!sd) return true; //Failed...
		m=sd->bl.m;
	} else
		m=map_mapname2mapid(mapname);
		
	memset(&item_tmp,0,sizeof(item_tmp));
	item_tmp.nameid=nameid;
	if(!flag)
		item_tmp.identify=1;
	else
		item_tmp.identify=itemdb_isidentified2(item_data);
	
	map_addflooritem(&item_tmp,amount,m,x,y,0,0,0,0);
	
	return true;
}


/// Counts / deletes the current item given by idx.
/// Used by buildin_delitem_search
/// Relies on all input data being already fully valid.
static void buildin_delitem_delete(struct map_session_data* sd, int idx, int* amount, bool delete_items)
{
	int delamount;
	struct item* inv = &sd->status.inventory[idx];
	
	delamount = ( amount[0] < inv->amount ) ? amount[0] : inv->amount;
	
	if( delete_items )
	{
		if( sd->inventory_data[idx]->type == IT_PETEGG && inv->card[0] == CARD0_PET )
		{// delete associated pet
			intif_delete_petdata(MakeDWord(inv->card[1], inv->card[2]));
		}
		pc_delitem(sd, idx, delamount, 0, 0, LOG_TYPE_SCRIPT);
	}
	
	amount[0]-= delamount;
}


/// Searches for item(s) and checks, if there is enough of them.
/// Used by delitem and delitem2
/// Relies on all input data being already fully valid.
/// @param exact_match will also match item attributes and cards, not just name id
/// @return true when all items could be deleted, false when there were not enough items to delete
static bool buildin_delitem_search(struct map_session_data* sd, struct item* it, bool exact_match)
{
	bool delete_items = false;
	int i, amount, important;
	struct item* inv;
	
	// prefer always non-equipped items
	it->equip = 0;
	
	// when searching for nameid only, prefer additionally
	if( !exact_match )
	{
		// non-refined items
		it->refine = 0;
		// card-less items
		memset(it->card, 0, sizeof(it->card));
	}
	
	for(;;)
	{
		amount = it->amount;
		important = 0;
		
		// 1st pass -- less important items / exact match
		for( i = 0; amount && i < ARRAYLENGTH(sd->status.inventory); i++ )
		{
			inv = &sd->status.inventory[i];
			
			if( !inv->nameid || !sd->inventory_data[i] || inv->nameid != it->nameid )
			{// wrong/invalid item
				continue;
			}
			
			if( inv->equip != it->equip || inv->refine != it->refine )
			{// not matching attributes
				important++;
				continue;
			}
			
			if( exact_match )
			{
				if( inv->identify != it->identify || inv->attribute != it->attribute || memcmp(inv->card, it->card, sizeof(inv->card)) )
				{// not matching exact attributes
					continue;
				}
			}
			else
			{
				if( sd->inventory_data[i]->type == IT_PETEGG )
				{
					if( inv->card[0] == CARD0_PET && CheckForCharServer() )
					{// pet which cannot be deleted
						continue;
					}
				}
				else if( memcmp(inv->card, it->card, sizeof(inv->card)) )
				{// named/carded item
					important++;
					continue;
				}
			}
			
			// count / delete item
			buildin_delitem_delete(sd, i, &amount, delete_items);
		}
		
		// 2nd pass -- any matching item
		if( amount == 0 || important == 0 )
		{// either everything was already consumed or no items were skipped
			;
		}
		else for( i = 0; amount && i < ARRAYLENGTH(sd->status.inventory); i++ )
		{
			inv = &sd->status.inventory[i];
			
			if( !inv->nameid || !sd->inventory_data[i] || inv->nameid != it->nameid )
			{// wrong/invalid item
				continue;
			}
			
			if( sd->inventory_data[i]->type == IT_PETEGG && inv->card[0] == CARD0_PET && CheckForCharServer() )
			{// pet which cannot be deleted
				continue;
			}
			
			if( exact_match )
			{
				if( inv->refine != it->refine || inv->identify != it->identify || inv->attribute != it->attribute || memcmp(inv->card, it->card, sizeof(inv->card)) )
				{// not matching attributes
					continue;
				}
			}
			
			// count / delete item
			buildin_delitem_delete(sd, i, &amount, delete_items);
		}
		
		if( amount )
		{// not enough items
			return false;
		}
		else if( delete_items )
		{// we are done with the work
			return true;
		}
		else
		{// get rid of the items now
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
	TBL_PC *sd;
	struct item it;
	struct script_data *data;
	
	if( script_hasdata(st,4) )
	{
		int account_id = script_getnum(st,4);
		sd = map_id2sd(account_id); // <account id>
		if( sd == NULL )
		{
			ShowError("script:delitem: player not found (AID=%d).\n", account_id);
			st->state = END;
			return false;
		}
	}
	else
	{
		sd = script_rid2sd(st);// attached player
		if( sd == NULL )
			return true;
	}
	
	data = script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) )
	{
		const char* item_name = script->conv_str(st,data);
		struct item_data* id = itemdb_searchname(item_name);
		if( id == NULL )
		{
			ShowError("script:delitem: unknown item \"%s\".\n", item_name);
			st->state = END;
			return false;
		}
		it.nameid = id->nameid;// "<item name>"
	}
	else
	{
		it.nameid = script->conv_num(st,data);// <item id>
		if( !itemdb_exists( it.nameid ) )
		{
			ShowError("script:delitem: unknown item \"%d\".\n", it.nameid);
			st->state = END;
			return false;
		}
	}
	
	it.amount=script_getnum(st,3);
	
	if( it.amount <= 0 )
		return true;// nothing to do
	
	if( buildin_delitem_search(sd, &it, false) )
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
	TBL_PC *sd;
	struct item it;
	struct script_data *data;
	
	if( script_hasdata(st,11) )
	{
		int account_id = script_getnum(st,11);
		sd = map_id2sd(account_id); // <account id>
		if( sd == NULL )
		{
			ShowError("script:delitem2: player not found (AID=%d).\n", account_id);
			st->state = END;
			return false;
		}
	}
	else
	{
		sd = script_rid2sd(st);// attached player
		if( sd == NULL )
			return true;
	}
	
	data = script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) )
	{
		const char* item_name = script->conv_str(st,data);
		struct item_data* id = itemdb_searchname(item_name);
		if( id == NULL )
		{
			ShowError("script:delitem2: unknown item \"%s\".\n", item_name);
			st->state = END;
			return false;
		}
		it.nameid = id->nameid;// "<item name>"
	}
	else
	{
		it.nameid = script->conv_num(st,data);// <item id>
		if( !itemdb_exists( it.nameid ) )
		{
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
	
	if( buildin_delitem_search(sd, &it, true) )
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
	TBL_PC *sd;
	sd=script_rid2sd(st);
	if (sd)
		st->npc_item_flag = sd->npc_item_flag = 1;
	return true;
}

BUILDIN(disableitemuse)
{
	TBL_PC *sd;
	sd=script_rid2sd(st);
	if (sd)
		st->npc_item_flag = sd->npc_item_flag = 0;
	return true;
}

/*==========================================
 * return the basic stats of sd
 * chk pc_readparam for available type
 *------------------------------------------*/
BUILDIN(readparam)
{
	int type;
	TBL_PC *sd;
	
	type=script_getnum(st,2);
	if( script_hasdata(st,3) )
		sd=map_nick2sd(script_getstr(st,3));
	else
		sd=script_rid2sd(st);
	
	if(sd==NULL){
		script_pushint(st,-1);
		return true;
	}
	
	script_pushint(st,pc_readparam(sd,type));
	
	return true;
}

/*==========================================
 * Return charid identification
 * return by @num :
 *	0 : char_id
 *	1 : party_id
 *	2 : guild_id
 *	3 : account_id
 *	4 : bg_id
 *------------------------------------------*/
BUILDIN(getcharid)
{
	int num;
	TBL_PC *sd;
	
	num = script_getnum(st,2);
	if( script_hasdata(st,3) )
		sd=map_nick2sd(script_getstr(st,3));
	else
		sd=script_rid2sd(st);
	
	if(sd==NULL){
		script_pushint(st,0);	//return 0, according docs
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
		if( ( nd = npc_name2id(script_getstr(st,3)) ) == NULL )
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
	
	if( ( p = party_search(party_id) ) != NULL )
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
 *	- : nom des membres
 *	1 : char_id des membres
 *	2 : account_id des membres
 *------------------------------------------*/
BUILDIN(getpartymember)
{
	struct party_data *p;
	int i,j=0,type=0;
	
	p=party_search(script_getnum(st,2));
	
	if( script_hasdata(st,3) )
 		type=script_getnum(st,3);
	
	if(p!=NULL){
		for(i=0;i<MAX_PARTY;i++){
			if(p->party.member[i].account_id){
				switch (type) {
					case 2:
						mapreg_setreg(reference_uid(add_str("$@partymemberaid"), j),p->party.member[i].account_id);
						break;
					case 1:
						mapreg_setreg(reference_uid(add_str("$@partymembercid"), j),p->party.member[i].char_id);
						break;
					default:
						mapreg_setregstr(reference_uid(add_str("$@partymembername$"), j),p->party.member[i].name);
				}
				j++;
			}
		}
	}
	mapreg_setreg(add_str("$@partymembercount"),j);
	
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
	
	p=party_search(party_id);
	
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
 * Get char string information by type :
 * Return by @type :
 *	0 : char_name
 *	1 : party_name or ""
 *	2 : guild_name or ""
 *	3 : map_name
 *	- : ""
 *------------------------------------------*/
BUILDIN(strcharinfo)
{
	TBL_PC *sd;
	int num;
	struct guild* g;
	struct party_data* p;
	
	sd=script_rid2sd(st);
	if (!sd) { //Avoid crashing....
		script_pushconststr(st,"");
		return true;
	}
	num=script_getnum(st,2);
	switch(num){
		case 0:
			script_pushstrcopy(st,sd->status.name);
			break;
		case 1:
			if( ( p = party_search(sd->status.party_id) ) != NULL ) {
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
			script_pushconststr(st,map[sd->bl.m].name);
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
 *	0 : name
 *	1 : str#
 *	2 : #str
 *	3 : ::str
 *	4 : map name
 *------------------------------------------*/
BUILDIN(strnpcinfo)
{
	TBL_NPC* nd;
	int num;
	char *buf,*name=NULL;
	
	nd = map_id2nd(st->oid);
	if (!nd) {
		script_pushconststr(st, "");
		return true;
	}
	
	num = script_getnum(st,2);
	switch(num){
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
			name = aStrdup(map[nd->bl.m].name);
			break;
	}
	
	if(name)
		script_pushstr(st, name);
	else
		script_pushconststr(st, "");
	
	return true;
}


// aegis->athena slot position conversion table
static unsigned int equip[] = {EQP_HEAD_TOP,EQP_ARMOR,EQP_HAND_L,EQP_HAND_R,EQP_GARMENT,EQP_SHOES,EQP_ACC_L,EQP_ACC_R,EQP_HEAD_MID,EQP_HEAD_LOW,EQP_COSTUME_HEAD_LOW,EQP_COSTUME_HEAD_MID,EQP_COSTUME_HEAD_TOP,EQP_COSTUME_GARMENT};

/*==========================================
 * GetEquipID(Pos);     Pos: 1-10
 *------------------------------------------*/
BUILDIN(getequipid)
{
	int i, num;
	TBL_PC* sd;
	struct item_data* item;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	num = script_getnum(st,2) - 1;
	if( num < 0 || num >= ARRAYLENGTH(equip) )
	{
		script_pushint(st,-1);
		return true;
	}
	
	// get inventory position of item
	i = pc_checkequip(sd,equip[num]);
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
	TBL_PC* sd;
	struct item_data* item;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	num = script_getnum(st,2) - 1;
	if( num < 0 || num >= ARRAYLENGTH(equip) )
	{
		script_pushconststr(st,"");
		return true;
	}
	
	// get inventory position of item
	i = pc_checkequip(sd,equip[num]);
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
	TBL_PC *sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	num=script_getnum(st,2);
	for(i=0; i<MAX_INVENTORY; i++) {
		if(sd->status.inventory[i].attribute){
			brokencounter++;
			if(num==brokencounter){
				id=sd->status.inventory[i].nameid;
				break;
			}
		}
	}
	
	script_pushint(st,id);
	
	return true;
}

/*==========================================
 * repair [Valaris]
 *------------------------------------------*/
BUILDIN(repair)
{
	int i,num;
	int repaircounter=0;
	TBL_PC *sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	num=script_getnum(st,2);
	for(i=0; i<MAX_INVENTORY; i++) {
		if(sd->status.inventory[i].attribute){
			repaircounter++;
			if(num==repaircounter){
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
	TBL_PC *sd;
	
	sd = script_rid2sd(st);
	if(sd == NULL)
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
	TBL_PC *sd;
	
	num = script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
	
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
 *	1 : true
 *	0 : false
 *------------------------------------------*/
BUILDIN(getequipisenableref)
{
	int i = -1,num;
	TBL_PC *sd;
	
	num = script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	if( num > 0 && num <= ARRAYLENGTH(equip) )
		i = pc_checkequip(sd,equip[num-1]);
	if( i >= 0 && sd->inventory_data[i] && !sd->inventory_data[i]->flag.no_refine && !sd->status.inventory[i].expire_time )
		script_pushint(st,1);
	else
		script_pushint(st,0);
	
	return true;
}

/*==========================================
 * Chk if the item equiped at pos is identify (huh ?)
 * return (npc)
 *	1 : true
 *	0 : false
 *------------------------------------------*/
BUILDIN(getequipisidentify)
{
	int i = -1,num;
	TBL_PC *sd;
	
	num = script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
	if(i >= 0)
		script_pushint(st,sd->status.inventory[i].identify);
	else
		script_pushint(st,0);
	
	return true;
}

/*==========================================
 * Get the item refined value at pos
 * return (npc)
 *	x : refine amount
 *	0 : false (not refined)
 *------------------------------------------*/
BUILDIN(getequiprefinerycnt)
{
	int i = -1,num;
	TBL_PC *sd;
	
	num = script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
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
 *	x : weapon level
 *	0 : false
 *------------------------------------------*/
BUILDIN(getequipweaponlv)
{
	int i = -1,num;
	TBL_PC *sd;
	
	num = script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
	if(i >= 0 && sd->inventory_data[i])
		script_pushint(st,sd->inventory_data[i]->wlv);
	else
		script_pushint(st,0);
	
	return true;
}

/*==========================================
 * Get the item refine chance (from refine.txt) for item at pos
 * return (npc)
 *	x : refine chance
 *	0 : false (max refine level or unequip..)
 *------------------------------------------*/
BUILDIN(getequippercentrefinery)
{
	int i = -1,num;
	TBL_PC *sd;
	
	num = script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
	if(i >= 0 && sd->status.inventory[i].nameid && sd->status.inventory[i].refine < MAX_REFINE)
		script_pushint(st,status_get_refine_chance(itemdb_wlv(sd->status.inventory[i].nameid), (int)sd->status.inventory[i].refine));
	else
		script_pushint(st,0);
	
	return true;
}

/*==========================================
 * Refine +1 item at pos and log and display refine
 *------------------------------------------*/
BUILDIN(successrefitem)
{
	int i=-1,num,ep;
	TBL_PC *sd;
	
	num = script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
	if(i >= 0) {
		ep=sd->status.inventory[i].equip;
		
		//Logs items, got from (N)PC scripts [Lupus]
		logs->pick_pc(sd, LOG_TYPE_SCRIPT, -1, &sd->status.inventory[i],sd->inventory_data[i]);
		
		sd->status.inventory[i].refine++;
		pc_unequipitem(sd,i,2); // status calc will happen in pc_equipitem() below
		
		clif->refine(sd->fd,0,i,sd->status.inventory[i].refine);
		clif->delitem(sd,i,1,3);
		
		//Logs items, got from (N)PC scripts [Lupus]
		logs->pick_pc(sd, LOG_TYPE_SCRIPT, 1, &sd->status.inventory[i],sd->inventory_data[i]);
		
		clif->additem(sd,i,1,0);
		pc_equipitem(sd,i,ep);
		clif->misceffect(&sd->bl,3);
		if(sd->status.inventory[i].refine == 10 &&
		   sd->status.inventory[i].card[0] == CARD0_FORGE &&
		   sd->status.char_id == (int)MakeDWord(sd->status.inventory[i].card[2],sd->status.inventory[i].card[3])
		   ){ // Fame point system [DracoRPG]
	 		switch (sd->inventory_data[i]->wlv){
				case 1:
					pc_addfame(sd,1); // Success to refine to +10 a lv1 weapon you forged = +1 fame point
					break;
				case 2:
					pc_addfame(sd,25); // Success to refine to +10 a lv2 weapon you forged = +25 fame point
					break;
				case 3:
					pc_addfame(sd,1000); // Success to refine to +10 a lv3 weapon you forged = +1000 fame point
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
	TBL_PC *sd;
	
	num = script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
	if(i >= 0) {
		sd->status.inventory[i].refine = 0;
		pc_unequipitem(sd,i,3); //recalculate bonus
		clif->refine(sd->fd,1,i,sd->status.inventory[i].refine); //notify client of failure
		
		pc_delitem(sd,i,1,0,2,LOG_TYPE_SCRIPT);
		
		clif->misceffect(&sd->bl,2); 	// display failure effect
	}
	
	return true;
}

/*==========================================
 * Downgrades an Equipment Part by -1 . [Masao]
 *------------------------------------------*/
BUILDIN(downrefitem)
{
	int i = -1,num,ep;
	TBL_PC *sd;
	
	num = script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	if (num > 0 && num <= ARRAYLENGTH(equip))
		i = pc_checkequip(sd,equip[num-1]);
	if(i >= 0) {
		ep = sd->status.inventory[i].equip;
		
		//Logs items, got from (N)PC scripts [Lupus]
		logs->pick_pc(sd, LOG_TYPE_SCRIPT, -1, &sd->status.inventory[i],sd->inventory_data[i]);
		
		sd->status.inventory[i].refine++;
		pc_unequipitem(sd,i,2); // status calc will happen in pc_equipitem() below
		
		clif->refine(sd->fd,2,i,sd->status.inventory[i].refine = sd->status.inventory[i].refine - 2);
		clif->delitem(sd,i,1,3);
		
		//Logs items, got from (N)PC scripts [Lupus]
		logs->pick_pc(sd, LOG_TYPE_SCRIPT, 1, &sd->status.inventory[i],sd->inventory_data[i]);
		
		clif->additem(sd,i,1,0);
		pc_equipitem(sd,i,ep);
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
	TBL_PC *sd;
	
	num = script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
	if(i >= 0) {
		pc_unequipitem(sd,i,3); //recalculate bonus
		pc_delitem(sd,i,1,0,2,LOG_TYPE_SCRIPT);
	}
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN(statusup)
{
	int type;
	TBL_PC *sd;
	
	type=script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	pc_statusup(sd,type);
	
	return true;
}
/*==========================================
 *
 *------------------------------------------*/
BUILDIN(statusup2)
{
	int type,val;
	TBL_PC *sd;
	
	type=script_getnum(st,2);
	val=script_getnum(st,3);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	pc_statusup2(sd,type,val);
	
	return true;
}

/// See 'doc/item_bonus.txt'
///
/// bonus <bonus type>,<val1>;
/// bonus2 <bonus type>,<val1>,<val2>;
/// bonus3 <bonus type>,<val1>,<val2>,<val3>;
/// bonus4 <bonus type>,<val1>,<val2>,<val3>,<val4>;
/// bonus5 <bonus type>,<val1>,<val2>,<val3>,<val4>,<val5>;
BUILDIN(bonus)
{
	int type;
	int val1;
	int val2 = 0;
	int val3 = 0;
	int val4 = 0;
	int val5 = 0;
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
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
			val1 = ( script_isstring(st,3) ? skill->name2id(script_getstr(st,3)) : script_getnum(st,3) );
			break;
		default:
			val1 = script_getnum(st,3);
			break;
	}
	
	switch( script_lastdata(st)-2 ) {
		case 1:
			pc_bonus(sd, type, val1);
			break;
		case 2:
			val2 = script_getnum(st,4);
			pc_bonus2(sd, type, val1, val2);
			break;
		case 3:
			val2 = script_getnum(st,4);
			val3 = script_getnum(st,5);
			pc_bonus3(sd, type, val1, val2, val3);
			break;
		case 4:
			if( type == SP_AUTOSPELL_ONSKILL && script_isstring(st,4) )
				val2 = skill->name2id(script_getstr(st,4)); // 2nd value can be skill name
			else
				val2 = script_getnum(st,4);
			
			val3 = script_getnum(st,5);
			val4 = script_getnum(st,6);
			pc_bonus4(sd, type, val1, val2, val3, val4);
			break;
		case 5:
			if( type == SP_AUTOSPELL_ONSKILL && script_isstring(st,4) )
				val2 = skill->name2id(script_getstr(st,4)); // 2nd value can be skill name
			else
				val2 = script_getnum(st,4);
			
			val3 = script_getnum(st,5);
			val4 = script_getnum(st,6);
			val5 = script_getnum(st,7);
			pc_bonus5(sd, type, val1, val2, val3, val4, val5);
			break;
		default:
			ShowDebug("buildin_bonus: unexpected number of arguments (%d)\n", (script_lastdata(st) - 1));
			break;
	}
	
	return true;
}

BUILDIN(autobonus)
{
	unsigned int dur;
	short rate;
	short atk_type = 0;
	TBL_PC* sd;
	const char *bonus_script, *other_script = NULL;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true; // no player attached
	
	if( sd->state.autobonus&sd->status.inventory[current_equip_item_index].equip )
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
	
	if( pc_addautobonus(sd->autobonus,ARRAYLENGTH(sd->autobonus),
						bonus_script,rate,dur,atk_type,other_script,sd->status.inventory[current_equip_item_index].equip,false) )
	{
		script_add_autobonus(bonus_script);
		if( other_script )
			script_add_autobonus(other_script);
	}
	
	return true;
}

BUILDIN(autobonus2)
{
	unsigned int dur;
	short rate;
	short atk_type = 0;
	TBL_PC* sd;
	const char *bonus_script, *other_script = NULL;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true; // no player attached
	
	if( sd->state.autobonus&sd->status.inventory[current_equip_item_index].equip )
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
	
	if( pc_addautobonus(sd->autobonus2,ARRAYLENGTH(sd->autobonus2),
						bonus_script,rate,dur,atk_type,other_script,sd->status.inventory[current_equip_item_index].equip,false) )
	{
		script_add_autobonus(bonus_script);
		if( other_script )
			script_add_autobonus(other_script);
	}
	
	return true;
}

BUILDIN(autobonus3)
{
	unsigned int dur;
	short rate,atk_type;
	TBL_PC* sd;
	const char *bonus_script, *other_script = NULL;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true; // no player attached
	
	if( sd->state.autobonus&sd->status.inventory[current_equip_item_index].equip )
		return true;
	
	rate = script_getnum(st,3);
	dur = script_getnum(st,4);
	atk_type = ( script_isstring(st,5) ? skill->name2id(script_getstr(st,5)) : script_getnum(st,5) );
	bonus_script = script_getstr(st,2);
	if( !rate || !dur || !atk_type || !bonus_script )
		return true;
	
	if( script_hasdata(st,6) )
		other_script = script_getstr(st,6);
	
	if( pc_addautobonus(sd->autobonus3,ARRAYLENGTH(sd->autobonus3),
						bonus_script,rate,dur,atk_type,other_script,sd->status.inventory[current_equip_item_index].equip,true) )
	{
		script_add_autobonus(bonus_script);
		if( other_script )
			script_add_autobonus(other_script);
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
BUILDIN(skill)
{
	int id;
	int level;
	int flag = 1;
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;// no player attached, report source
	
	id = ( script_isstring(st,2) ? skill->name2id(script_getstr(st,2)) : script_getnum(st,2) );
	level = script_getnum(st,3);
	if( script_hasdata(st,4) )
		flag = script_getnum(st,4);
	pc_skill(sd, id, level, flag);
	
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
BUILDIN(addtoskill)
{
	int id;
	int level;
	int flag = 2;
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;// no player attached, report source
	
	id = ( script_isstring(st,2) ? skill->name2id(script_getstr(st,2)) : script_getnum(st,2) );
	level = script_getnum(st,3);
	if( script_hasdata(st,4) )
		flag = script_getnum(st,4);
	pc_skill(sd, id, level, flag);
	
	return true;
}

/// Increases the level of a guild skill.
///
/// guildskill <skill id>,<amount>;
/// guildskill "<skill name>",<amount>;
BUILDIN(guildskill)
{
	int id;
	int level;
	TBL_PC* sd;
	int i;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;// no player attached, report source
	
	id = ( script_isstring(st,2) ? skill->name2id(script_getstr(st,2)) : script_getnum(st,2) );
	level = script_getnum(st,3);
	for( i=0; i < level; i++ )
		guild->skillup(sd, id);
	
	return true;
}

/// Returns the level of the player skill.
///
/// getskilllv(<skill id>) -> <level>
/// getskilllv("<skill name>") -> <level>
BUILDIN(getskilllv)
{
	int id;
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;// no player attached, report source
	
	id = ( script_isstring(st,2) ? skill->name2id(script_getstr(st,2)) : script_getnum(st,2) );
	script_pushint(st, pc_checkskill(sd,id));
	
	return true;
}

/// Returns the level of the guild skill.
///
/// getgdskilllv(<guild id>,<skill id>) -> <level>
/// getgdskilllv(<guild id>,"<skill name>") -> <level>
BUILDIN(getgdskilllv)
{
	int guild_id;
	uint16 skill_id;
	struct guild* g;
	
	guild_id = script_getnum(st,2);
	skill_id = ( script_isstring(st,3) ? skill->name2id(script_getstr(st,3)) : script_getnum(st,3) );
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
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;// no player attached, report source
	
	script_pushint(st, pc_get_group_level(sd));
	
	return true;
}

/// Returns the group ID of the player.
///
/// getgroupid() -> <int>
BUILDIN(getgroupid)
{
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if (sd == NULL)
		return false; // no player attached, report source
	script_pushint(st, pc_get_group_id(sd));
	
	return true;
}

/// Terminates the execution of this script instance.
///
/// end
BUILDIN(end)
{
	st->state = END;
	return true;
}

/// Checks if the player has that effect state (option).
///
/// checkoption(<option>) -> <bool>
BUILDIN(checkoption)
{
	int option;
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
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
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
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
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
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
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;// no player attached, report source
	
	option = script_getnum(st,2);
	if( script_hasdata(st,3) )
		flag = script_getnum(st,3);
	else if( !option ){// Request to remove everything.
		flag = 0;
		option = OPTION_FALCON|OPTION_RIDING;
#ifndef NEW_CARTS
		option |= OPTION_CART;
#endif
	}
	if( flag ){// Add option
		if( option&OPTION_WEDDING && !battle_config.wedding_modifydisplay )
			option &= ~OPTION_WEDDING;// Do not show the wedding sprites
		pc_setoption(sd, sd->sc.option|option);
	} else// Remove option
		pc_setoption(sd, sd->sc.option&~option);
	
	return true;
}

/// Returns if the player has a cart.
///
/// checkcart() -> <bool>
///
/// @author Valaris
BUILDIN(checkcart)
{
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
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
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;// no player attached, report source
	
	if( script_hasdata(st,2) )
		type = script_getnum(st,2);
	pc_setcart(sd, type);
	
	return true;
}

/// Returns if the player has a falcon.
///
/// checkfalcon() -> <bool>
///
/// @author Valaris
BUILDIN(checkfalcon)
{
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;// no player attached, report source
	
	if( pc_isfalcon(sd) )
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
	int flag = 1;
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;// no player attached, report source
	
	if( script_hasdata(st,2) )
		flag = script_getnum(st,2);
	
	pc_setfalcon(sd, flag);
	
	return true;
}

/// Returns if the player is riding.
///
/// checkriding() -> <bool>
///
/// @author Valaris
BUILDIN(checkriding)
{
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;// no player attached, report source
	
	if( pc_isriding(sd) || pc_isridingwug(sd) || pc_isridingdragon(sd) )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);
	
	return true;
}

/// Sets if the player is riding.
/// <flag> defaults to 1
///
/// setriding <flag>;
/// setriding;
BUILDIN(setriding)
{
	int flag = 1;
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;// no player attached, report source
	
	if( script_hasdata(st,2) )
		flag = script_getnum(st,2);
	pc_setriding(sd, flag);
	
	return true;
}

/// Returns if the player has a warg.
///
/// checkwug() -> <bool>
///
BUILDIN(checkwug)
{
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;// no player attached, report source
	
	if( pc_iswug(sd) || pc_isridingwug(sd) )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);
	
	return true;
}

/// Returns if the player is wearing MADO Gear.
///
/// checkmadogear() -> <bool>
///
BUILDIN(checkmadogear)
{
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;// no player attached, report source
	
	if( pc_ismadogear(sd) )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);
	
	return true;
}

/// Sets if the player is riding MADO Gear.
/// <flag> defaults to 1
///
/// setmadogear <flag>;
/// setmadogear;
BUILDIN(setmadogear)
{
	int flag = 1;
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;// no player attached, report source
	
	if( script_hasdata(st,2) )
		flag = script_getnum(st,2);
	pc_setmadogear(sd, flag);
	
	return true;
}

/// Sets the save point of the player.
///
/// save "<map name>",<x>,<y>
/// savepoint "<map name>",<x>,<y>
BUILDIN(savepoint)
{
	int x;
	int y;
	short map;
	const char* str;
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;// no player attached, report source
	
	str = script_getstr(st, 2);
	x   = script_getnum(st,3);
	y   = script_getnum(st,4);
	map = mapindex_name2id(str);
	if( map )
		pc_setsavepoint(sd, map, x, y);
	
	return true;
}

/*==========================================
 * GetTimeTick(0: System Tick, 1: Time Second Tick)
 *------------------------------------------*/
BUILDIN(gettimetick)	/* Asgard Version */
{
	int type;
	time_t timer;
	struct tm *t;
	
	type=script_getnum(st,2);
	
	switch(type){
		case 2:
			//type 2:(Get the number of seconds elapsed since 00:00 hours, Jan 1, 1970 UTC
			//        from the system clock.)
			script_pushint(st,(int)time(NULL));
			break;
		case 1:
			//type 1:(Second Ticks: 0-86399, 00:00:00-23:59:59)
			time(&timer);
			t=localtime(&timer);
			script_pushint(st,((t->tm_hour)*3600+(t->tm_min)*60+t->tm_sec));
			break;
		case 0:
		default:
			//type 0:(System Ticks)
			script_pushint(st,gettick());
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
BUILDIN(gettime)	/* Asgard Version */
{
	int type;
	time_t timer;
	struct tm *t;
	
	type=script_getnum(st,2);
	
	time(&timer);
	t=localtime(&timer);
	
	switch(type){
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
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	storage_storageopen(sd);
	return true;
}

BUILDIN(guildopenstorage)
{
	TBL_PC* sd;
	int ret;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	ret = storage_guild_storageopen(sd);
	script_pushint(st,ret);
	return true;
}

/*==========================================
 * Make player use a skill trought item usage
 *------------------------------------------*/
/// itemskill <skill id>,<level>{,flag
/// itemskill "<skill name>",<level>{,flag
BUILDIN(itemskill) {
	int id;
	int lv;
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL || sd->ud.skilltimer != INVALID_TIMER )
		return true;
	
	id = ( script_isstring(st,2) ? skill->name2id(script_getstr(st,2)) : script_getnum(st,2) );
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
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
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
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
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
	TBL_PC* sd;
	int id,pet_id;
	
	id=script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	pet_id = search_petDB_index(id, PET_CLASS);
	
	if (pet_id < 0)
		pet_id = search_petDB_index(id, PET_EGG);
	if (pet_id >= 0 && sd) {
		sd->catch_target_class = pet_db[pet_id].class_;
		intif_create_pet(
						 sd->status.account_id, sd->status.char_id,
						 (short)pet_db[pet_id].class_, (short)mob_db(pet_db[pet_id].class_)->lv,
						 (short)pet_db[pet_id].EggID, 0, (short)pet_db[pet_id].intimate,
						 100, 0, 1, pet_db[pet_id].jname);
	}
	
	return true;
}
/*==========================================
 * Give player exp base,job * quest_exp_rate/100
 *------------------------------------------*/
BUILDIN(getexp)
{
	TBL_PC* sd;
	int base=0,job=0;
	double bonus;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	base=script_getnum(st,2);
	job =script_getnum(st,3);
	if(base<0 || job<0)
		return true;
	
	// bonus for npc-given exp
	bonus = battle_config.quest_exp_rate / 100.;
	base = (int) cap_value(base * bonus, 0, INT_MAX);
	job = (int) cap_value(job * bonus, 0, INT_MAX);
	
	pc_gainexp(sd, NULL, base, job, true);
	
	return true;
}

/*==========================================
 * Gain guild exp [Celest]
 *------------------------------------------*/
BUILDIN(guildgetexp)
{
	TBL_PC* sd;
	int exp;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
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
	TBL_PC *sd;
	int guild_id;
	const char *name;
	
	guild_id = script_getnum(st,2);
	name = script_getstr(st,3);
	sd=map_nick2sd(name);
	
	if (!sd)
		script_pushint(st,0);
	else
		script_pushint(st,guild->gm_change(guild_id, sd));
	
	return true;
}

/*==========================================
 * Spawn a monster :
 @mapn,x,y : location
 @str : monster name
 @class_ : mob_id
 @amount : nb to spawn
 @event : event to attach to mob
 *------------------------------------------*/
BUILDIN(monster)
{
	const char* mapn	= script_getstr(st,2);
	int x				= script_getnum(st,3);
	int y				= script_getnum(st,4);
	const char* str		= script_getstr(st,5);
	int class_			= script_getnum(st,6);
	int amount			= script_getnum(st,7);
	const char* event	= "";
	unsigned int size	= SZ_SMALL;
	unsigned int ai		= AI_NONE;
	int mob_id;
	
	struct map_session_data* sd;
	int16 m;
	
	if (script_hasdata(st, 8))
	{
		event = script_getstr(st, 8);
		check_event(st, event);
	}
	
	if (script_hasdata(st, 9))
	{
		size = script_getnum(st, 9);
		if (size > 3)
		{
			ShowWarning("buildin_monster: Attempted to spawn non-existing size %d for monster class %d\n", size, class_);
			return false;
		}
	}
	
	if (script_hasdata(st, 10))
	{
		ai = script_getnum(st, 10);
		if (ai > 4)
		{
			ShowWarning("buildin_monster: Attempted to spawn non-existing ai %d for monster class %d\n", ai, class_);
			return false;
		}
	}
	
	if (class_ >= 0 && !mobdb_checkid(class_))
	{
		ShowWarning("buildin_monster: Attempted to spawn non-existing monster class %d\n", class_);
		return false;
	}
	
	sd = map_id2sd(st->rid);
	
	if (sd && strcmp(mapn, "this") == 0)
		m = sd->bl.m;
	else {
		m = map_mapname2mapid(mapn);
		if (map[m].flag.src4instance && st->instance_id >= 0) { // Try to redirect to the instance map, not the src map
			if ((m = instance->mapid2imapid(m, st->instance_id)) < 0) {
				ShowError("buildin_monster: Trying to spawn monster (%d) on instance map (%s) without instance attached.\n", class_, mapn);
				return false;
			}
		}
	}
	
	mob_id = mob_once_spawn(sd, m, x, y, str, class_, amount, event, size, ai);
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
	struct mob_db *mob;
	
	if( !mobdb_checkid(class_) )
	{
		script_pushint(st, 0);
		return true;
	}
	
	mob = mob_db(class_);
	
	for( i = 0; i < MAX_MOB_DROP; i++ )
	{
		if( mob->dropitem[i].nameid < 1 )
			continue;
		if( itemdb_exists(mob->dropitem[i].nameid) == NULL )
			continue;
		
		mapreg_setreg(reference_uid(add_str("$@MobDrop_item"), j), mob->dropitem[i].nameid);
		mapreg_setreg(reference_uid(add_str("$@MobDrop_rate"), j), mob->dropitem[i].p);
		
		j++;
	}
	
	mapreg_setreg(add_str("$@MobDrop_count"), j);
	script_pushint(st, 1);
	
	return true;
}
/*==========================================
 * Same as monster but randomize location in x0,x1,y0,y1 area
 *------------------------------------------*/
BUILDIN(areamonster)
{
	const char* mapn	= script_getstr(st,2);
	int x0				= script_getnum(st,3);
	int y0				= script_getnum(st,4);
	int x1				= script_getnum(st,5);
	int y1				= script_getnum(st,6);
	const char* str		= script_getstr(st,7);
	int class_			= script_getnum(st,8);
	int amount			= script_getnum(st,9);
	const char* event	= "";
	unsigned int size	= SZ_SMALL;
	unsigned int ai		= AI_NONE;
	int mob_id;
	
	struct map_session_data* sd;
	int16 m;
	
	if (script_hasdata(st,10)) {
		event = script_getstr(st, 10);
		check_event(st, event);
	}
	
	if (script_hasdata(st, 11)) {
		size = script_getnum(st, 11);
		if (size > 3) {
			ShowWarning("buildin_monster: Attempted to spawn non-existing size %d for monster class %d\n", size, class_);
			return false;
		}
	}
	
	if (script_hasdata(st, 12)) {
		ai = script_getnum(st, 12);
		if (ai > 4) {
			ShowWarning("buildin_monster: Attempted to spawn non-existing ai %d for monster class %d\n", ai, class_);
			return false;
		}
	}
	
	sd = map_id2sd(st->rid);
	
	if (sd && strcmp(mapn, "this") == 0)
		m = sd->bl.m;
	else {
		m = map_mapname2mapid(mapn);
		if (map[m].flag.src4instance && st->instance_id >= 0) { // Try to redirect to the instance map, not the src map
			if ((m = instance->mapid2imapid(m, st->instance_id)) < 0) {
				ShowError("buildin_areamonster: Trying to spawn monster (%d) on instance map (%s) without instance attached.\n", class_, mapn);
				return false;
			}
		}
	}
	
	mob_id = mob_once_spawn_area(sd, m, x0, y0, x1, y1, str, class_, amount, event, size, ai);
	script_pushint(st, mob_id);
	
	return true;
}
/*==========================================
 * KillMonster subcheck, verify if mob to kill ain't got an even to handle, could be force kill by allflag
 *------------------------------------------*/
static int buildin_killmonster_sub_strip(struct block_list *bl,va_list ap)
{ //same fix but with killmonster instead - stripping events from mobs.
	TBL_MOB* md = (TBL_MOB*)bl;
	char *event=va_arg(ap,char *);
	int allflag=va_arg(ap,int);
	
	md->state.npc_killmonster = 1;
	
	if(!allflag){
		if(strcmp(event,md->npc_event)==0)
			status_kill(bl);
	}else{
		if(!md->spawn)
			status_kill(bl);
	}
	md->state.npc_killmonster = 0;
	return 0;
}
static int buildin_killmonster_sub(struct block_list *bl,va_list ap)
{
	TBL_MOB* md = (TBL_MOB*)bl;
	char *event=va_arg(ap,char *);
	int allflag=va_arg(ap,int);
	
	if(!allflag){
		if(strcmp(event,md->npc_event)==0)
			status_kill(bl);
	}else{
		if(!md->spawn)
			status_kill(bl);
	}
	return 0;
}
BUILDIN(killmonster)
{
	const char *mapname,*event;
	int16 m,allflag=0;
	mapname=script_getstr(st,2);
	event=script_getstr(st,3);
	if(strcmp(event,"All")==0)
		allflag = 1;
	else
		check_event(st, event);
	
	if( (m=map_mapname2mapid(mapname))<0 )
		return true;
	
	if( map[m].flag.src4instance && st->instance_id >= 0 && (m = instance->mapid2imapid(m, st->instance_id)) < 0 )
		return true;
	
	if( script_hasdata(st,4) ) {
		if ( script_getnum(st,4) == 1 ) {
			map_foreachinmap(buildin_killmonster_sub, m, BL_MOB, event ,allflag);
			return true;
		}
	}
	
	map_freeblock_lock();
	map_foreachinmap(buildin_killmonster_sub_strip, m, BL_MOB, event ,allflag);
	map_freeblock_unlock();
	return true;
}

static int buildin_killmonsterall_sub_strip(struct block_list *bl,va_list ap)
{ //Strips the event from the mob if it's killed the old method.
	struct mob_data *md;
	
	md = BL_CAST(BL_MOB, bl);
	if (md->npc_event[0])
		md->npc_event[0] = 0;
	
	status_kill(bl);
	return 0;
}
static int buildin_killmonsterall_sub(struct block_list *bl,va_list ap)
{
	status_kill(bl);
	return 0;
}
BUILDIN(killmonsterall)
{
	const char *mapname;
	int16 m;
	mapname=script_getstr(st,2);
	
	if( (m = map_mapname2mapid(mapname))<0 )
		return true;
	
	if( map[m].flag.src4instance && st->instance_id >= 0 && (m = instance->mapid2imapid(m, st->instance_id)) < 0 )
		return true;
	
	if( script_hasdata(st,3) ) {
		if ( script_getnum(st,3) == 1 ) {
			map_foreachinmap(buildin_killmonsterall_sub,m,BL_MOB);
			return true;
		}
	}
	
	map_foreachinmap(buildin_killmonsterall_sub_strip,m,BL_MOB);
	return true;
}

/*==========================================
 * Creates a clone of a player.
 * clone map, x, y, event, char_id, master_id, mode, flag, duration
 *------------------------------------------*/
BUILDIN(clone)
{
	TBL_PC *sd, *msd=NULL;
	int char_id,master_id=0,x,y, mode = 0, flag = 0, m;
	unsigned int duration = 0;
	const char *map,*event="";
	
	map=script_getstr(st,2);
	x=script_getnum(st,3);
	y=script_getnum(st,4);
	event=script_getstr(st,5);
	char_id=script_getnum(st,6);
	
	if( script_hasdata(st,7) )
		master_id=script_getnum(st,7);
	
	if( script_hasdata(st,8) )
		mode=script_getnum(st,8);
	
	if( script_hasdata(st,9) )
		flag=script_getnum(st,9);
	
	if( script_hasdata(st,10) )
		duration=script_getnum(st,10);
	
	check_event(st, event);
	
	m = map_mapname2mapid(map);
	if (m < 0) return true;
	
	sd = map_charid2sd(char_id);
	
	if (master_id) {
		msd = map_charid2sd(master_id);
		if (msd)
			master_id = msd->bl.id;
		else
			master_id = 0;
	}
	if (sd) //Return ID of newly crafted clone.
		script_pushint(st,mob_clone_spawn(sd, m, x, y, event, master_id, mode, flag, 1000*duration));
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
	
	if( ( sd = script_rid2sd(st) ) == NULL )
	{
		return true;
	}
	
	check_event(st, event);
	npc_event(sd, event, 0);
	return true;
}
/*==========================================
 *------------------------------------------*/
BUILDIN(donpcevent)
{
	const char* event = script_getstr(st,2);
	check_event(st, event);
	if( !npc_event_do(event) ) {
		struct npc_data * nd = map_id2nd(st->oid);
		ShowDebug("NPCEvent '%s' not found! (source: %s)\n",event,nd?nd->name:"Unknown");
		script_pushint(st, 0);
	} else
		script_pushint(st, 1);
	return true;
}

/// for Aegis compatibility
/// basically a specialized 'donpcevent', with the event specified as two arguments instead of one
BUILDIN(cmdothernpc)	// Added by RoVeRT
{
	const char* npc = script_getstr(st,2);
	const char* command = script_getstr(st,3);
	char event[EVENT_NAME_LENGTH];
	snprintf(event, sizeof(event), "%s::OnCommand%s", npc, command);
	check_event(st, event);
	npc_event_do(event);
	return true;
}

/*==========================================
 *------------------------------------------*/
BUILDIN(addtimer)
{
	int tick = script_getnum(st,2);
	const char* event = script_getstr(st, 3);
	TBL_PC* sd;
	
	check_event(st, event);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	pc_addeventtimer(sd,tick,event);
	return true;
}
/*==========================================
 *------------------------------------------*/
BUILDIN(deltimer)
{
	const char *event;
	TBL_PC* sd;
	
	event=script_getstr(st, 2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	check_event(st, event);
	pc_deleventtimer(sd,event);
	return true;
}
/*==========================================
 *------------------------------------------*/
BUILDIN(addtimercount)
{
	const char *event;
	int tick;
	TBL_PC* sd;
	
	event=script_getstr(st, 2);
	tick=script_getnum(st,3);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	check_event(st, event);
	pc_addeventtimercount(sd,event,tick);
	return true;
}

/*==========================================
 *------------------------------------------*/
BUILDIN(initnpctimer)
{
	struct npc_data *nd;
	int flag = 0;
	
	if( script_hasdata(st,3) )
	{	//Two arguments: NPC name and attach flag.
		nd = npc_name2id(script_getstr(st, 2));
		flag = script_getnum(st,3);
	}
	else if( script_hasdata(st,2) )
	{	//Check if argument is numeric (flag) or string (npc name)
		struct script_data *data;
		data = script_getdata(st,2);
		get_val(st,data);
		if( data_isstring(data) ) //NPC name
			nd = npc_name2id(script->conv_str(st, data));
		else if( data_isint(data) ) //Flag
		{
			nd = (struct npc_data *)map_id2bl(st->oid);
			flag = script->conv_num(st,data);
		}
		else
		{
			ShowError("initnpctimer: invalid argument type #1 (needs be int or string)).\n");
			return false;
		}
	}
	else
		nd = (struct npc_data *)map_id2bl(st->oid);
	
	if( !nd )
		return true;
	if( flag ) //Attach
	{
		TBL_PC* sd = script_rid2sd(st);
		if( sd == NULL )
			return true;
		nd->u.scr.rid = sd->bl.id;
	}
	
	nd->u.scr.timertick = 0;
	npc_settimerevent_tick(nd,0);
	npc_timerevent_start(nd, st->rid);
	return true;
}
/*==========================================
 *------------------------------------------*/
BUILDIN(startnpctimer)
{
	struct npc_data *nd;
	int flag = 0;
	
	if( script_hasdata(st,3) )
	{	//Two arguments: NPC name and attach flag.
		nd = npc_name2id(script_getstr(st, 2));
		flag = script_getnum(st,3);
	}
	else if( script_hasdata(st,2) )
	{	//Check if argument is numeric (flag) or string (npc name)
		struct script_data *data;
		data = script_getdata(st,2);
		get_val(st,data);
		if( data_isstring(data) ) //NPC name
			nd = npc_name2id(script->conv_str(st, data));
		else if( data_isint(data) ) //Flag
		{
			nd = (struct npc_data *)map_id2bl(st->oid);
			flag = script->conv_num(st,data);
		}
		else
		{
			ShowError("initnpctimer: invalid argument type #1 (needs be int or string)).\n");
			return false;
		}
	}
	else
		nd=(struct npc_data *)map_id2bl(st->oid);
	
	if( !nd )
		return true;
	if( flag ) //Attach
	{
		TBL_PC* sd = script_rid2sd(st);
		if( sd == NULL )
			return true;
		nd->u.scr.rid = sd->bl.id;
	}
	
	npc_timerevent_start(nd, st->rid);
	return true;
}
/*==========================================
 *------------------------------------------*/
BUILDIN(stopnpctimer)
{
	struct npc_data *nd;
	int flag = 0;
	
	if( script_hasdata(st,3) )
	{	//Two arguments: NPC name and attach flag.
		nd = npc_name2id(script_getstr(st, 2));
		flag = script_getnum(st,3);
	}
	else if( script_hasdata(st,2) )
	{	//Check if argument is numeric (flag) or string (npc name)
		struct script_data *data;
		data = script_getdata(st,2);
		get_val(st,data);
		if( data_isstring(data) ) //NPC name
			nd = npc_name2id(script->conv_str(st, data));
		else if( data_isint(data) ) //Flag
		{
			nd = (struct npc_data *)map_id2bl(st->oid);
			flag = script->conv_num(st,data);
		}
		else
		{
			ShowError("initnpctimer: invalid argument type #1 (needs be int or string)).\n");
			return false;
		}
	}
	else
		nd=(struct npc_data *)map_id2bl(st->oid);
	
	if( !nd )
		return true;
	if( flag ) //Detach
		nd->u.scr.rid = 0;
	
	npc_timerevent_stop(nd);
	return true;
}
/*==========================================
 *------------------------------------------*/
BUILDIN(getnpctimer)
{
	struct npc_data *nd;
	TBL_PC *sd;
	int type = script_getnum(st,2);
	int val = 0;
	
	if( script_hasdata(st,3) )
		nd = npc_name2id(script_getstr(st,3));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);
	
	if( !nd || nd->bl.type != BL_NPC )
	{
		script_pushint(st,0);
		ShowError("getnpctimer: Invalid NPC.\n");
		return false;
	}
	
	switch( type )
	{
		case 0: val = npc_gettimerevent_tick(nd); break;
		case 1:
			if( nd->u.scr.rid )
			{
				sd = map_id2sd(nd->u.scr.rid);
				if( !sd )
				{
					ShowError("buildin_getnpctimer: Attached player not found!\n");
					break;
				}
				val = (sd->npc_timer_id != INVALID_TIMER);
			}
			else
				val = (nd->u.scr.timerid != INVALID_TIMER);
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
		nd = npc_name2id(script_getstr(st,3));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);
	
	if( !nd || nd->bl.type != BL_NPC )
	{
		script_pushint(st,1);
		ShowError("setnpctimer: Invalid NPC.\n");
		return false;
	}
	
	npc_settimerevent_tick(nd,tick);
	script_pushint(st,0);
	return true;
}

/*==========================================
 * attaches the player rid to the timer [Celest]
 *------------------------------------------*/
BUILDIN(attachnpctimer)
{
	TBL_PC *sd;
	struct npc_data *nd = (struct npc_data *)map_id2bl(st->oid);
	
	if( !nd || nd->bl.type != BL_NPC )
	{
		script_pushint(st,1);
		ShowError("setnpctimer: Invalid NPC.\n");
		return false;
	}
	
	if( script_hasdata(st,2) )
		sd = map_nick2sd(script_getstr(st,2));
	else
		sd = script_rid2sd(st);
	
	if( !sd )
	{
		script_pushint(st,1);
		ShowWarning("attachnpctimer: Invalid player.\n");
		return false;
	}
	
	nd->u.scr.rid = sd->bl.id;
	script_pushint(st,0);
	return true;
}

/*==========================================
 * detaches a player rid from the timer [Celest]
 *------------------------------------------*/
BUILDIN(detachnpctimer)
{
	struct npc_data *nd;
	
	if( script_hasdata(st,2) )
		nd = npc_name2id(script_getstr(st,2));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);
	
	if( !nd || nd->bl.type != BL_NPC )
	{
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
BUILDIN(playerattached)
{
	if(st->rid == 0 || map_id2sd(st->rid) == NULL)
		script_pushint(st,0);
	else
		script_pushint(st,st->rid);
	return true;
}

/*==========================================
 *------------------------------------------*/
BUILDIN(announce)
{
	const char *mes       = script_getstr(st,2);
	int         flag      = script_getnum(st,3);
	const char *fontColor = script_hasdata(st,4) ? script_getstr(st,4) : NULL;
	int         fontType  = script_hasdata(st,5) ? script_getnum(st,5) : 0x190; // default fontType (FW_NORMAL)
	int         fontSize  = script_hasdata(st,6) ? script_getnum(st,6) : 12;    // default fontSize
	int         fontAlign = script_hasdata(st,7) ? script_getnum(st,7) : 0;     // default fontAlign
	int         fontY     = script_hasdata(st,8) ? script_getnum(st,8) : 0;     // default fontY
	
	if (flag&0x0f) // Broadcast source or broadcast region defined
	{
		send_target target;
		struct block_list *bl = (flag&0x08) ? map_id2bl(st->oid) : (struct block_list *)script_rid2sd(st); // If bc_npc flag is set, use NPC as broadcast source
		if (bl == NULL)
			return true;
		
		flag &= 0x07;
		target = (flag == 1) ? ALL_SAMEMAP :
		(flag == 2) ? AREA :
		(flag == 3) ? SELF :
		ALL_CLIENT;
		if (fontColor)
			clif->broadcast2(bl, mes, (int)strlen(mes)+1, strtol(fontColor, (char **)NULL, 0), fontType, fontSize, fontAlign, fontY, target);
		else
			clif->broadcast(bl, mes, (int)strlen(mes)+1, flag&0xf0, target);
	}
	else
	{
		if (fontColor)
			intif_broadcast2(mes, (int)strlen(mes)+1, strtol(fontColor, (char **)NULL, 0), fontType, fontSize, fontAlign, fontY);
		else
			intif_broadcast(mes, (int)strlen(mes)+1, flag&0xf0);
	}
	return true;
}
/*==========================================
 *------------------------------------------*/
static int buildin_announce_sub(struct block_list *bl, va_list ap)
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
		clif->broadcast2(bl, mes, len, strtol(fontColor, (char **)NULL, 0), fontType, fontSize, fontAlign, fontY, SELF);
	else
		clif->broadcast(bl, mes, len, type, SELF);
	return 0;
}
/* Runs item effect on attached character.
 * itemeffect <item id>;
 * itemeffect "<item name>"; */
BUILDIN(itemeffect) {
	TBL_NPC *nd;
	TBL_PC *sd;
	struct script_data *data;
	struct item_data *item_data;
	
	nullpo_retr( 1, ( sd = script_rid2sd( st ) ) );
	nullpo_retr( 1, ( nd = (TBL_NPC *)map_id2bl( sd->npc_id ) ) );
	
	data = script_getdata( st, 2 );
	get_val( st, data );
	
	if( data_isstring( data ) ){
		const char *name = script->conv_str( st, data );
		
		if( ( item_data = itemdb_searchname( name ) ) == NULL ){
			ShowError( "buildin_itemeffect: Nonexistant item %s requested.\n", name );
			return false;
		}
	} else if( data_isint( data ) ){
		int nameid = script->conv_num( st, data );
		
		if( ( item_data = itemdb_exists( nameid ) ) == NULL ){
			ShowError("buildin_itemeffect: Nonexistant item %d requested.\n", nameid );
			return false;
		}
	} else {
		ShowError("buildin_itemeffect: invalid data type for argument #1 (%d).", data->type );
		return false;
	}
	
	run_script( item_data->script, 0, sd->bl.id, nd->bl.id );
	
	return true;
}

BUILDIN(mapannounce)
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
	
	if ((m = map_mapname2mapid(mapname)) < 0)
		return true;
	
	map_foreachinmap(buildin_announce_sub, m, BL_PC,
					 mes, strlen(mes)+1, flag&0xf0, fontColor, fontType, fontSize, fontAlign, fontY);
	return true;
}
/*==========================================
 *------------------------------------------*/
BUILDIN(areaannounce)
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
	
	if ((m = map_mapname2mapid(mapname)) < 0)
		return true;
	
	map_foreachinarea(buildin_announce_sub, m, x0, y0, x1, y1, BL_PC,
					  mes, strlen(mes)+1, flag&0xf0, fontColor, fontType, fontSize, fontAlign, fontY);
	return true;
}

/*==========================================
 *------------------------------------------*/
BUILDIN(getusers)
{
	int flag, val = 0;
	struct map_session_data* sd;
	struct block_list* bl = NULL;
	
	flag = script_getnum(st,2);
	
	switch(flag&0x07)
	{
		case 0:
			if(flag&0x8)
			{// npc
				bl = map_id2bl(st->oid);
			}
			else if((sd = script_rid2sd(st))!=NULL)
			{// pc
				bl = &sd->bl;
			}
			
			if(bl)
			{
				val = map[bl->m].users;
			}
			break;
		case 1:
			val = map_getusers();
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
	TBL_PC *sd, *pl_sd;
	int /*disp_num=1,*/ group_level = 0;
	struct s_mapiterator* iter;
	
	sd = script_rid2sd(st);
	if (!sd) return true;
	
	group_level = pc_get_group_level(sd);
	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); pl_sd = (TBL_PC*)mapit->next(iter) )
	{
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
	int i=0,c=0;
	struct guild *g = NULL;
	str=script_getstr(st,2);
	gid=script_getnum(st,3);
	if ((m = map_mapname2mapid(str)) < 0) { // map id on this server (m == -1 if not in actual map-server)
		script_pushint(st,-1);
		return true;
	}
	g = guild->search(gid);
	
	if (g){
		for(i = 0; i < g->max_member; i++)
		{
			if (g->member[i].sd && g->member[i].sd->bl.m == m)
				c++;
		}
	}
	
	script_pushint(st,c);
	return true;
}
/*==========================================
 *------------------------------------------*/
BUILDIN(getmapusers)
{
	const char *str;
	int16 m;
	str=script_getstr(st,2);
	if( (m=map_mapname2mapid(str))< 0){
		script_pushint(st,-1);
		return true;
	}
	script_pushint(st,map[m].users);
	return true;
}
/*==========================================
 *------------------------------------------*/
static int buildin_getareausers_sub(struct block_list *bl,va_list ap)
{
	int *users=va_arg(ap,int *);
	(*users)++;
	return 0;
}
BUILDIN(getareausers)
{
	const char *str;
	int16 m,x0,y0,x1,y1,users=0; //doubt we can have more then 32k users on
	str=script_getstr(st,2);
	x0=script_getnum(st,3);
	y0=script_getnum(st,4);
	x1=script_getnum(st,5);
	y1=script_getnum(st,6);
	if( (m=map_mapname2mapid(str))< 0){
		script_pushint(st,-1);
		return true;
	}
	map_foreachinarea(buildin_getareausers_sub,
					  m,x0,y0,x1,y1,BL_PC,&users);
	script_pushint(st,users);
	return true;
}

/*==========================================
 *------------------------------------------*/
static int buildin_getareadropitem_sub(struct block_list *bl,va_list ap)
{
	int item=va_arg(ap,int);
	int *amount=va_arg(ap,int *);
	struct flooritem_data *drop=(struct flooritem_data *)bl;
	
	if(drop->item_data.nameid==item)
		(*amount)+=drop->item_data.amount;
	
	return 0;
}
BUILDIN(getareadropitem)
{
	const char *str;
	int16 m,x0,y0,x1,y1;
	int item,amount=0;
	struct script_data *data;
	
	str=script_getstr(st,2);
	x0=script_getnum(st,3);
	y0=script_getnum(st,4);
	x1=script_getnum(st,5);
	y1=script_getnum(st,6);
	
	data=script_getdata(st,7);
	get_val(st,data);
	if( data_isstring(data) ){
		const char *name=script->conv_str(st,data);
		struct item_data *item_data = itemdb_searchname(name);
		item=UNKNOWN_ITEM_ID;
		if( item_data )
			item=item_data->nameid;
	}else
		item=script->conv_num(st,data);
	
	if( (m=map_mapname2mapid(str))< 0){
		script_pushint(st,-1);
		return true;
	}
	map_foreachinarea(buildin_getareadropitem_sub,
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
	npc_enable(str,1);
	return true;
}
/*==========================================
 *------------------------------------------*/
BUILDIN(disablenpc)
{
	const char *str;
	str=script_getstr(st,2);
	npc_enable(str,0);
	return true;
}

/*==========================================
 *------------------------------------------*/
BUILDIN(hideoffnpc)
{
	const char *str;
	str=script_getstr(st,2);
	npc_enable(str,2);
	return true;
}
/*==========================================
 *------------------------------------------*/
BUILDIN(hideonnpc)
{
	const char *str;
	str=script_getstr(st,2);
	npc_enable(str,4);
	return true;
}

/// Starts a status effect on the target unit or on the attached player.
///
/// sc_start <effect_id>,<duration>,<val1>{,<unit_id>};
BUILDIN(sc_start)
{
	struct block_list* bl;
	enum sc_type type;
	int tick;
	int val1;
	int val4 = 0;
	
	type = (sc_type)script_getnum(st,2);
	tick = script_getnum(st,3);
	val1 = script_getnum(st,4);
	if( script_hasdata(st,5) )
		bl = map_id2bl(script_getnum(st,5));
	else
		bl = map_id2bl(st->rid);
	
	if( tick == 0 && val1 > 0 && type > SC_NONE && type < SC_MAX && status_sc2skill(type) != 0 )
	{// When there isn't a duration specified, try to get it from the skill_db
		tick = skill->get_time(status_sc2skill(type), val1);
	}
	
	if( potion_flag == 1 && potion_target )
	{	//skill.c set the flags before running the script, this must be a potion-pitched effect.
		bl = map_id2bl(potion_target);
		tick /= 2;// Thrown potions only last half.
		val4 = 1;// Mark that this was a thrown sc_effect
	}
	
	if( bl )
		status_change_start(bl, type, 10000, val1, 0, 0, val4, tick, 2);
	
	return true;
}

/// Starts a status effect on the target unit or on the attached player.
///
/// sc_start2 <effect_id>,<duration>,<val1>,<percent chance>{,<unit_id>};
BUILDIN(sc_start2)
{
	struct block_list* bl;
	enum sc_type type;
	int tick;
	int val1;
	int val4 = 0;
	int rate;
	
	type = (sc_type)script_getnum(st,2);
	tick = script_getnum(st,3);
	val1 = script_getnum(st,4);
	rate = script_getnum(st,5);
	if( script_hasdata(st,6) )
		bl = map_id2bl(script_getnum(st,6));
	else
		bl = map_id2bl(st->rid);
	
	if( tick == 0 && val1 > 0 && type > SC_NONE && type < SC_MAX && status_sc2skill(type) != 0 )
	{// When there isn't a duration specified, try to get it from the skill_db
		tick = skill->get_time(status_sc2skill(type), val1);
	}
	
	if( potion_flag == 1 && potion_target )
	{	//skill.c set the flags before running the script, this must be a potion-pitched effect.
		bl = map_id2bl(potion_target);
		tick /= 2;// Thrown potions only last half.
		val4 = 1;// Mark that this was a thrown sc_effect
	}
	
	if( bl )
		status_change_start(bl, type, rate, val1, 0, 0, val4, tick, 2);
	
	return true;
}

/// Starts a status effect on the target unit or on the attached player.
///
/// sc_start4 <effect_id>,<duration>,<val1>,<val2>,<val3>,<val4>{,<unit_id>};
BUILDIN(sc_start4)
{
	struct block_list* bl;
	enum sc_type type;
	int tick;
	int val1;
	int val2;
	int val3;
	int val4;
	
	type = (sc_type)script_getnum(st,2);
	tick = script_getnum(st,3);
	val1 = script_getnum(st,4);
	val2 = script_getnum(st,5);
	val3 = script_getnum(st,6);
	val4 = script_getnum(st,7);
	if( script_hasdata(st,8) )
		bl = map_id2bl(script_getnum(st,8));
	else
		bl = map_id2bl(st->rid);
	
	if( tick == 0 && val1 > 0 && type > SC_NONE && type < SC_MAX && status_sc2skill(type) != 0 )
	{// When there isn't a duration specified, try to get it from the skill_db
		tick = skill->get_time(status_sc2skill(type), val1);
	}
	
	if( potion_flag == 1 && potion_target )
	{	//skill.c set the flags before running the script, this must be a potion-pitched effect.
		bl = map_id2bl(potion_target);
		tick /= 2;// Thrown potions only last half.
	}
	
	if( bl )
		status_change_start(bl, type, 10000, val1, val2, val3, val4, tick, 2);
	
	return true;
}

/// Ends one or all status effects on the target unit or on the attached player.
///
/// sc_end <effect_id>{,<unit_id>};
BUILDIN(sc_end)
{
	struct block_list* bl;
	int type;
	
	type = script_getnum(st, 2);
	if (script_hasdata(st, 3))
		bl = map_id2bl(script_getnum(st, 3));
	else
		bl = map_id2bl(st->rid);
	
	if (potion_flag == 1 && potion_target) //##TODO how does this work [FlavioJS]
		bl = map_id2bl(potion_target);
	
	if (!bl)
		return true;
	
	if (type >= 0 && type < SC_MAX)
	{
		struct status_change *sc = status_get_sc(bl);
		struct status_change_entry *sce = sc ? sc->data[type] : NULL;
		
		if (!sce)
			return true;
		
		
		switch (type)
		{
			case SC_WEIGHT50:
			case SC_WEIGHT90:
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
		status_change_clear(bl, 3); // remove all effects
	
	return true;
}

/*==========================================
 * @FIXME atm will return reduced tick, 0 immune, 1 no tick
 *------------------------------------------*/
BUILDIN(getscrate)
{
	struct block_list *bl;
	int type,rate;
	
	type=script_getnum(st,2);
	rate=script_getnum(st,3);
	if( script_hasdata(st,4) ) //get for the bl assigned
		bl = map_id2bl(script_getnum(st,4));
	else
		bl = map_id2bl(st->rid);
	
	if (bl)
		rate = status_get_sc_def(bl, (sc_type)type, 10000, 10000, 0);
	
	script_pushint(st,rate);
	return true;
}

/*==========================================
 * getstatus <type>{, <info>};
 *------------------------------------------*/
BUILDIN(getstatus)
{
	int id, type;
	struct map_session_data* sd = script_rid2sd(st);
	
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
	
	switch( type )
	{
		case 1:	 script_pushint(st, sd->sc.data[id]->val1);	break;
		case 2:  script_pushint(st, sd->sc.data[id]->val2);	break;
		case 3:  script_pushint(st, sd->sc.data[id]->val3);	break;
		case 4:  script_pushint(st, sd->sc.data[id]->val4);	break;
		case 5:
		{
			struct TimerData* timer = (struct TimerData*)get_timer(sd->sc.data[id]->timer);
			
			if( timer )
			{// return the amount of time remaining
				script_pushint(st, timer->tick - gettick());
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
	TBL_PC *sd;
	
	pet_id= script_getnum(st,2);
	sd=script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	pet_catch_process1(sd,pet_id);
	return true;
}

/*==========================================
 * [orn]
 *------------------------------------------*/
BUILDIN(homunculus_evolution)
{
	TBL_PC *sd;
	
	sd=script_rid2sd(st);
	if( sd == NULL )
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
 *------------------------------------------*/
BUILDIN(homunculus_mutate) {
	int homun_id;
	enum homun_type m_class, m_id;
	TBL_PC *sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL || sd->hd == NULL )
		return true;
	
	if(script_hasdata(st,2))
		homun_id = script_getnum(st,2);
	else
		homun_id = 6048 + (rnd() % 4);
	
	if(homun_alive(sd->hd)) {
		m_class = homun->class2type(sd->hd->homunculus.class_);
		m_id    = homun->class2type(homun_id);
		
		if ( m_class != -1 && m_id != -1 && m_class == HT_EVO && m_id == HT_S && sd->hd->homunculus.level >= 99 )
			homun->mutate(sd->hd, homun_id);
		else
			clif->emotion(&sd->hd->bl, E_SWT);
	}
	return true;
}

// [Zephyrus]
BUILDIN(homunculus_shuffle) {
	TBL_PC *sd;
	
	sd=script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	if(homun_alive(sd->hd))
		homun->shuffle(sd->hd);
	
	return true;
}

//These two functions bring the eA MAPID_* class functionality to scripts.
BUILDIN(eaclass)
{
	int class_;
	if( script_hasdata(st,2) )
		class_ = script_getnum(st,2);
	else {
		TBL_PC *sd;
		sd=script_rid2sd(st);
		if (!sd) {
			script_pushint(st,-1);
			return true;
		}
		class_ = sd->status.class_;
	}
	script_pushint(st,pc_jobid2mapid(class_));
	return true;
}

BUILDIN(roclass)
{
	int class_ =script_getnum(st,2);
	int sex;
	if( script_hasdata(st,3) )
		sex = script_getnum(st,3);
	else {
		TBL_PC *sd;
		if (st->rid && (sd=script_rid2sd(st)))
			sex = sd->status.sex;
		else
			sex = 1; //Just use male when not found.
	}
	script_pushint(st,pc_mapid2jobid(class_, sex));
	return true;
}

/*==========================================
 * Tells client to open a hatching window, used for pet incubator
 *------------------------------------------*/
BUILDIN(birthpet)
{
	TBL_PC *sd;
	sd=script_rid2sd(st);
	if( sd == NULL )
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
 *	1 : make like after rebirth
 *	2 : blvl,jlvl=1, skillpoint=0
 * 	3 : don't reset skill, blvl=1
 *	4 : jlvl=0
 *------------------------------------------*/
BUILDIN(resetlvl)
{
	TBL_PC *sd;
	
	int type=script_getnum(st,2);
	
	sd=script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	pc_resetlvl(sd,type);
	return true;
}
/*==========================================
 * Reset a player status point
 *------------------------------------------*/
BUILDIN(resetstatus)
{
	TBL_PC *sd;
	sd=script_rid2sd(st);
	pc_resetstate(sd);
	return true;
}

/*==========================================
 * script command resetskill
 *------------------------------------------*/
BUILDIN(resetskill)
{
	TBL_PC *sd;
	sd=script_rid2sd(st);
	pc_resetskill(sd,1);
	return true;
}

/*==========================================
 * Counts total amount of skill points.
 *------------------------------------------*/
BUILDIN(skillpointcount)
{
	TBL_PC *sd;
	sd=script_rid2sd(st);
	script_pushint(st,sd->status.skill_point + pc_resetskill(sd,2));
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN(changebase)
{
	TBL_PC *sd=NULL;
	int vclass;
	
	if( script_hasdata(st,3) )
		sd=map_id2sd(script_getnum(st,3));
	else
		sd=script_rid2sd(st);
	
	if(sd == NULL)
		return true;
	
	vclass = script_getnum(st,2);
	if(vclass == JOB_WEDDING)
	{
		if (!battle_config.wedding_modifydisplay || //Do not show the wedding sprites
			sd->class_&JOBL_BABY //Baby classes screw up when showing wedding sprites. [Skotlex] They don't seem to anymore.
			)
			return true;
	}
	
	if(sd->disguise == -1 && vclass != sd->vd.class_) {
		status_set_viewdata(&sd->bl, vclass);
		//Updated client view. Base, Weapon and Cloth Colors.
		clif->changelook(&sd->bl,LOOK_BASE,sd->vd.class_);
		clif->changelook(&sd->bl,LOOK_WEAPON,sd->status.weapon);
		if (sd->vd.cloth_color)
			clif->changelook(&sd->bl,LOOK_CLOTHES_COLOR,sd->vd.cloth_color);
		clif->skillinfoblock(sd);
	}
	
	return true;
}

/*==========================================
 * Unequip all item and request for a changesex to char-serv
 *------------------------------------------*/
BUILDIN(changesex)
{
	int i;
	TBL_PC *sd = NULL;
	sd = script_rid2sd(st);
	
	pc_resetskill(sd,4);
	// to avoid any problem with equipment and invalid sex, equipment is unequiped.
	for( i=0; i<EQI_MAX; i++ )
		if( sd->equip_index[i] >= 0 ) pc_unequipitem(sd, sd->equip_index[i], 3);
	chrif_changesex(sd);
	return true;
}

/*==========================================
 * Works like 'announce' but outputs in the common chat window
 *------------------------------------------*/
BUILDIN(globalmes)
{
	struct block_list *bl = map_id2bl(st->oid);
	struct npc_data *nd = (struct npc_data *)bl;
	const char *name=NULL,*mes;
	
	mes=script_getstr(st,2);
	if(mes==NULL) return true;
	
	if(script_hasdata(st,3)){	//  npc name to display
		name=script_getstr(st,3);
	} else {
		name=nd->name; //use current npc name
	}
	
	npc_globalmessage(name,mes);	// broadcast  to all players connected
	
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
	int pub = 1;
	const char* title = script_getstr(st, 2);
	int limit = script_getnum(st, 3);
	const char* ev = script_hasdata(st,4) ? script_getstr(st,4) : "";
	int trigger =  script_hasdata(st,5) ? script_getnum(st,5) : limit;
	int zeny =  script_hasdata(st,6) ? script_getnum(st,6) : 0;
	int minLvl =  script_hasdata(st,7) ? script_getnum(st,7) : 1;
	int maxLvl =  script_hasdata(st,8) ? script_getnum(st,8) : MAX_LEVEL;
	
	nd = (struct npc_data *)map_id2bl(st->oid);
	if( nd != NULL )
		chat_createnpcchat(nd, title, limit, pub, trigger, ev, zeny, minLvl, maxLvl);
	
	return true;
}

/// Removes the waiting room of the current or target npc.
///
/// delwaitingroom "<npc_name>";
/// delwaitingroom;
BUILDIN(delwaitingroom)
{
	struct npc_data* nd;
	if( script_hasdata(st,2) )
		nd = npc_name2id(script_getstr(st, 2));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);
	if( nd != NULL )
		chat_deletenpcchat(nd);
	return true;
}

/// Kicks all the players from the waiting room of the current or target npc.
///
/// kickwaitingroomall "<npc_name>";
/// kickwaitingroomall;
BUILDIN(waitingroomkickall)
{
	struct npc_data* nd;
	struct chat_data* cd;
	
	if( script_hasdata(st,2) )
		nd = npc_name2id(script_getstr(st,2));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);
	
	if( nd != NULL && (cd=(struct chat_data *)map_id2bl(nd->chat_id)) != NULL )
		chat_npckickall(cd);
	return true;
}

/// Enables the waiting room event of the current or target npc.
///
/// enablewaitingroomevent "<npc_name>";
/// enablewaitingroomevent;
BUILDIN(enablewaitingroomevent)
{
	struct npc_data* nd;
	struct chat_data* cd;
	
	if( script_hasdata(st,2) )
		nd = npc_name2id(script_getstr(st, 2));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);
	
	if( nd != NULL && (cd=(struct chat_data *)map_id2bl(nd->chat_id)) != NULL )
		chat_enableevent(cd);
	return true;
}

/// Disables the waiting room event of the current or target npc.
///
/// disablewaitingroomevent "<npc_name>";
/// disablewaitingroomevent;
BUILDIN(disablewaitingroomevent)
{
	struct npc_data *nd;
	struct chat_data *cd;
	
	if( script_hasdata(st,2) )
		nd = npc_name2id(script_getstr(st, 2));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);
	
	if( nd != NULL && (cd=(struct chat_data *)map_id2bl(nd->chat_id)) != NULL )
		chat_disableevent(cd);
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
///
/// getwaitingroomstate(<type>,"<npc_name>") -> <info>
/// getwaitingroomstate(<type>) -> <info>
BUILDIN(getwaitingroomstate)
{
	struct npc_data *nd;
	struct chat_data *cd;
	int type;
	
	type = script_getnum(st,2);
	if( script_hasdata(st,3) )
		nd = npc_name2id(script_getstr(st, 3));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);
	
	if( nd == NULL || (cd=(struct chat_data *)map_id2bl(nd->chat_id)) == NULL )
	{
		script_pushint(st, -1);
		return true;
	}
	
	switch(type)
	{
		case 0:  script_pushint(st, cd->users); break;
		case 1:  script_pushint(st, cd->limit); break;
		case 2:  script_pushint(st, cd->trigger&0x7f); break;
		case 3:  script_pushint(st, ((cd->trigger&0x80)!=0)); break;
		case 4:  script_pushstrcopy(st, cd->title); break;
		case 5:  script_pushstrcopy(st, cd->pass); break;
		case 16: script_pushstrcopy(st, cd->npc_event);break;
		case 32: script_pushint(st, (cd->users >= cd->limit)); break;
		case 33: script_pushint(st, (cd->users >= cd->trigger)); break;
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
	int x;
	int y;
	int i;
	int n;
	const char* map_name;
	struct npc_data* nd;
	struct chat_data* cd;
	TBL_PC* sd;
	
	nd = (struct npc_data *)map_id2bl(st->oid);
	if( nd == NULL || (cd=(struct chat_data *)map_id2bl(nd->chat_id)) == NULL )
		return true;
	
	map_name = script_getstr(st,2);
	x = script_getnum(st,3);
	y = script_getnum(st,4);
	n = cd->trigger&0x7f;
	
	if( script_hasdata(st,5) )
		n = script_getnum(st,5);
	
	for( i = 0; i < n && cd->users > 0; i++ )
	{
		sd = cd->usersd[0];
		
		if( strcmp(map_name,"SavePoint") == 0 && map[sd->bl.m].flag.noteleport )
		{// can't teleport on this map
			break;
		}
		
		if( cd->zeny )
		{// fee set
			if( (uint32)sd->status.zeny < cd->zeny )
			{// no zeny to cover set fee
				break;
			}
			pc_payzeny(sd, cd->zeny, LOG_TYPE_NPC, NULL);
		}
		
		mapreg_setreg(reference_uid(add_str("$@warpwaitingpc"), i), sd->bl.id);
		
		if( strcmp(map_name,"Random") == 0 )
			pc_randomwarp(sd,CLR_TELEPORT);
		else if( strcmp(map_name,"SavePoint") == 0 )
			pc_setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, CLR_TELEPORT);
		else
			pc_setpos(sd, mapindex_name2id(map_name), x, y, CLR_OUTSIGHT);
	}
	mapreg_setreg(add_str("$@warpwaitingpcnum"), i);
	return true;
}

/////////////////////////////////////////////////////////////////////
// ...
//

/// Detaches a character from a script.
///
/// @param st Script state to detach the character from.
static void script_detach_rid(struct script_state* st)
{
	if(st->rid)
	{
		script_detach_state(st, false);
		st->rid = 0;
	}
}

/*==========================================
 * Attach sd char id to script and detach current one if any
 *------------------------------------------*/
BUILDIN(attachrid)
{
	int rid = script_getnum(st,2);
	
	if (map_id2sd(rid) != NULL) {
		script_detach_rid(st);
		
		st->rid = rid;
		script_attach_state(st);
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
	script_detach_rid(st);
	return true;
}
/*==========================================
 * Chk if account connected, (and charid from account if specified)
 *------------------------------------------*/
BUILDIN(isloggedin)
{
	TBL_PC* sd = map_id2sd(script_getnum(st,2));
	if (script_hasdata(st,3) && sd &&
		sd->status.char_id != script_getnum(st,3))
		sd = NULL;
	push_val(st->stack,C_INT,sd!=NULL);
	return true;
}


/*==========================================
 *
 *------------------------------------------*/
BUILDIN(setmapflagnosave)
{
	int16 m,x,y;
	unsigned short mapindex;
	const char *str,*str2;
	
	str=script_getstr(st,2);
	str2=script_getstr(st,3);
	x=script_getnum(st,4);
	y=script_getnum(st,5);
	m = map_mapname2mapid(str);
	mapindex = mapindex_name2id(str2);
	
	if(m >= 0 && mapindex) {
		map[m].flag.nosave=1;
		map[m].save.map=mapindex;
		map[m].save.x=x;
		map[m].save.y=y;
	}
	
	return true;
}

BUILDIN(getmapflag)
{
	int16 m,i;
	const char *str;
	
	str=script_getstr(st,2);
	i=script_getnum(st,3);
	
	m = map_mapname2mapid(str);
	if(m >= 0) {
		switch(i) {
			case MF_NOMEMO:				script_pushint(st,map[m].flag.nomemo); break;
			case MF_NOTELEPORT:			script_pushint(st,map[m].flag.noteleport); break;
			case MF_NOSAVE:				script_pushint(st,map[m].flag.nosave); break;
			case MF_NOBRANCH:			script_pushint(st,map[m].flag.nobranch); break;
			case MF_NOPENALTY:			script_pushint(st,map[m].flag.noexppenalty); break;
			case MF_NOZENYPENALTY:		script_pushint(st,map[m].flag.nozenypenalty); break;
			case MF_PVP:				script_pushint(st,map[m].flag.pvp); break;
			case MF_PVP_NOPARTY:		script_pushint(st,map[m].flag.pvp_noparty); break;
			case MF_PVP_NOGUILD:		script_pushint(st,map[m].flag.pvp_noguild); break;
			case MF_GVG:				script_pushint(st,map[m].flag.gvg); break;
			case MF_GVG_NOPARTY:		script_pushint(st,map[m].flag.gvg_noparty); break;
			case MF_NOTRADE:			script_pushint(st,map[m].flag.notrade); break;
			case MF_NOSKILL:			script_pushint(st,map[m].flag.noskill); break;
			case MF_NOWARP:				script_pushint(st,map[m].flag.nowarp); break;
			case MF_PARTYLOCK:			script_pushint(st,map[m].flag.partylock); break;
			case MF_NOICEWALL:			script_pushint(st,map[m].flag.noicewall); break;
			case MF_SNOW:				script_pushint(st,map[m].flag.snow); break;
			case MF_FOG:				script_pushint(st,map[m].flag.fog); break;
			case MF_SAKURA:				script_pushint(st,map[m].flag.sakura); break;
			case MF_LEAVES:				script_pushint(st,map[m].flag.leaves); break;
			case MF_CLOUDS:				script_pushint(st,map[m].flag.clouds); break;
			case MF_CLOUDS2:			script_pushint(st,map[m].flag.clouds2); break;
			case MF_FIREWORKS:			script_pushint(st,map[m].flag.fireworks); break;
			case MF_GVG_CASTLE:			script_pushint(st,map[m].flag.gvg_castle); break;
			case MF_GVG_DUNGEON:		script_pushint(st,map[m].flag.gvg_dungeon); break;
			case MF_NIGHTENABLED:		script_pushint(st,map[m].flag.nightenabled); break;
			case MF_NOBASEEXP:			script_pushint(st,map[m].flag.nobaseexp); break;
			case MF_NOJOBEXP:			script_pushint(st,map[m].flag.nojobexp); break;
			case MF_NOMOBLOOT:			script_pushint(st,map[m].flag.nomobloot); break;
			case MF_NOMVPLOOT:			script_pushint(st,map[m].flag.nomvploot); break;
			case MF_NORETURN:			script_pushint(st,map[m].flag.noreturn); break;
			case MF_NOWARPTO:			script_pushint(st,map[m].flag.nowarpto); break;
			case MF_NIGHTMAREDROP:		script_pushint(st,map[m].flag.pvp_nightmaredrop); break;
			case MF_NOCOMMAND:			script_pushint(st,map[m].nocommand); break;
			case MF_NODROP:				script_pushint(st,map[m].flag.nodrop); break;
			case MF_JEXP:				script_pushint(st,map[m].jexp); break;
			case MF_BEXP:				script_pushint(st,map[m].bexp); break;
			case MF_NOVENDING:			script_pushint(st,map[m].flag.novending); break;
			case MF_LOADEVENT:			script_pushint(st,map[m].flag.loadevent); break;
			case MF_NOCHAT:				script_pushint(st,map[m].flag.nochat); break;
			case MF_NOEXPPENALTY:		script_pushint(st,map[m].flag.noexppenalty ); break;
			case MF_GUILDLOCK:			script_pushint(st,map[m].flag.guildlock); break;
			case MF_TOWN:				script_pushint(st,map[m].flag.town); break;
			case MF_AUTOTRADE:			script_pushint(st,map[m].flag.autotrade); break;
			case MF_ALLOWKS:			script_pushint(st,map[m].flag.allowks); break;
			case MF_MONSTER_NOTELEPORT:	script_pushint(st,map[m].flag.monster_noteleport); break;
			case MF_PVP_NOCALCRANK:		script_pushint(st,map[m].flag.pvp_nocalcrank); break;
			case MF_BATTLEGROUND:		script_pushint(st,map[m].flag.battleground); break;
			case MF_RESET:				script_pushint(st,map[m].flag.reset); break;
		}
	}
	
	return true;
}
/* pvp timer handling */
static int script_mapflag_pvp_sub(struct block_list *bl,va_list ap) {
	TBL_PC* sd = (TBL_PC*)bl;
	if (sd->pvp_timer == INVALID_TIMER) {
		sd->pvp_timer = add_timer(gettick() + 200, pc_calc_pvprank_timer, sd->bl.id, 0);
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
BUILDIN(setmapflag)
{
	int16 m,i;
	const char *str, *val2 = NULL;
	struct script_data* data;
	int val=0;
	
	str=script_getstr(st,2);
	
	i = script_getnum(st, 3);
	
	if(script_hasdata(st,4)){
		data = script_getdata(st,4);
		get_val(st, data);
		
		
		if( data_isstring(data) )
			val2 = script_getstr(st, 4);
		else
			val = script_getnum(st, 4);
		
	}
	m = map_mapname2mapid(str);
	
	if(m >= 0) {
		switch(i) {
			case MF_NOMEMO:				map[m].flag.nomemo = 1; break;
			case MF_NOTELEPORT:			map[m].flag.noteleport = 1; break;
			case MF_NOSAVE:				map[m].flag.nosave = 1; break;
			case MF_NOBRANCH:			map[m].flag.nobranch = 1; break;
			case MF_NOPENALTY:			map[m].flag.noexppenalty = 1; map[m].flag.nozenypenalty = 1; break;
			case MF_NOZENYPENALTY:		map[m].flag.nozenypenalty = 1; break;
			case MF_PVP:
				map[m].flag.pvp = 1;
				if( !battle_config.pk_mode ) {
					map_foreachinmap(script_mapflag_pvp_sub,m,BL_PC);
				}
				break;
			case MF_PVP_NOPARTY:		map[m].flag.pvp_noparty = 1; break;
			case MF_PVP_NOGUILD:		map[m].flag.pvp_noguild = 1; break;
			case MF_GVG: {
				struct block_list bl;
				map[m].flag.gvg = 1;
				clif->map_property_mapall(m, MAPPROPERTY_AGITZONE);
				bl.type = BL_NUL;
				bl.m = m;
				clif->maptypeproperty2(&bl,ALL_SAMEMAP);
			}
				break;
			case MF_GVG_NOPARTY:		map[m].flag.gvg_noparty = 1; break;
			case MF_NOTRADE:			map[m].flag.notrade = 1; break;
			case MF_NOSKILL:			map[m].flag.noskill = 1; break;
			case MF_NOWARP:				map[m].flag.nowarp = 1; break;
			case MF_PARTYLOCK:			map[m].flag.partylock = 1; break;
			case MF_NOICEWALL:			map[m].flag.noicewall = 1; break;
			case MF_SNOW:				map[m].flag.snow = 1; break;
			case MF_FOG:				map[m].flag.fog = 1; break;
			case MF_SAKURA:				map[m].flag.sakura = 1; break;
			case MF_LEAVES:				map[m].flag.leaves = 1; break;
			case MF_CLOUDS:				map[m].flag.clouds = 1; break;
			case MF_CLOUDS2:			map[m].flag.clouds2 = 1; break;
			case MF_FIREWORKS:			map[m].flag.fireworks = 1; break;
			case MF_GVG_CASTLE:			map[m].flag.gvg_castle = 1; break;
			case MF_GVG_DUNGEON:		map[m].flag.gvg_dungeon = 1; break;
			case MF_NIGHTENABLED:		map[m].flag.nightenabled = 1; break;
			case MF_NOBASEEXP:			map[m].flag.nobaseexp = 1; break;
			case MF_NOJOBEXP:			map[m].flag.nojobexp = 1; break;
			case MF_NOMOBLOOT:			map[m].flag.nomobloot = 1; break;
			case MF_NOMVPLOOT:			map[m].flag.nomvploot = 1; break;
			case MF_NORETURN:			map[m].flag.noreturn = 1; break;
			case MF_NOWARPTO:			map[m].flag.nowarpto = 1; break;
			case MF_NIGHTMAREDROP:		map[m].flag.pvp_nightmaredrop = 1; break;
			case MF_ZONE: {
				char zone[6] = "zone\0";
				char empty[1] = "\0";
				char params[MAP_ZONE_MAPFLAG_LENGTH];
				memcpy(params, val2, MAP_ZONE_MAPFLAG_LENGTH);
				npc_parse_mapflag(map[m].name, empty, zone, params, empty, empty, empty);
			}
				break;
			case MF_NOCOMMAND:			map[m].nocommand = (val <= 0) ? 100 : val; break;
			case MF_NODROP:				map[m].flag.nodrop = 1; break;
			case MF_JEXP:				map[m].jexp = (val <= 0) ? 100 : val; break;
			case MF_BEXP:				map[m].bexp = (val <= 0) ? 100 : val; break;
			case MF_NOVENDING:			map[m].flag.novending = 1; break;
			case MF_LOADEVENT:			map[m].flag.loadevent = 1; break;
			case MF_NOCHAT:				map[m].flag.nochat = 1; break;
			case MF_NOEXPPENALTY:		map[m].flag.noexppenalty  = 1; break;
			case MF_GUILDLOCK:			map[m].flag.guildlock = 1; break;
			case MF_TOWN:				map[m].flag.town = 1; break;
			case MF_AUTOTRADE:			map[m].flag.autotrade = 1; break;
			case MF_ALLOWKS:			map[m].flag.allowks = 1; break;
			case MF_MONSTER_NOTELEPORT:	map[m].flag.monster_noteleport = 1; break;
			case MF_PVP_NOCALCRANK:		map[m].flag.pvp_nocalcrank = 1; break;
			case MF_BATTLEGROUND:		map[m].flag.battleground = (val <= 0 || val > 2) ? 1 : val; break;
			case MF_RESET:				map[m].flag.reset = 1; break;
		}
	}
	
	return true;
}

BUILDIN(removemapflag)
{
	int16 m,i;
	const char *str;
	int val=0;
	
	str=script_getstr(st,2);
	i=script_getnum(st,3);
	if(script_hasdata(st,4)){
		val=script_getnum(st,4);
	}
	m = map_mapname2mapid(str);
	if(m >= 0) {
		switch(i) {
			case MF_NOMEMO:				map[m].flag.nomemo = 0; break;
			case MF_NOTELEPORT:			map[m].flag.noteleport = 0; break;
			case MF_NOSAVE:				map[m].flag.nosave = 0; break;
			case MF_NOBRANCH:			map[m].flag.nobranch = 0; break;
			case MF_NOPENALTY:			map[m].flag.noexppenalty = 0; map[m].flag.nozenypenalty = 0; break;
			case MF_NOZENYPENALTY:		map[m].flag.nozenypenalty = 0; break;
			case MF_PVP: {
				struct block_list bl;
				bl.type = BL_NUL;
				bl.m = m;
				map[m].flag.pvp = 0;
				clif->map_property_mapall(m, MAPPROPERTY_NOTHING);
				clif->maptypeproperty2(&bl,ALL_SAMEMAP);
			}
				break;
			case MF_PVP_NOPARTY:		map[m].flag.pvp_noparty = 0; break;
			case MF_PVP_NOGUILD:		map[m].flag.pvp_noguild = 0; break;
			case MF_GVG: {
				struct block_list bl;
				bl.type = BL_NUL;
				bl.m = m;
				map[m].flag.gvg = 0;
				clif->map_property_mapall(m, MAPPROPERTY_NOTHING);
				clif->maptypeproperty2(&bl,ALL_SAMEMAP);
			}
				break;
			case MF_GVG_NOPARTY:		map[m].flag.gvg_noparty = 0; break;
			case MF_NOTRADE:			map[m].flag.notrade = 0; break;
			case MF_NOSKILL:			map[m].flag.noskill = 0; break;
			case MF_NOWARP:				map[m].flag.nowarp = 0; break;
			case MF_PARTYLOCK:			map[m].flag.partylock = 0; break;
			case MF_NOICEWALL:			map[m].flag.noicewall = 0; break;
			case MF_SNOW:				map[m].flag.snow = 0; break;
			case MF_FOG:				map[m].flag.fog = 0; break;
			case MF_SAKURA:				map[m].flag.sakura = 0; break;
			case MF_LEAVES:				map[m].flag.leaves = 0; break;
			case MF_CLOUDS:				map[m].flag.clouds = 0; break;
			case MF_CLOUDS2:			map[m].flag.clouds2 = 0; break;
			case MF_FIREWORKS:			map[m].flag.fireworks = 0; break;
			case MF_GVG_CASTLE:			map[m].flag.gvg_castle = 0; break;
			case MF_GVG_DUNGEON:		map[m].flag.gvg_dungeon = 0; break;
			case MF_NIGHTENABLED:		map[m].flag.nightenabled = 0; break;
			case MF_NOBASEEXP:			map[m].flag.nobaseexp = 0; break;
			case MF_NOJOBEXP:			map[m].flag.nojobexp = 0; break;
			case MF_NOMOBLOOT:			map[m].flag.nomobloot = 0; break;
			case MF_NOMVPLOOT:			map[m].flag.nomvploot = 0; break;
			case MF_NORETURN:			map[m].flag.noreturn = 0; break;
			case MF_NOWARPTO:			map[m].flag.nowarpto = 0; break;
			case MF_NIGHTMAREDROP:		map[m].flag.pvp_nightmaredrop = 0; break;
			case MF_ZONE:
				map_zone_change2(m, map[m].prev_zone);
				break;
			case MF_NOCOMMAND:			map[m].nocommand = 0; break;
			case MF_NODROP:				map[m].flag.nodrop = 0; break;
			case MF_JEXP:				map[m].jexp = 0; break;
			case MF_BEXP:				map[m].bexp = 0; break;
			case MF_NOVENDING:			map[m].flag.novending = 0; break;
			case MF_LOADEVENT:			map[m].flag.loadevent = 0; break;
			case MF_NOCHAT:				map[m].flag.nochat = 0; break;
			case MF_NOEXPPENALTY:		map[m].flag.noexppenalty  = 0; break;
			case MF_GUILDLOCK:			map[m].flag.guildlock = 0; break;
			case MF_TOWN:				map[m].flag.town = 0; break;
			case MF_AUTOTRADE:			map[m].flag.autotrade = 0; break;
			case MF_ALLOWKS:			map[m].flag.allowks = 0; break;
			case MF_MONSTER_NOTELEPORT:	map[m].flag.monster_noteleport = 0; break;
			case MF_PVP_NOCALCRANK:		map[m].flag.pvp_nocalcrank = 0; break;
			case MF_BATTLEGROUND:		map[m].flag.battleground = 0; break;
			case MF_RESET:				map[m].flag.reset = 0; break;
		}
	}
	
	return true;
}

BUILDIN(pvpon)
{
	int16 m;
	const char *str;
	TBL_PC* sd = NULL;
	struct s_mapiterator* iter;
	struct block_list bl;
	
	str = script_getstr(st,2);
	m = map_mapname2mapid(str);
	if( m < 0 || map[m].flag.pvp )
		return true; // nothing to do
	
	map_zone_change2(m, strdb_get(zone_db, MAP_ZONE_PVP_NAME));
	map[m].flag.pvp = 1;
	clif->map_property_mapall(m, MAPPROPERTY_FREEPVPZONE);
	bl.type = BL_NUL;
	bl.m = m;
	clif->maptypeproperty2(&bl,ALL_SAMEMAP);
	
	
	if(battle_config.pk_mode) // disable ranking functions if pk_mode is on [Valaris]
		return true;
	
	iter = mapit_getallusers();
	for( sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); sd = (TBL_PC*)mapit->next(iter) )
	{
		if( sd->bl.m != m || sd->pvp_timer != INVALID_TIMER )
			continue; // not applicable
		
		sd->pvp_timer = add_timer(gettick()+200,pc_calc_pvprank_timer,sd->bl.id,0);
		sd->pvp_rank = 0;
		sd->pvp_lastusers = 0;
		sd->pvp_point = 5;
		sd->pvp_won = 0;
		sd->pvp_lost = 0;
	}
	mapit->free(iter);
	
	return true;
}

static int buildin_pvpoff_sub(struct block_list *bl,va_list ap)
{
	TBL_PC* sd = (TBL_PC*)bl;
	clif->pvpset(sd, 0, 0, 2);
	if (sd->pvp_timer != INVALID_TIMER) {
		delete_timer(sd->pvp_timer, pc_calc_pvprank_timer);
		sd->pvp_timer = INVALID_TIMER;
	}
	return 0;
}

BUILDIN(pvpoff)
{
	int16 m;
	const char *str;
	struct block_list bl;
	
	str=script_getstr(st,2);
	m = map_mapname2mapid(str);
	if(m < 0 || !map[m].flag.pvp)
		return true; //fixed Lupus
	
	map_zone_change2(m, map[m].prev_zone);
	map[m].flag.pvp = 0;
	clif->map_property_mapall(m, MAPPROPERTY_NOTHING);
	bl.type = BL_NUL;
	bl.m = m;
	clif->maptypeproperty2(&bl,ALL_SAMEMAP);
	
	if(battle_config.pk_mode) // disable ranking options if pk_mode is on [Valaris]
		return true;
	
	map_foreachinmap(buildin_pvpoff_sub, m, BL_PC);
	return true;
}

BUILDIN(gvgon)
{
	int16 m;
	const char *str;
	
	str=script_getstr(st,2);
	m = map_mapname2mapid(str);
	if(m >= 0 && !map[m].flag.gvg) {
		struct block_list bl;
		map_zone_change2(m, strdb_get(zone_db, MAP_ZONE_GVG_NAME));
		map[m].flag.gvg = 1;
		clif->map_property_mapall(m, MAPPROPERTY_AGITZONE);
		bl.type = BL_NUL;
		bl.m = m;
		clif->maptypeproperty2(&bl,ALL_SAMEMAP);
	}
	
	return true;
}
BUILDIN(gvgoff)
{
	int16 m;
	const char *str;
	
	str=script_getstr(st,2);
	m = map_mapname2mapid(str);
	if(m >= 0 && map[m].flag.gvg) {
		struct block_list bl;
		map_zone_change2(m, map[m].prev_zone);
		map[m].flag.gvg = 0;
		clif->map_property_mapall(m, MAPPROPERTY_NOTHING);
		bl.type = BL_NUL;
		bl.m = m;
		clif->maptypeproperty2(&bl,ALL_SAMEMAP);
	}
	
	return true;
}
/*==========================================
 *	Shows an emoticon on top of the player/npc
 *	emotion emotion#, <target: 0 - NPC, 1 - PC>, <NPC/PC name>
 *------------------------------------------*/
//Optional second parameter added by [Skotlex]
BUILDIN(emotion)
{
	int type;
	int player=0;
	
	type=script_getnum(st,2);
	if(type < 0 || type > 100)
		return true;
	
	if( script_hasdata(st,3) )
		player=script_getnum(st,3);
	
	if (player) {
		TBL_PC *sd = NULL;
		if( script_hasdata(st,4) )
			sd = map_nick2sd(script_getstr(st,4));
		else
			sd = script_rid2sd(st);
		if (sd)
			clif->emotion(&sd->bl,type);
	} else
		if( script_hasdata(st,4) )
		{
			TBL_NPC *nd = npc_name2id(script_getstr(st,4));
			if(nd)
				clif->emotion(&nd->bl,type);
		}
		else
			clif->emotion(map_id2bl(st->oid),type);
	return true;
}

static int buildin_maprespawnguildid_sub_pc(struct map_session_data* sd, va_list ap)
{
	int16 m=va_arg(ap,int);
	int g_id=va_arg(ap,int);
	int flag=va_arg(ap,int);
	
	if(!sd || sd->bl.m != m)
		return 0;
	if(
	   (sd->status.guild_id == g_id && flag&1) || //Warp out owners
	   (sd->status.guild_id != g_id && flag&2) || //Warp out outsiders
	   (sd->status.guild_id == 0)	// Warp out players not in guild [Valaris]
	   )
		pc_setpos(sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,CLR_TELEPORT);
	return 1;
}

static int buildin_maprespawnguildid_sub_mob(struct block_list *bl,va_list ap)
{
	struct mob_data *md=(struct mob_data *)bl;
	
	if(!md->guardian_data && md->class_ != MOBID_EMPERIUM)
		status_kill(bl);
	
	return 0;
}

BUILDIN(maprespawnguildid)
{
	const char *mapname=script_getstr(st,2);
	int g_id=script_getnum(st,3);
	int flag=script_getnum(st,4);
	
	int16 m=map_mapname2mapid(mapname);
	
	if(m == -1)
		return true;
	
	//Catch ALL players (in case some are 'between maps' on execution time)
	map_foreachpc(buildin_maprespawnguildid_sub_pc,m,g_id,flag);
	if (flag&4) //Remove script mobs.
		map_foreachinmap(buildin_maprespawnguildid_sub_mob,m,BL_MOB);
	return true;
}

BUILDIN(agitstart)
{
	if(agit_flag==1) return true;      // Agit already Start.
	agit_flag=1;
	guild->agit_start();
	return true;
}

BUILDIN(agitend)
{
	if(agit_flag==0) return true;      // Agit already End.
	agit_flag=0;
	guild->agit_end();
	return true;
}

BUILDIN(agitstart2)
{
	if(agit2_flag==1) return true;      // Agit2 already Start.
	agit2_flag=1;
	guild->agit2_start();
	return true;
}

BUILDIN(agitend2)
{
	if(agit2_flag==0) return true;      // Agit2 already End.
	agit2_flag=0;
	guild->agit2_end();
	return true;
}

/*==========================================
 * Returns whether woe is on or off.	// choice script
 *------------------------------------------*/
BUILDIN(agitcheck)
{
	script_pushint(st,agit_flag);
	return true;
}

/*==========================================
 * Returns whether woese is on or off.	// choice script
 *------------------------------------------*/
BUILDIN(agitcheck2)
{
	script_pushint(st,agit2_flag);
	return true;
}

/// Sets the guild_id of this npc.
///
/// flagemblem <guild_id>;
BUILDIN(flagemblem)
{
	TBL_NPC* nd;
	int g_id = script_getnum(st,2);
	
	if(g_id < 0) return true;
	
	nd = (TBL_NPC*)map_id2nd(st->oid);
	if( nd == NULL ) {
		ShowError("script:flagemblem: npc %d not found\n", st->oid);
	} else if( nd->subtype != SCRIPT ) {
		ShowError("script:flagemblem: unexpected subtype %d for npc %d '%s'\n", nd->subtype, st->oid, nd->exname);
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
	const char* mapname = mapindex_getmapname(script_getstr(st,2),NULL);
	struct guild_castle* gc = guild->mapname2gc(mapname);
	const char* name = (gc) ? gc->castle_name : "";
	script_pushstrcopy(st,name);
	return true;
}

BUILDIN(getcastledata)
{
	const char *mapname = mapindex_getmapname(script_getstr(st,2),NULL);
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
	const char *mapname = mapindex_getmapname(script_getstr(st,2),NULL);
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
	
	if( script_hasdata(st,3) ){
		event=script_getstr(st,3);
		check_event(st, event);
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
	TBL_PC *sd;
	int count;
	
	num=script_getnum(st,2);
	sd=script_rid2sd(st);
	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
	
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
BUILDIN(successremovecards) {
	int i=-1,j,c,cardflag=0;
	
	TBL_PC* sd = script_rid2sd(st);
	int num = script_getnum(st,2);
	
	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
	
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
			
			if((flag=pc_additem(sd,&item_tmp,1,LOG_TYPE_SCRIPT))){	// get back the cart in inventory
				clif->additem(sd,0,0,flag);
				map_addflooritem(&item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
			}
		}
	}
	
	if(cardflag == 1) {//if card was remove remplace item with no card
		int flag;
		struct item item_tmp;
		memset(&item_tmp,0,sizeof(item_tmp));
		
		item_tmp.nameid      = sd->status.inventory[i].nameid;
		item_tmp.identify    = 1;
		item_tmp.refine      = sd->status.inventory[i].refine;
		item_tmp.attribute   = sd->status.inventory[i].attribute;
		item_tmp.expire_time = sd->status.inventory[i].expire_time;
		
		for (j = sd->inventory_data[i]->slot; j < MAX_SLOTS; j++)
			item_tmp.card[j]=sd->status.inventory[i].card[j];
		
		pc_delitem(sd,i,1,0,3,LOG_TYPE_SCRIPT);
		if((flag=pc_additem(sd,&item_tmp,1,LOG_TYPE_SCRIPT))){	//chk if can be spawn in inventory otherwise put on floor
			clif->additem(sd,0,0,flag);
			map_addflooritem(&item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
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
BUILDIN(failedremovecards) {
	int i=-1,j,c,cardflag=0;
	
	TBL_PC* sd = script_rid2sd(st);
	int num = script_getnum(st,2);
	int typefail = script_getnum(st,3);
	
	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
	
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
				
				if((flag=pc_additem(sd,&item_tmp,1,LOG_TYPE_SCRIPT))){
					clif->additem(sd,0,0,flag);
					map_addflooritem(&item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
				}
			}
		}
	}
	
	if(cardflag == 1) {
		if(typefail == 0 || typefail == 2){	// destroy the item
			pc_delitem(sd,i,1,0,2,LOG_TYPE_SCRIPT);
		}
		if(typefail == 1){	// destroy the card
			int flag;
			struct item item_tmp;
			
			memset(&item_tmp,0,sizeof(item_tmp));
			
			item_tmp.nameid      = sd->status.inventory[i].nameid;
			item_tmp.identify    = 1;
			item_tmp.refine      = sd->status.inventory[i].refine;
			item_tmp.attribute   = sd->status.inventory[i].attribute;
			item_tmp.expire_time = sd->status.inventory[i].expire_time;
			
			for (j = sd->inventory_data[i]->slot; j < MAX_SLOTS; j++)
				item_tmp.card[j]=sd->status.inventory[i].card[j];
			
			pc_delitem(sd,i,1,0,2,LOG_TYPE_SCRIPT);
			
			if((flag=pc_additem(sd,&item_tmp,1,LOG_TYPE_SCRIPT))){
				clif->additem(sd,0,0,flag);
				map_addflooritem(&item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
			}
		}
		clif->misceffect(&sd->bl,2);
	}
	
	return true;
}

/* ================================================================
 * mapwarp "<from map>","<to map>",<x>,<y>,<type>,<ID for Type>;
 * type: 0=everyone, 1=guild, 2=party;	[Reddozen]
 * improved by [Lance]
 * ================================================================*/
BUILDIN(mapwarp)	// Added by RoVeRT
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
	if(script_hasdata(st,7)){
		check_val=script_getnum(st,6);
		check_ID=script_getnum(st,7);
	}
	
	if((m=map_mapname2mapid(mapname))< 0)
		return true;
	
	if(!(index=mapindex_name2id(str)))
		return true;
	
	switch(check_val){
		case 1:
			g = guild->search(check_ID);
			if (g){
				for( i=0; i < g->max_member; i++)
				{
					if(g->member[i].sd && g->member[i].sd->bl.m==m){
						pc_setpos(g->member[i].sd,index,x,y,CLR_TELEPORT);
					}
				}
			}
			break;
		case 2:
			p = party_search(check_ID);
			if(p){
				for(i=0;i<MAX_PARTY; i++){
					if(p->data[i].sd && p->data[i].sd->bl.m == m){
						pc_setpos(p->data[i].sd,index,x,y,CLR_TELEPORT);
					}
				}
			}
			break;
		default:
			map_foreachinmap(buildin_areawarp_sub,m,BL_PC,index,x,y,0,0);
			break;
	}
	
	return true;
}

static int buildin_mobcount_sub(struct block_list *bl,va_list ap)	// Added by RoVeRT
{
	char *event=va_arg(ap,char *);
	struct mob_data *md = ((struct mob_data *)bl);
	if( md->status.hp > 0 && (!event || strcmp(event,md->npc_event) == 0) )
		return 1;
	return 0;
}

BUILDIN(mobcount)	// Added by RoVeRT
{
	const char *mapname,*event;
	int16 m;
	mapname=script_getstr(st,2);
	event=script_getstr(st,3);
	
	if( strcmp(event, "all") == 0 )
		event = NULL;
	else
		check_event(st, event);
	
	if( strcmp(mapname, "this") == 0 ) {
		struct map_session_data *sd = script_rid2sd(st);
		if( sd )
			m = sd->bl.m;
		else {
			script_pushint(st,-1);
			return true;
		}
	}
	else if( (m = map_mapname2mapid(mapname)) < 0 ) {
		script_pushint(st,-1);
		return true;
	}
	
	if( map[m].flag.src4instance && map[m].instance_id >= 0 && st->instance_id >= 0 && (m = instance->mapid2imapid(m, st->instance_id)) < 0 ) {
		script_pushint(st,-1);
		return true;
	}
	
	script_pushint(st,map_foreachinmap(buildin_mobcount_sub, m, BL_MOB, event));
	
	return true;
}

BUILDIN(marriage)
{
	const char *partner=script_getstr(st,2);
	TBL_PC *sd=script_rid2sd(st);
	TBL_PC *p_sd=map_nick2sd(partner);
	
	if(sd==NULL || p_sd==NULL || pc_marriage(sd,p_sd) < 0){
		script_pushint(st,0);
		return true;
	}
	script_pushint(st,1);
	return true;
}
BUILDIN(wedding_effect)
{
	TBL_PC *sd=script_rid2sd(st);
	struct block_list *bl;
	
	if(sd==NULL) {
		bl=map_id2bl(st->oid);
	} else
		bl=&sd->bl;
	clif->wedding_effect(bl);
	return true;
}
BUILDIN(divorce)
{
	TBL_PC *sd=script_rid2sd(st);
	if(sd==NULL || pc_divorce(sd) < 0){
		script_pushint(st,0);
		return true;
	}
	script_pushint(st,1);
	return true;
}

BUILDIN(ispartneron)
{
	TBL_PC *sd=script_rid2sd(st);
	
	if(sd==NULL || !pc_ismarried(sd) ||
	   map_charid2sd(sd->status.partner_id) == NULL) {
		script_pushint(st,0);
		return true;
	}
	
	script_pushint(st,1);
	return true;
}

BUILDIN(getpartnerid)
{
    TBL_PC *sd=script_rid2sd(st);
    if (sd == NULL) {
        script_pushint(st,0);
        return true;
    }
	
    script_pushint(st,sd->status.partner_id);
    return true;
}

BUILDIN(getchildid)
{
    TBL_PC *sd=script_rid2sd(st);
    if (sd == NULL) {
        script_pushint(st,0);
        return true;
    }
	
    script_pushint(st,sd->status.child);
    return true;
}

BUILDIN(getmotherid)
{
    TBL_PC *sd=script_rid2sd(st);
    if (sd == NULL) {
        script_pushint(st,0);
        return true;
    }
	
    script_pushint(st,sd->status.mother);
    return true;
}

BUILDIN(getfatherid)
{
    TBL_PC *sd=script_rid2sd(st);
    if (sd == NULL) {
        script_pushint(st,0);
        return true;
    }
	
    script_pushint(st,sd->status.father);
    return true;
}

BUILDIN(warppartner)
{
	int x,y;
	unsigned short mapindex;
	const char *str;
	TBL_PC *sd=script_rid2sd(st);
	TBL_PC *p_sd=NULL;
	
	if(sd==NULL || !pc_ismarried(sd) ||
	   (p_sd=map_charid2sd(sd->status.partner_id)) == NULL) {
		script_pushint(st,0);
		return true;
	}
	
	str=script_getstr(st,2);
	x=script_getnum(st,3);
	y=script_getnum(st,4);
	
	mapindex = mapindex_name2id(str);
	if (mapindex) {
		pc_setpos(p_sd,mapindex,x,y,CLR_OUTSIGHT);
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
	
	if(!mobdb_checkid(class_))
	{
		if (num < 3) //requested a string
			script_pushconststr(st,"");
		else
			script_pushint(st,0);
		return true;
	}
	
	switch (num) {
		case 1: script_pushstrcopy(st,mob_db(class_)->name); break;
		case 2: script_pushstrcopy(st,mob_db(class_)->jname); break;
		case 3: script_pushint(st,mob_db(class_)->lv); break;
		case 4: script_pushint(st,mob_db(class_)->status.max_hp); break;
		case 5: script_pushint(st,mob_db(class_)->status.max_sp); break;
		case 6: script_pushint(st,mob_db(class_)->base_exp); break;
		case 7: script_pushint(st,mob_db(class_)->job_exp); break;
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
BUILDIN(guardian)
{
	int class_=0,x=0,y=0,guardian=0;
	const char *str,*map,*evt="";
	struct script_data *data;
	bool has_index = false;
	
	map	  =script_getstr(st,2);
	x	  =script_getnum(st,3);
	y	  =script_getnum(st,4);
	str	  =script_getstr(st,5);
	class_=script_getnum(st,6);
	
	if( script_hasdata(st,8) )
	{// "<event label>",<guardian index>
		evt=script_getstr(st,7);
		guardian=script_getnum(st,8);
		has_index = true;
	} else if( script_hasdata(st,7) ){
		data=script_getdata(st,7);
		get_val(st,data);
		if( data_isstring(data) )
		{// "<event label>"
			evt=script_getstr(st,7);
		} else if( data_isint(data) )
		{// <guardian index>
			guardian=script_getnum(st,7);
			has_index = true;
		} else {
			ShowError("script:guardian: invalid data type for argument #6 (from 1)\n");
			script_reportdata(data);
			return false;
		}
	}
	
	check_event(st, evt);
	script_pushint(st, mob_spawn_guardian(map,x,y,str,class_,evt,guardian,has_index));
	
	return true;
}
/*==========================================
 * Invisible Walls [Zephyrus]
 *------------------------------------------*/
BUILDIN(setwall)
{
	const char *map, *name;
	int x, y, m, size, dir;
	bool shootable;
	
	map = script_getstr(st,2);
	x = script_getnum(st,3);
	y = script_getnum(st,4);
	size = script_getnum(st,5);
	dir = script_getnum(st,6);
	shootable = script_getnum(st,7);
	name = script_getstr(st,8);
	
	if( (m = map_mapname2mapid(map)) < 0 )
		return true; // Invalid Map
	
	map_iwall_set(m, x, y, size, dir, shootable, name);
	return true;
}
BUILDIN(delwall)
{
	const char *name = script_getstr(st,2);
	map_iwall_remove(name);
	
	return true;
}

/// Retrieves various information about the specified guardian.
///
/// guardianinfo("<map_name>", <index>, <type>) -> <value>
/// type: 0 - whether it is deployed or not
///       1 - maximum hp
///       2 - current hp
///
BUILDIN(guardianinfo)
{
	const char* mapname = mapindex_getmapname(script_getstr(st,2),NULL);
	int id = script_getnum(st,3);
	int type = script_getnum(st,4);
	
	struct guild_castle* gc = guild->mapname2gc(mapname);
	struct mob_data* gd;
	
	if( gc == NULL || id < 0 || id >= MAX_GUARDIANS )
	{
		script_pushint(st,-1);
		return true;
	}
	
	if( type == 0 )
		script_pushint(st, gc->guardian[id].visible);
	else
		if( !gc->guardian[id].visible )
			script_pushint(st,-1);
		else
			if( (gd = map_id2md(gc->guardian[id].id)) == NULL )
				script_pushint(st,-1);
			else
			{
				if     ( type == 1 ) script_pushint(st,gd->status.max_hp);
				else if( type == 2 ) script_pushint(st,gd->status.hp);
				else
					script_pushint(st,-1);
			}
	
	return true;
}

/*==========================================
 * Get the item name by item_id or null
 *------------------------------------------*/
BUILDIN(getitemname)
{
	int item_id=0;
	struct item_data *i_data;
	char *item_name;
	struct script_data *data;
	
	data=script_getdata(st,2);
	get_val(st,data);
	
	if( data_isstring(data) ){
		const char *name=script->conv_str(st,data);
		struct item_data *item_data = itemdb_searchname(name);
		if( item_data )
			item_id=item_data->nameid;
	}else
		item_id=script->conv_num(st,data);
	
	i_data = itemdb_exists(item_id);
	if (i_data == NULL)
	{
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
	
	i_data = itemdb_exists(item_id);
	
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
 getiteminfo(itemID,n), where n
 0 value_buy;
 1 value_sell;
 2 type;
 3 maxchance = Max drop chance of this item e.g. 1 = 0.01% , etc..
 if = 0, then monsters don't drop it at all (rare or a quest item)
 if = -1, then this item is sold in NPC shops only
 4 sex;
 5 equip;
 6 weight;
 7 atk;
 8 def;
 9 range;
 10 slot;
 11 look;
 12 elv;
 13 wlv;
 14 view id
 *------------------------------------------*/
BUILDIN(getiteminfo)
{
	int item_id,n;
	int *item_arr;
	struct item_data *i_data;
	
	item_id	= script_getnum(st,2);
	n	= script_getnum(st,3);
	i_data = itemdb_exists(item_id);
	
	if (i_data && n>=0 && n<=14) {
		item_arr = (int*)&i_data->value_buy;
		script_pushint(st,item_arr[n]);
	} else
		script_pushint(st,-1);
	return true;
}

/*==========================================
 * Set some values of an item [Lupus]
 * Price, Weight, etc...
 setiteminfo(itemID,n,Value), where n
 0 value_buy;
 1 value_sell;
 2 type;
 3 maxchance = Max drop chance of this item e.g. 1 = 0.01% , etc..
 if = 0, then monsters don't drop it at all (rare or a quest item)
 if = -1, then this item is sold in NPC shops only
 4 sex;
 5 equip;
 6 weight;
 7 atk;
 8 def;
 9 range;
 10 slot;
 11 look;
 12 elv;
 13 wlv;
 14 view id
 * Returns Value or -1 if the wrong field's been set
 *------------------------------------------*/
BUILDIN(setiteminfo)
{
	int item_id,n,value;
	int *item_arr;
	struct item_data *i_data;
	
	item_id	= script_getnum(st,2);
	n	= script_getnum(st,3);
	value	= script_getnum(st,4);
	i_data = itemdb_exists(item_id);
	
	if (i_data && n>=0 && n<=14) {
		item_arr = (int*)&i_data->value_buy;
		item_arr[n] = value;
		script_pushint(st,value);
	} else
		script_pushint(st,-1);
	return true;
}

/*==========================================
 * Returns value from equipped item slot n [Lupus]
 getequipcardid(num,slot)
 where
 num = eqip position slot
 slot = 0,1,2,3 (Card Slot N)
 
 This func returns CARD ID, 255,254,-255 (for card 0, if the item is produced)
 it's useful when you want to check item cards or if it's signed
 Useful for such quests as "Sign this refined item with players name" etc
 Hat[0] +4 -> Player's Hat[0] +4
 *------------------------------------------*/
BUILDIN(getequipcardid)
{
	int i=-1,num,slot;
	TBL_PC *sd;
	
	num=script_getnum(st,2);
	slot=script_getnum(st,3);
	sd=script_rid2sd(st);
	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
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
	
	TBL_PC *sd=script_rid2sd(st);
	
	if(sd==NULL || sd->pd==NULL)
		return true;
	
	pd=sd->pd;
	if (pd->bonus)
	{ //Clear previous bonus
		if (pd->bonus->timer != INVALID_TIMER)
			delete_timer(pd->bonus->timer, pet_skill_bonus_timer);
	} else //init
		pd->bonus = (struct pet_bonus *) aMalloc(sizeof(struct pet_bonus));
	
	pd->bonus->type=script_getnum(st,2);
	pd->bonus->val=script_getnum(st,3);
	pd->bonus->duration=script_getnum(st,4);
	pd->bonus->delay=script_getnum(st,5);
	
	if (pd->state.skillbonus == 1)
		pd->state.skillbonus=0;	// waiting state
	
	// wait for timer to start
	if (battle_config.pet_equip_required && pd->pet.equip == 0)
		pd->bonus->timer = INVALID_TIMER;
	else
		pd->bonus->timer = add_timer(gettick()+pd->bonus->delay*1000, pet_skill_bonus_timer, sd->bl.id, 0);
	
	return true;
}

/*==========================================
 * pet looting [Valaris] //Rewritten by [Skotlex]
 *------------------------------------------*/
BUILDIN(petloot)
{
	int max;
	struct pet_data *pd;
	TBL_PC *sd=script_rid2sd(st);
	
	if(sd==NULL || sd->pd==NULL)
		return true;
	
	max=script_getnum(st,2);
	
	if(max < 1)
		max = 1;	//Let'em loot at least 1 item.
	else if (max > MAX_PETLOOT_SIZE)
		max = MAX_PETLOOT_SIZE;
	
	pd = sd->pd;
	if (pd->loot != NULL)
	{	//Release whatever was there already and reallocate memory
		pet_lootitem_drop(pd, pd->msd);
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
	TBL_PC *sd=script_rid2sd(st);
	char card_var[NAME_LENGTH];
	
	int i,j=0,k;
	if(!sd) return true;
	for(i=0;i<MAX_INVENTORY;i++){
		if(sd->status.inventory[i].nameid > 0 && sd->status.inventory[i].amount > 0){
			pc_setreg(sd,reference_uid(add_str("@inventorylist_id"), j),sd->status.inventory[i].nameid);
			pc_setreg(sd,reference_uid(add_str("@inventorylist_amount"), j),sd->status.inventory[i].amount);
			pc_setreg(sd,reference_uid(add_str("@inventorylist_equip"), j),sd->status.inventory[i].equip);
			pc_setreg(sd,reference_uid(add_str("@inventorylist_refine"), j),sd->status.inventory[i].refine);
			pc_setreg(sd,reference_uid(add_str("@inventorylist_identify"), j),sd->status.inventory[i].identify);
			pc_setreg(sd,reference_uid(add_str("@inventorylist_attribute"), j),sd->status.inventory[i].attribute);
			for (k = 0; k < MAX_SLOTS; k++)
			{
				sprintf(card_var, "@inventorylist_card%d",k+1);
				pc_setreg(sd,reference_uid(add_str(card_var), j),sd->status.inventory[i].card[k]);
			}
			pc_setreg(sd,reference_uid(add_str("@inventorylist_expire"), j),sd->status.inventory[i].expire_time);
			j++;
		}
	}
	pc_setreg(sd,add_str("@inventorylist_count"),j);
	return true;
}

BUILDIN(getskilllist)
{
	TBL_PC *sd=script_rid2sd(st);
	int i,j=0;
	if(!sd) return true;
	for(i=0;i<MAX_SKILL;i++){
		if(sd->status.skill[i].id > 0 && sd->status.skill[i].lv > 0){
			pc_setreg(sd,reference_uid(add_str("@skilllist_id"), j),sd->status.skill[i].id);
			pc_setreg(sd,reference_uid(add_str("@skilllist_lv"), j),sd->status.skill[i].lv);
			pc_setreg(sd,reference_uid(add_str("@skilllist_flag"), j),sd->status.skill[i].flag);
			j++;
		}
	}
	pc_setreg(sd,add_str("@skilllist_count"),j);
	return true;
}

BUILDIN(clearitem)
{
	TBL_PC *sd=script_rid2sd(st);
	int i;
	if(sd==NULL) return true;
	for (i=0; i<MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].amount) {
			pc_delitem(sd, i, sd->status.inventory[i].amount, 0, 0, LOG_TYPE_SCRIPT);
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
	TBL_PC* sd = script_rid2sd(st);
	if (sd == NULL) return true;
	
	id = script_getnum(st,2);
	
	if (mobdb_checkid(id) || npcdb_checkid(id)) {
		pc_disguise(sd, id);
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
	TBL_PC* sd = script_rid2sd(st);
	if (sd == NULL) return true;
	
	if (sd->disguise != -1) {
		pc_disguise(sd, -1);
		script_pushint(st,0);
	} else {
		script_pushint(st,1);
	}
	return true;
}

/*==========================================
 * Transform a bl to another _class,
 * @type unused
 *------------------------------------------*/
BUILDIN(classchange)
{
	int _class,type;
	struct block_list *bl=map_id2bl(st->oid);
	
	if(bl==NULL) return true;
	
	_class=script_getnum(st,2);
	type=script_getnum(st,3);
	clif->class_change(bl,_class,type);
	return true;
}

/*==========================================
 * Display an effect
 *------------------------------------------*/
BUILDIN(misceffect)
{
	int type;
	
	type=script_getnum(st,2);
	if(st->oid && st->oid != fake_nd->bl.id) {
		struct block_list *bl = map_id2bl(st->oid);
		if (bl)
			clif->specialeffect(bl,type,AREA);
	} else{
		TBL_PC *sd=script_rid2sd(st);
		if(sd)
			clif->specialeffect(&sd->bl,type,AREA);
	}
	return true;
}
/*==========================================
 * Play a BGM on a single client [Rikter/Yommy]
 *------------------------------------------*/
BUILDIN(playBGM)
{
	const char* name;
	struct map_session_data* sd;
	
	if( ( sd = script_rid2sd(st) ) != NULL )
	{
		name = script_getstr(st,2);
		
		clif->playBGM(sd, name);
	}
	
	return true;
}

static int playBGM_sub(struct block_list* bl,va_list ap)
{
	const char* name = va_arg(ap,const char*);
	
	clif->playBGM(BL_CAST(BL_PC, bl), name);
	
	return 0;
}

static int playBGM_foreachpc_sub(struct map_session_data* sd, va_list args)
{
	const char* name = va_arg(args, const char*);
	
	clif->playBGM(sd, name);
	return 0;
}

/*==========================================
 * Play a BGM on multiple client [Rikter/Yommy]
 *------------------------------------------*/
BUILDIN(playBGMall)
{
	const char* name;
	
	name = script_getstr(st,2);
	
	if( script_hasdata(st,7) )
	{// specified part of map
		const char* map = script_getstr(st,3);
		int x0 = script_getnum(st,4);
		int y0 = script_getnum(st,5);
		int x1 = script_getnum(st,6);
		int y1 = script_getnum(st,7);
		
		map_foreachinarea(playBGM_sub, map_mapname2mapid(map), x0, y0, x1, y1, BL_PC, name);
	}
	else if( script_hasdata(st,3) )
	{// entire map
		const char* map = script_getstr(st,3);
		
		map_foreachinmap(playBGM_sub, map_mapname2mapid(map), BL_PC, name);
	}
	else
	{// entire server
		map_foreachpc(&playBGM_foreachpc_sub, name);
	}
	
	return true;
}

/*==========================================
 * Play a .wav sound for sd
 *------------------------------------------*/
BUILDIN(soundeffect)
{
	TBL_PC* sd = script_rid2sd(st);
	const char* name = script_getstr(st,2);
	int type = script_getnum(st,3);
	
	if(sd)
	{
		clif->soundeffect(sd,&sd->bl,name,type);
	}
	return true;
}

int soundeffect_sub(struct block_list* bl,va_list ap)
{
	char* name = va_arg(ap,char*);
	int type = va_arg(ap,int);
	
	clif->soundeffect((TBL_PC *)bl, bl, name, type);
	
	return true;
}

/*==========================================
 * Play a sound effect (.wav) on multiple clients
 * soundeffectall "<filepath>",<type>{,"<map name>"}{,<x0>,<y0>,<x1>,<y1>};
 *------------------------------------------*/
BUILDIN(soundeffectall)
{
	struct block_list* bl;
	const char* name;
	int type;
	
	bl = (st->rid) ? &(script_rid2sd(st)->bl) : map_id2bl(st->oid);
	if (!bl)
		return true;
	
	name = script_getstr(st,2);
	type = script_getnum(st,3);
	
	//FIXME: enumerating map squares (map_foreach) is slower than enumerating the list of online players (map_foreachpc?) [ultramage]
	
	if(!script_hasdata(st,4))
	{	// area around
		clif->soundeffectall(bl, name, type, AREA);
	}
	else
		if(!script_hasdata(st,5))
		{	// entire map
			const char* map = script_getstr(st,4);
			map_foreachinmap(soundeffect_sub, map_mapname2mapid(map), BL_PC, name, type);
		}
		else
			if(script_hasdata(st,8))
			{	// specified part of map
				const char* map = script_getstr(st,4);
				int x0 = script_getnum(st,5);
				int y0 = script_getnum(st,6);
				int x1 = script_getnum(st,7);
				int y1 = script_getnum(st,8);
				map_foreachinarea(soundeffect_sub, map_mapname2mapid(map), x0, y0, x1, y1, BL_PC, name, type);
			}
			else
			{
				ShowError("buildin_soundeffectall: insufficient arguments for specific area broadcast.\n");
			}
	
	return true;
}
/*==========================================
 * pet status recovery [Valaris] / Rewritten by [Skotlex]
 *------------------------------------------*/
BUILDIN(petrecovery)
{
	struct pet_data *pd;
	TBL_PC *sd=script_rid2sd(st);
	
	if(sd==NULL || sd->pd==NULL)
		return true;
	
	pd=sd->pd;
	
	if (pd->recovery)
	{ //Halt previous bonus
		if (pd->recovery->timer != INVALID_TIMER)
			delete_timer(pd->recovery->timer, pet_recovery_timer);
	} else //Init
		pd->recovery = (struct pet_recovery *)aMalloc(sizeof(struct pet_recovery));
	
	pd->recovery->type = (sc_type)script_getnum(st,2);
	pd->recovery->delay = script_getnum(st,3);
	pd->recovery->timer = INVALID_TIMER;
	
	return true;
}

/*==========================================
 * pet healing [Valaris] //Rewritten by [Skotlex]
 *------------------------------------------*/
BUILDIN(petheal)
{
	struct pet_data *pd;
	TBL_PC *sd=script_rid2sd(st);
	
	if(sd==NULL || sd->pd==NULL)
		return true;
	
	pd=sd->pd;
	if (pd->s_skill)
	{ //Clear previous skill
		if (pd->s_skill->timer != INVALID_TIMER)
		{
			if (pd->s_skill->id)
				delete_timer(pd->s_skill->timer, pet_skill_support_timer);
			else
				delete_timer(pd->s_skill->timer, pet_heal_timer);
		}
	} else //init memory
		pd->s_skill = (struct pet_skill_support *) aMalloc(sizeof(struct pet_skill_support));
	
	pd->s_skill->id=0; //This id identifies that it IS petheal rather than pet_skillsupport
	//Use the lv as the amount to heal
	pd->s_skill->lv=script_getnum(st,2);
	pd->s_skill->delay=script_getnum(st,3);
	pd->s_skill->hp=script_getnum(st,4);
	pd->s_skill->sp=script_getnum(st,5);
	
	//Use delay as initial offset to avoid skill/heal exploits
	if (battle_config.pet_equip_required && pd->pet.equip == 0)
		pd->s_skill->timer = INVALID_TIMER;
	else
		pd->s_skill->timer = add_timer(gettick()+pd->s_skill->delay*1000,pet_heal_timer,sd->bl.id,0);
	
	return true;
}

/*==========================================
 * pet attack skills [Valaris] //Rewritten by [Skotlex]
 *------------------------------------------*/
/// petskillattack <skill id>,<level>,<rate>,<bonusrate>
/// petskillattack "<skill name>",<level>,<rate>,<bonusrate>
BUILDIN(petskillattack)
{
	struct pet_data *pd;
	TBL_PC *sd=script_rid2sd(st);
	
	if(sd==NULL || sd->pd==NULL)
		return true;
	
	pd=sd->pd;
	if (pd->a_skill == NULL)
		pd->a_skill = (struct pet_skill_attack *)aMalloc(sizeof(struct pet_skill_attack));
	
	pd->a_skill->id=( script_isstring(st,2) ? skill->name2id(script_getstr(st,2)) : script_getnum(st,2) );
	pd->a_skill->lv=script_getnum(st,3);
	pd->a_skill->div_ = 0;
	pd->a_skill->rate=script_getnum(st,4);
	pd->a_skill->bonusrate=script_getnum(st,5);
	
	return true;
}

/*==========================================
 * pet attack skills [Valaris]
 *------------------------------------------*/
/// petskillattack2 <skill id>,<level>,<div>,<rate>,<bonusrate>
/// petskillattack2 "<skill name>",<level>,<div>,<rate>,<bonusrate>
BUILDIN(petskillattack2)
{
	struct pet_data *pd;
	TBL_PC *sd=script_rid2sd(st);
	
	if(sd==NULL || sd->pd==NULL)
		return true;
	
	pd=sd->pd;
	if (pd->a_skill == NULL)
		pd->a_skill = (struct pet_skill_attack *)aMalloc(sizeof(struct pet_skill_attack));
	
	pd->a_skill->id=( script_isstring(st,2) ? skill->name2id(script_getstr(st,2)) : script_getnum(st,2) );
	pd->a_skill->lv=script_getnum(st,3);
	pd->a_skill->div_ = script_getnum(st,4);
	pd->a_skill->rate=script_getnum(st,5);
	pd->a_skill->bonusrate=script_getnum(st,6);
	
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
	TBL_PC *sd=script_rid2sd(st);
	
	if(sd==NULL || sd->pd==NULL)
		return true;
	
	pd=sd->pd;
	if (pd->s_skill)
	{ //Clear previous skill
		if (pd->s_skill->timer != INVALID_TIMER)
		{
			if (pd->s_skill->id)
				delete_timer(pd->s_skill->timer, pet_skill_support_timer);
			else
				delete_timer(pd->s_skill->timer, pet_heal_timer);
		}
	} else //init memory
		pd->s_skill = (struct pet_skill_support *) aMalloc(sizeof(struct pet_skill_support));
	
	pd->s_skill->id=( script_isstring(st,2) ? skill->name2id(script_getstr(st,2)) : script_getnum(st,2) );
	pd->s_skill->lv=script_getnum(st,3);
	pd->s_skill->delay=script_getnum(st,4);
	pd->s_skill->hp=script_getnum(st,5);
	pd->s_skill->sp=script_getnum(st,6);
	
	//Use delay as initial offset to avoid skill/heal exploits
	if (battle_config.pet_equip_required && pd->pet.equip == 0)
		pd->s_skill->timer = INVALID_TIMER;
	else
		pd->s_skill->timer = add_timer(gettick()+pd->s_skill->delay*1000,pet_skill_support_timer,sd->bl.id,0);
	
	return true;
}

/*==========================================
 * Scripted skill effects [Celest]
 *------------------------------------------*/
/// skilleffect <skill id>,<level>
/// skilleffect "<skill name>",<level>
BUILDIN(skilleffect)
{
	TBL_PC *sd;
	
	uint16 skill_id=( script_isstring(st,2) ? skill->name2id(script_getstr(st,2)) : script_getnum(st,2) );
	uint16 skill_lv=script_getnum(st,3);
	sd=script_rid2sd(st);
	
	clif->skill_nodamage(&sd->bl,&sd->bl,skill_id,skill_lv,1);
	
	return true;
}

/*==========================================
 * NPC skill effects [Valaris]
 *------------------------------------------*/
/// npcskilleffect <skill id>,<level>,<x>,<y>
/// npcskilleffect "<skill name>",<level>,<x>,<y>
BUILDIN(npcskilleffect)
{
	struct block_list *bl= map_id2bl(st->oid);
	
	uint16 skill_id=( script_isstring(st,2) ? skill->name2id(script_getstr(st,2)) : script_getnum(st,2) );
	uint16 skill_lv=script_getnum(st,3);
	int x=script_getnum(st,4);
	int y=script_getnum(st,5);
	
	if (bl)
		clif->skill_poseffect(bl,skill_id,skill_lv,x,y,gettick());
	
	return true;
}

/*==========================================
 * Special effects [Valaris]
 *------------------------------------------*/
BUILDIN(specialeffect)
{
	struct block_list *bl=map_id2bl(st->oid);
	int type = script_getnum(st,2);
	enum send_target target = script_hasdata(st,3) ? (send_target)script_getnum(st,3) : AREA;
	
	if(bl==NULL)
		return true;
	
	if( script_hasdata(st,4) )
	{
		TBL_NPC *nd = npc_name2id(script_getstr(st,4));
		if(nd)
			clif->specialeffect(&nd->bl, type, target);
	}
	else
	{
		if (target == SELF) {
			TBL_PC *sd=script_rid2sd(st);
			if (sd)
				clif->specialeffect_single(bl,type,sd->fd);
		} else {
			clif->specialeffect(bl, type, target);
		}
	}
	
	return true;
}

BUILDIN(specialeffect2)
{
	TBL_PC *sd=script_rid2sd(st);
	int type = script_getnum(st,2);
	enum send_target target = script_hasdata(st,3) ? (send_target)script_getnum(st,3) : AREA;
	
	if( script_hasdata(st,4) )
		sd = map_nick2sd(script_getstr(st,4));
	
	if (sd)
		clif->specialeffect(&sd->bl, type, target);
	
	return true;
}

/*==========================================
 * Nude [Valaris]
 *------------------------------------------*/
BUILDIN(nude)
{
	TBL_PC *sd = script_rid2sd(st);
	int i, calcflag = 0;
	
	if( sd == NULL )
		return true;
	
	for( i = 0 ; i < EQI_MAX; i++ ) {
		if( sd->equip_index[ i ] >= 0 ) {
			if( !calcflag )
				calcflag = 1;
			pc_unequipitem( sd , sd->equip_index[ i ] , 2);
		}
	}
	
	if( calcflag )
		status_calc_pc(sd,0);
	
	return true;
}

/*==========================================
 * gmcommand [MouseJstr]
 *------------------------------------------*/
BUILDIN(atcommand)
{
	TBL_PC dummy_sd;
	TBL_PC* sd;
	int fd;
	const char* cmd;
	
	cmd = script_getstr(st,2);
	
	if (st->rid) {
		sd = script_rid2sd(st);
		fd = sd->fd;
	} else { //Use a dummy character.
		sd = &dummy_sd;
		fd = 0;
		
		memset(&dummy_sd, 0, sizeof(TBL_PC));
		if (st->oid)
		{
			struct block_list* bl = map_id2bl(st->oid);
			memcpy(&dummy_sd.bl, bl, sizeof(struct block_list));
			if (bl->type == BL_NPC)
				safestrncpy(dummy_sd.status.name, ((TBL_NPC*)bl)->name, NAME_LENGTH);
		}
	}
	
	if (!atcommand->parse(fd, sd, cmd, 0)) {
		ShowWarning("script: buildin_atcommand: failed to execute command '%s'\n", cmd);
		script_reportsrc(st);
		return false;
	}
	
	return true;
}

/*==========================================
 * Displays a message for the player only (like system messages like "you got an apple" )
 *------------------------------------------*/
BUILDIN(dispbottom)
{
	TBL_PC *sd=script_rid2sd(st);
	const char *message;
	message=script_getstr(st,2);
	if(sd)
		clif->disp_onlyself(sd,message,(int)strlen(message));
	return true;
}

/*==========================================
 * All The Players Full Recovery
 * (HP/SP full restore and resurrect if need)
 *------------------------------------------*/
BUILDIN(recovery)
{
	TBL_PC* sd;
	struct s_mapiterator* iter;
	
	iter = mapit_getallusers();
	for( sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); sd = (TBL_PC*)mapit->next(iter) )
	{
		if(pc_isdead(sd))
			status_revive(&sd->bl, 100, 100);
		else
			status_percent_heal(&sd->bl, 100, 100);
		clif->message(sd->fd,msg_txt(680));
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
	TBL_PC *sd=script_rid2sd(st);
	TBL_PET *pd;
	int type=script_getnum(st,2);
	
	if(!sd || !sd->pd) {
		if (type == 2)
			script_pushconststr(st,"null");
		else
			script_pushint(st,0);
		return true;
	}
	pd = sd->pd;
	switch(type){
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
	TBL_PC *sd=script_rid2sd(st);
	TBL_HOM *hd;
	int type=script_getnum(st,2);
	
	hd = sd?sd->hd:NULL;
	if(!homun_alive(hd))
	{
		if (type == 2)
			script_pushconststr(st,"null");
		else
			script_pushint(st,0);
		return true;
	}
	
	switch(type){
		case 0: script_pushint(st,hd->homunculus.hom_id); break;
		case 1: script_pushint(st,hd->homunculus.class_); break;
		case 2: script_pushstrcopy(st,hd->homunculus.name); break;
		case 3: script_pushint(st,hd->homunculus.intimacy); break;
		case 4: script_pushint(st,hd->homunculus.hunger); break;
		case 5: script_pushint(st,hd->homunculus.rename_flag); break;
		case 6: script_pushint(st,hd->homunculus.level); break;
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
	int type, char_id;
	struct map_session_data* sd;
	struct mercenary_data* md;
	
	type = script_getnum(st,2);
	
	if( script_hasdata(st,3) )
	{
		char_id = script_getnum(st,3);
		
		if( ( sd = map_charid2sd(char_id) ) == NULL )
		{
			ShowError("buildin_getmercinfo: No such character (char_id=%d).\n", char_id);
			script_pushnil(st);
			return false;
		}
	}
	else
	{
		if( ( sd = script_rid2sd(st) ) == NULL )
		{
			script_pushnil(st);
			return true;
		}
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
		case 3: script_pushint(st,md ? mercenary_get_faith(md) : 0); break;
		case 4: script_pushint(st,md ? mercenary_get_calls(md) : 0); break;
		case 5: script_pushint(st,md ? md->mercenary.kill_count : 0); break;
		case 6: script_pushint(st,md ? mercenary_get_lifetime(md) : 0); break;
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
 selected card or not.
 checkequipedcard(4001);
 *------------------------------------------*/
BUILDIN(checkequipedcard)
{
	TBL_PC *sd=script_rid2sd(st);
	
	if(sd){
		int n,i,c=0;
		c=script_getnum(st,2);
		
		for(i=0;i<MAX_INVENTORY;i++){
			if(sd->status.inventory[i].nameid > 0 && sd->status.inventory[i].amount && sd->inventory_data[i]){
				if (itemdb_isspecial(sd->status.inventory[i].card[0]))
					continue;
				for(n=0;n<sd->inventory_data[i]->slot;n++){
					if(sd->status.inventory[i].card[n]==c){
						script_pushint(st,1);
						return true;
					}
				}
			}
		}
	}
	script_pushint(st,0);
	return true;
}

BUILDIN(jump_zero)
{
	int sel;
	sel=script_getnum(st,2);
	if(!sel) {
		int pos;
		if( !data_islabel(script_getdata(st,3)) ){
			ShowError("script: jump_zero: not label !\n");
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
BUILDIN(movenpc) {
	TBL_NPC *nd = NULL;
	const char *npc;
	int x,y;
	
	npc = script_getstr(st,2);
	x = script_getnum(st,3);
	y = script_getnum(st,4);
	
	if ((nd = npc_name2id(npc)) == NULL)
		return -1;
	
	if (script_hasdata(st,5))
		nd->dir = script_getnum(st,5) % 8;
	npc_movenpc(nd, x, y);
	return true;
}

/*==========================================
 * message [MouseJstr]
 *------------------------------------------*/
BUILDIN(message)
{
	const char *msg,*player;
	TBL_PC *pl_sd = NULL;
	
	player = script_getstr(st,2);
	msg = script_getstr(st,3);
	
	if((pl_sd=map_nick2sd((char *) player)) == NULL)
		return true;
	clif->message(pl_sd->fd, msg);
	
	return true;
}

/*==========================================
 * npctalk (sends message to surrounding area)
 *------------------------------------------*/
BUILDIN(npctalk)
{
	const char* str;
	char name[NAME_LENGTH], message[256];
	
	struct npc_data* nd = (struct npc_data *)map_id2bl(st->oid);
	str = script_getstr(st,2);
	
	if(nd)
	{
		safestrncpy(name, nd->name, sizeof(name));
		strtok(name, "#"); // discard extra name identifier if present
		safesnprintf(message, sizeof(message), "%s : %s", name, str);
		clif->disp_overhead(&nd->bl, message);
	}
	
	return true;
}

// change npc walkspeed [Valaris]
BUILDIN(npcspeed)
{
	struct npc_data* nd;
	int speed;
	
	speed = script_getnum(st,2);
	nd =(struct npc_data *)map_id2bl(st->oid);
	
	if( nd ) {
		if( nd->ud == &npc_base_ud ) {
			nd->ud = NULL;
			CREATE(nd->ud, struct unit_data, 1);
			unit_dataset(&nd->bl);
		}
		nd->speed = speed;
		nd->ud->state.speed_changed = 1;
	}
	
	return true;
}
// make an npc walk to a position [Valaris]
BUILDIN(npcwalkto) {
	struct npc_data *nd=(struct npc_data *)map_id2bl(st->oid);
	int x=0,y=0;
	
	x=script_getnum(st,2);
	y=script_getnum(st,3);
	
	if(nd) {
		if( nd->ud == &npc_base_ud ) {
			nd->ud = NULL;
			CREATE(nd->ud, struct unit_data, 1);
			unit_dataset(&nd->bl);
		}
		
		if (!nd->status.hp) {
			status_calc_npc(nd, true);
		} else {
			status_calc_npc(nd, false);
		}
		unit_walktoxy(&nd->bl,x,y,0);
	}
	
	return true;
}
// stop an npc's movement [Valaris]
BUILDIN(npcstop)
{
	struct npc_data *nd=(struct npc_data *)map_id2bl(st->oid);
	
	if(nd) {
		unit_stop_walking(&nd->bl,1|4);
	}
	
	return true;
}


/*==========================================
 * getlook char info. getlook(arg)
 *------------------------------------------*/
BUILDIN(getlook)
{
	int type,val;
	TBL_PC *sd;
	sd=script_rid2sd(st);
	
	type=script_getnum(st,2);
	val=-1;
	switch(type) {
		case LOOK_HAIR:			val=sd->status.hair; break; //1
		case LOOK_WEAPON:		val=sd->status.weapon; break; //2
		case LOOK_HEAD_BOTTOM:	val=sd->status.head_bottom; break; //3
		case LOOK_HEAD_TOP:		val=sd->status.head_top; break; //4
		case LOOK_HEAD_MID:		val=sd->status.head_mid; break; //5
		case LOOK_HAIR_COLOR:	val=sd->status.hair_color; break; //6
		case LOOK_CLOTHES_COLOR:val=sd->status.clothes_color; break; //7
		case LOOK_SHIELD:		val=sd->status.shield; break; //8
		case LOOK_SHOES: break; //9
		case LOOK_ROBE:			val=sd->status.robe; break; //12
	}
	
	script_pushint(st,val);
	return true;
}

/*==========================================
 *     get char save point. argument: 0- map name, 1- x, 2- y
 *------------------------------------------*/
BUILDIN(getsavepoint)
{
	TBL_PC* sd;
	int type;
	
	sd = script_rid2sd(st);
	if (sd == NULL) {
		script_pushint(st,0);
		return true;
	}
	
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
	TBL_PC *sd=NULL;
	
	int num;
	const char *name;
	char prefix;
	
	int x,y,type;
	char mapname[MAP_NAME_LENGTH];
	
	if( !data_isreference(script_getdata(st,2)) ){
		ShowWarning("script: buildin_getmapxy: not mapname variable\n");
		script_pushint(st,-1);
		return false;
	}
	if( !data_isreference(script_getdata(st,3)) ){
		ShowWarning("script: buildin_getmapxy: not mapx variable\n");
		script_pushint(st,-1);
		return false;
	}
	if( !data_isreference(script_getdata(st,4)) ){
		ShowWarning("script: buildin_getmapxy: not mapy variable\n");
		script_pushint(st,-1);
		return false;
	}
	
	// Possible needly check function parameters on C_STR,C_INT,C_INT
	type=script_getnum(st,5);
	
	switch (type){
		case 0:	//Get Character Position
			if( script_hasdata(st,6) )
				sd=map_nick2sd(script_getstr(st,6));
			else
				sd=script_rid2sd(st);
			
			if (sd)
				bl = &sd->bl;
			break;
		case 1:	//Get NPC Position
			if( script_hasdata(st,6) )
			{
				struct npc_data *nd;
				nd=npc_name2id(script_getstr(st,6));
				if (nd)
					bl = &nd->bl;
			} else //In case the origin is not an npc?
				bl=map_id2bl(st->oid);
			break;
		case 2:	//Get Pet Position
			if(script_hasdata(st,6))
				sd=map_nick2sd(script_getstr(st,6));
			else
				sd=script_rid2sd(st);
			
			if (sd && sd->pd)
				bl = &sd->pd->bl;
			break;
		case 3:	//Get Mob Position
			break; //Not supported?
		case 4:	//Get Homun Position
			if(script_hasdata(st,6))
				sd=map_nick2sd(script_getstr(st,6));
			else
				sd=script_rid2sd(st);
			
			if (sd && sd->hd)
				bl = &sd->hd->bl;
			break;
		case 5: //Get Mercenary Position
			if(script_hasdata(st,6))
				sd=map_nick2sd(script_getstr(st,6));
			else
				sd=script_rid2sd(st);
			
			if (sd && sd->md)
				bl = &sd->md->bl;
			break;
		case 6: //Get Elemental Position
			if(script_hasdata(st,6))
				sd=map_nick2sd(script_getstr(st,6));
			else
				sd=script_rid2sd(st);
			
			if (sd && sd->ed)
				bl = &sd->ed->bl;
			break;
		default:
			ShowWarning("script: buildin_getmapxy: Invalid type %d\n", type);
			script_pushint(st,-1);
			return false;
	}
	if (!bl) { //No object found.
		script_pushint(st,-1);
		return true;
	}
	
	x= bl->x;
	y= bl->y;
	safestrncpy(mapname, map[bl->m].name, MAP_NAME_LENGTH);
	
	//Set MapName$
	num=st->stack->stack_data[st->start+2].u.num;
	name=get_str(num&0x00ffffff);
	prefix=*name;
	
	if(not_server_variable(prefix))
		sd=script_rid2sd(st);
	else
		sd=NULL;
	set_reg(st,sd,num,name,(void*)mapname,script_getref(st,2));
	
	//Set MapX
	num=st->stack->stack_data[st->start+3].u.num;
	name=get_str(num&0x00ffffff);
	prefix=*name;
	
	if(not_server_variable(prefix))
		sd=script_rid2sd(st);
	else
		sd=NULL;
	set_reg(st,sd,num,name,(void*)__64BPTRSIZE(x),script_getref(st,3));
	
	//Set MapY
	num=st->stack->stack_data[st->start+4].u.num;
	name=get_str(num&0x00ffffff);
	prefix=*name;
	
	if(not_server_variable(prefix))
		sd=script_rid2sd(st);
	else
		sd=NULL;
	set_reg(st,sd,num,name,(void*)__64BPTRSIZE(y),script_getref(st,4));
	
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
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return false;
	
	str = script_getstr(st,2);
	logs->npc(sd,str);
	return true;
}

BUILDIN(summon)
{
	int _class, timeout=0;
	const char *str,*event="";
	TBL_PC *sd;
	struct mob_data *md;
	int tick = gettick();
	
	sd=script_rid2sd(st);
	if (!sd) return true;
	
	str	=script_getstr(st,2);
	_class=script_getnum(st,3);
	if( script_hasdata(st,4) )
		timeout=script_getnum(st,4);
	if( script_hasdata(st,5) ){
		event=script_getstr(st,5);
		check_event(st, event);
	}
	
	clif->skill_poseffect(&sd->bl,AM_CALLHOMUN,1,sd->bl.x,sd->bl.y,tick);
	
	md = mob_once_spawn_sub(&sd->bl, sd->bl.m, sd->bl.x, sd->bl.y, str, _class, event, SZ_SMALL, AI_NONE);
	if (md) {
		md->master_id=sd->bl.id;
		md->special_state.ai = AI_ATTACK;
		if( md->deletetimer != INVALID_TIMER )
			delete_timer(md->deletetimer, mob_timer_delete);
		md->deletetimer = add_timer(tick+(timeout>0?timeout*1000:60000),mob_timer_delete,md->bl.id,0);
		mob_spawn (md); //Now it is ready for spawning.
		clif->specialeffect(&md->bl,344,AREA);
		sc_start4(&md->bl, SC_MODECHANGE, 100, 1, 0, MD_AGGRESSIVE, 0, 60000);
	}
	return true;
}

/*==========================================
 * Checks whether it is daytime/nighttime
 *------------------------------------------*/
BUILDIN(isnight)
{
	script_pushint(st,(night_flag == 1));
	return true;
}

BUILDIN(isday)
{
	script_pushint(st,(night_flag == 0));
	return true;
}

/*================================================
 * Check how many items/cards in the list are
 * equipped - used for 2/15's cards patch [celest]
 *------------------------------------------------*/
BUILDIN(isequippedcnt)
{
	TBL_PC *sd;
	int i, j, k, id = 1;
	int ret = 0;
	
	sd = script_rid2sd(st);
	if (!sd) { //If the player is not attached it is a script error anyway... but better prevent the map server from crashing...
		script_pushint(st,0);
		return true;
	}
	
	for (i=0; id!=0; i++) {
		FETCH (i+2, id) else id = 0;
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
	TBL_PC *sd;
	int i, j, k, id = 1;
	int index, flag;
	int ret = -1;
	//Original hash to reverse it when full check fails.
	unsigned int setitem_hash = 0, setitem_hash2 = 0;
	
	sd = script_rid2sd(st);
	
	if (!sd) { //If the player is not attached it is a script error anyway... but better prevent the map server from crashing...
		script_pushint(st,0);
		return true;
	}
	
	setitem_hash = sd->bonus.setitem_hash;
	setitem_hash2 = sd->bonus.setitem_hash2;
	for (i=0; id!=0; i++) {
		FETCH (i+2, id) else id = 0;
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
				
				for (k = 0; k < sd->inventory_data[index]->slot; k++)
				{	//New hash system which should support up to 4 slots on any equipment. [Skotlex]
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
	TBL_PC *sd;
	int i, k, id = 1;
	int ret = 0;
	int index;
	
	sd = script_rid2sd(st);
	
	for (i=0; id!=0; i++) {
		FETCH (i+2, id) else id = 0;
		if (id <= 0)
			continue;
		
		index = current_equip_item_index; //we get CURRENT WEAPON inventory index from status.c [Lupus]
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
	//	script_pushint(st,current_equip_item_index);
	return true;
}

/*=======================================================
 * Returns the refined number of the current item, or an
 * item with inventory index specified
 *-------------------------------------------------------*/
BUILDIN(getrefine)
{
	TBL_PC *sd;
	if ((sd = script_rid2sd(st))!= NULL)
		script_pushint(st,sd->status.inventory[current_equip_item_index].refine);
	else
		script_pushint(st,0);
	return true;
}

/*=======================================================
 * Day/Night controls
 *-------------------------------------------------------*/
BUILDIN(night)
{
	if (night_flag != 1) map_night_timer(night_timer_tid, 0, 0, 1);
	return true;
}
BUILDIN(day)
{
	if (night_flag != 0) map_day_timer(day_timer_tid, 0, 0, 1);
	return true;
}

//=======================================================
// Unequip [Spectre]
//-------------------------------------------------------
BUILDIN(unequip)
{
	int i;
	size_t num;
	TBL_PC *sd;
	
	num = script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd != NULL && num >= 1 && num <= ARRAYLENGTH(equip) )
	{
		i = pc_checkequip(sd,equip[num-1]);
		if (i >= 0)
			pc_unequipitem(sd,i,1|2);
	}
	return true;
}

BUILDIN(equip)
{
	int nameid=0,i;
	TBL_PC *sd;
	struct item_data *item_data;
	
	sd = script_rid2sd(st);
	
	nameid=script_getnum(st,2);
	if((item_data = itemdb_exists(nameid)) == NULL)
	{
		ShowError("wrong item ID : equipitem(%i)\n",nameid);
		return false;
	}
	ARR_FIND( 0, MAX_INVENTORY, i, sd->status.inventory[i].nameid == nameid );
	if( i < MAX_INVENTORY )
		pc_equipitem(sd,i,item_data->equip);
	
	return true;
}

BUILDIN(autoequip)
{
	int nameid, flag;
	struct item_data *item_data;
	nameid=script_getnum(st,2);
	flag=script_getnum(st,3);
	
	if( ( item_data = itemdb_exists(nameid) ) == NULL )
	{
		ShowError("buildin_autoequip: Invalid item '%d'.\n", nameid);
		return false;
	}
	
	if( !itemdb_isequip2(item_data) )
	{
		ShowError("buildin_autoequip: Item '%d' cannot be equipped.\n", nameid);
		return false;
	}
	
	item_data->flag.autoequip = flag>0?1:0;
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
	flag = script_getstr(st,2);
	script_pushint(st,battle->config_get_value(flag));
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
		index = len;
	
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
	int i = 0, j = 0;
	int start;
	
	
	char *temp;
	const char* name;
	
	TBL_PC* sd = NULL;
	
	temp = (char*)aMalloc(len + 1);
	
	if( !data_isreference(data) )
	{
		ShowError("script:explode: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return false;// not a variable
	}
	
	id = reference_getid(data);
	start = reference_getindex(data);
	name = reference_getname(data);
	
	if( not_array_variable(*name) )
	{
		ShowError("script:explode: illegal scope\n");
		script_reportdata(data);
		st->state = END;
		return false;// not supported
	}
	
	if( !is_string_variable(name) )
	{
		ShowError("script:explode: not string array\n");
		script_reportdata(data);
		st->state = END;
		return false;// data type mismatch
	}
	
	if( not_server_variable(*name) )
	{
		sd = script_rid2sd(st);
		if( sd == NULL )
			return true;// no player attached
	}
	
	while(str[i] != '\0') {
		if(str[i] == delimiter && start < SCRIPT_MAX_ARRAYSIZE-1) { //break at delimiter but ignore after reaching last array index
			temp[j] = '\0';
			set_reg(st, sd, reference_uid(id, start++), name, (void*)temp, reference_getref(data));
			j = 0;
			++i;
		} else {
			temp[j++] = str[i++];
		}
	}
	//set last string
	temp[j] = '\0';
	set_reg(st, sd, reference_uid(id, start), name, (void*)temp, reference_getref(data));
	
	aFree(temp);
	return true;
}

//=======================================================
// implode <string_array>
// implode <string_array>, <glue>
//-------------------------------------------------------
BUILDIN(implode)
{
	struct script_data* data = script_getdata(st, 2);
	const char *glue = NULL, *name, *temp;
	int32 glue_len = 0, array_size, id;
	size_t len = 0;
	int i, k = 0;
	
	TBL_PC* sd = NULL;
	
	char *output;
	
	if( !data_isreference(data) )
	{
		ShowError("script:implode: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return false;// not a variable
	}
	
	id = reference_getid(data);
	name = reference_getname(data);
	
	if( not_array_variable(*name) )
	{
		ShowError("script:implode: illegal scope\n");
		script_reportdata(data);
		st->state = END;
		return false;// not supported
	}
	
	if( !is_string_variable(name) )
	{
		ShowError("script:implode: not string array\n");
		script_reportdata(data);
		st->state = END;
		return false;// data type mismatch
	}
	
	if( not_server_variable(*name) )
	{
		sd = script_rid2sd(st);
		if( sd == NULL )
			return true;// no player attached
	}
	
	//count chars
	array_size = getarraysize(st, id, reference_getindex(data), is_string_variable(name), reference_getref(data)) - 1;
	
	if(array_size == -1) //empty array check (AmsTaff)
    {
        ShowWarning("script:implode: array length = 0\n");
        output = (char*)aMalloc(sizeof(char)*5);
        sprintf(output,"%s","NULL");
	} else {
		for(i = 0; i <= array_size; ++i) {
			temp = (char*) get_val2(st, reference_uid(id, i), reference_getref(data));
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
			temp = (char*) get_val2(st, reference_uid(id, i), reference_getref(data));
			len = strlen(temp);
			memcpy(&output[k], temp, len);
			k += len;
			if(glue_len != 0) {
				memcpy(&output[k], glue, glue_len);
				k += glue_len;
			}
			script_removetop(st, -1, 0);
		}
		temp = (char*) get_val2(st, reference_uid(id, array_size), reference_getref(data));
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
BUILDIN(sprintf)
{
    unsigned int len, argc = 0, arg = 0, buf2_len = 0;
    const char* format;
    char* p;
    char* q;
    char* buf  = NULL;
    char* buf2 = NULL;
    struct script_data* data;
    StringBuf final_buf;
	
    // Fetch init data
    format = script_getstr(st, 2);
    argc = script_lastdata(st)-2;
    len = strlen(format);
	
    // Skip parsing, where no parsing is required.
    if(len==0){
        script_pushconststr(st,"");
        return true;
    }
	
    // Pessimistic alloc
    CREATE(buf, char, len+1);
	
    // Need not be parsed, just solve stuff like %%.
    if(argc==0){
		memcpy(buf,format,len+1);
        script_pushstrcopy(st, buf);
        aFree(buf);
        return true;
    }
	
    safestrncpy(buf, format, len+1);
	
    // Issue sprintf for each parameter
    StrBuf->Init(&final_buf);
    q = buf;
    while((p = strchr(q, '%'))!=NULL){
        if(p!=q){
            len = p-q+1;
            if(buf2_len<len){
                RECREATE(buf2, char, len);
                buf2_len = len;
            }
            safestrncpy(buf2, q, len);
            StrBuf->AppendStr(&final_buf, buf2);
            q = p;
        }
        p = q+1;
        if(*p=='%'){  // %%
            StrBuf->AppendStr(&final_buf, "%");
            q+=2;
            continue;
        }
        if(*p=='n'){  // %n
            ShowWarning("buildin_sprintf: Format %%n not supported! Skipping...\n");
            script_reportsrc(st);
            q+=2;
            continue;
        }
        if(arg>=argc){
            ShowError("buildin_sprintf: Not enough arguments passed!\n");
            if(buf) aFree(buf);
            if(buf2) aFree(buf2);
            StrBuf->Destroy(&final_buf);
            script_pushconststr(st,"");
            return false;
        }
        if((p = strchr(q+1, '%'))==NULL){
            p = strchr(q, 0);  // EOS
        }
        len = p-q+1;
        if(buf2_len<len){
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
        if(data_isstring(data)){  // String
            StrBuf->Printf(&final_buf, buf2, script_getstr(st, arg+3));
        }else if(data_isint(data)){  // Number
            StrBuf->Printf(&final_buf, buf2, script_getnum(st, arg+3));
        }else if(data_isreference(data)){  // Variable
            char* name = reference_getname(data);
            if(name[strlen(name)-1]=='$'){  // var Str
                StrBuf->Printf(&final_buf, buf2, script_getstr(st, arg+3));
            }else{  // var Int
                StrBuf->Printf(&final_buf, buf2, script_getnum(st, arg+3));
            }
        }else{  // Unsupported type
            ShowError("buildin_sprintf: Unknown argument type!\n");
            if(buf) aFree(buf);
            if(buf2) aFree(buf2);
            StrBuf->Destroy(&final_buf);
            script_pushconststr(st,"");
            return false;
        }
        arg++;
    }
	
    // Append anything left
    if(*q){
        StrBuf->AppendStr(&final_buf, q);
    }
	
    // Passed more, than needed
    if(arg<argc){
        ShowWarning("buildin_sprintf: Unused arguments passed.\n");
        script_reportsrc(st);
    }
	
    script_pushstrcopy(st, StrBuf->Value(&final_buf));
	
    if(buf) aFree(buf);
    if(buf2) aFree(buf2);
    StrBuf->Destroy(&final_buf);
	
    return true;
}

//=======================================================
// sscanf(<str>, <format>, ...);
// Implements C sscanf.
//-------------------------------------------------------
BUILDIN(sscanf){
    unsigned int argc, arg = 0, len;
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
	
    // Get data
    str = script_getstr(st, 2);
    format = script_getstr(st, 3);
    argc = script_lastdata(st)-3;
	
    len = strlen(format);
    CREATE(buf, char, len*2+1);
	
    // Issue sscanf for each parameter
    *buf = 0;
    q = format;
    while((p = strchr(q, '%'))){
        if(p!=q){
            strncat(buf, q, (size_t)(p-q));
            q = p;
        }
        p = q+1;
        if(*p=='*' || *p=='%'){  // Skip
            strncat(buf, q, 2);
            q+=2;
            continue;
        }
        if(arg>=argc){
            ShowError("buildin_sscanf: Not enough arguments passed!\n");
            script_pushint(st, -1);
            if(buf) aFree(buf);
            if(ref_str) aFree(ref_str);
            return false;
        }
        if((p = strchr(q+1, '%'))==NULL){
            p = strchr(q, 0);  // EOS
        }
        len = p-q;
        strncat(buf, q, len);
        q = p;
		
        // Validate output
        data = script_getdata(st, arg+4);
        if(!data_isreference(data) || !reference_tovariable(data)){
            ShowError("buildin_sscanf: Target argument is not a variable!\n");
            script_pushint(st, -1);
            if(buf) aFree(buf);
            if(ref_str) aFree(ref_str);
            return false;
        }
        buf_p = reference_getname(data);
        if(not_server_variable(*buf_p) && (sd = script_rid2sd(st))==NULL){
            script_pushint(st, -1);
            if(buf) aFree(buf);
            if(ref_str) aFree(ref_str);
            return true;
        }
		
        // Save value if any
        if(buf_p[strlen(buf_p)-1]=='$'){  // String
            if(ref_str==NULL){
                CREATE(ref_str, char, strlen(str)+1);
            }
            if(sscanf(str, buf, ref_str)==0){
                break;
            }
			set_reg(st, sd, reference_uid( reference_getid(data), reference_getindex(data) ), buf_p, (void *)(ref_str), reference_getref(data));
        } else {  // Number
            if(sscanf(str, buf, &ref_int)==0){
                break;
            }
			set_reg(st, sd, reference_uid( reference_getid(data), reference_getindex(data) ), buf_p, (void *)__64BPTRSIZE(ref_int), reference_getref(data));
        }
        arg++;
		
        // Disable used format (%... -> %*...)
        buf_p = strchr(buf, 0);
        memmove(buf_p-len+2, buf_p-len+1, len);
        *(buf_p-len+1) = '*';
    }
	
    script_pushint(st, arg);
    if(buf) aFree(buf);
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
		if( !script_isstring(st,5) )
			usecase = script_getnum(st, 5) != 0;
		else {
			ShowError("script:replacestr: Invalid usecase value. Expected int got string\n");
			st->state = END;
			return false;
		}
	}
	
	if(script_hasdata(st, 6)) {
		count = script_getnum(st, 6);
		if(count == 0) {
			ShowError("script:replacestr: Invalid count value. Expected int got string\n");
			st->state = END;
			return false;
		}
	}
	
	StrBuf->Init(&output);
	
	for(; i < inputlen; i++) {
		if(count && count == numFinds) {	//found enough, stop looking
			break;
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
		if( !script_isstring(st,4) )
			usecase = script_getnum(st, 4) != 0;
		else {
			ShowError("script:countstr: Invalid usecase value. Expected int got string\n");
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
BUILDIN(setnpcdisplay)
{
	const char* name;
	const char* newname = NULL;
	int class_ = -1, size = -1;
	struct script_data* data;
	struct npc_data* nd;
	
	name = script_getstr(st,2);
	data = script_getdata(st,3);
	
	if( script_hasdata(st,4) )
		class_ = script_getnum(st,4);
	if( script_hasdata(st,5) )
		size = script_getnum(st,5);
	
	get_val(st, data);
	if( data_isstring(data) )
 		newname = script->conv_str(st,data);
	else if( data_isint(data) )
 		class_ = script->conv_num(st,data);
	else
	{
		ShowError("script:setnpcdisplay: expected a string or number\n");
		script_reportdata(data);
		return false;
	}
	
	nd = npc_name2id(name);
	if( nd == NULL )
	{// not found
		script_pushint(st,1);
		return true;
	}
	
	// update npc
	if( newname )
		npc_setdisplayname(nd, newname);
	
	if( size != -1 && size != (int)nd->size )
		nd->size = size;
	else
		size = -1;
	
	if( class_ != -1 && nd->class_ != class_ )
		npc_setclass(nd, class_);
	else if( size != -1 )
	{ // Required to update the visual size
		clif->clearunit_area(&nd->bl, CLR_OUTSIGHT);
		clif->spawn(&nd->bl);
	}
	
	script_pushint(st,0);
	return true;
}

BUILDIN(atoi)
{
	const char *value;
	value = script_getstr(st,2);
	script_pushint(st,atoi(value));
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

// [zBuffer] List of mathematics commands --->
BUILDIN(sqrt)
{
	double i, a;
	i = script_getnum(st,2);
	a = sqrt(i);
	script_pushint(st,(int)a);
	return true;
}

BUILDIN(pow)
{
	double i, a, b;
	a = script_getnum(st,2);
	b = script_getnum(st,3);
	i = pow(a,b);
	script_pushint(st,(int)i);
	return true;
}

BUILDIN(distance)
{
	int x0, y0, x1, y1;
	
	x0 = script_getnum(st,2);
	y0 = script_getnum(st,3);
	x1 = script_getnum(st,4);
	y1 = script_getnum(st,5);
	
	script_pushint(st,distance_xy(x0,y0,x1,y1));
	return true;
}

// <--- [zBuffer] List of mathematics commands

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

// [zBuffer] List of dynamic var commands --->

BUILDIN(setd)
{
	TBL_PC *sd=NULL;
	char varname[100];
	const char *buffer;
	int elem;
	buffer = script_getstr(st, 2);
	
	if(sscanf(buffer, "%99[^[][%d]", varname, &elem) < 2)
		elem = 0;
	
	if( not_server_variable(*varname) )
	{
		sd = script_rid2sd(st);
		if( sd == NULL )
		{
			ShowError("script:setd: no player attached for player variable '%s'\n", buffer);
			return true;
		}
	}
	
	if( is_string_variable(varname) ) {
		setd_sub(st, sd, varname, elem, (void *)script_getstr(st, 3), NULL);
	} else {
		setd_sub(st, sd, varname, elem, (void *)__64BPTRSIZE(script_getnum(st, 3)), NULL);
	}
	
	return true;
}

int buildin_query_sql_sub(struct script_state* st, Sql* handle)
{
	int i, j;
	TBL_PC* sd = NULL;
	const char* query;
	struct script_data* data;
	const char* name;
	int max_rows = SCRIPT_MAX_ARRAYSIZE; // maximum number of rows
	int num_vars;
	int num_cols;
	
	// check target variables
	for( i = 3; script_hasdata(st,i); ++i ) {
		data = script_getdata(st, i);
		if( data_isreference(data) ) { // it's a variable
			name = reference_getname(data);
			if( not_server_variable(*name) && sd == NULL ) { // requires a player
				sd = script_rid2sd(st);
				if( sd == NULL ) { // no player attached
					script_reportdata(data);
					st->state = END;
					return false;
				}
			}
			if( not_array_variable(*name) )
				max_rows = 1;// not an array, limit to one row
		} else {
			ShowError("script:query_sql: not a variable\n");
			script_reportdata(data);
			st->state = END;
			return false;
		}
	}
	num_vars = i - 3;
	
	// Execute the query
	query = script_getstr(st,2);
	
	if( SQL_ERROR == SQL->QueryStr(handle, query) ) {
		Sql_ShowDebug(handle);
		script_pushint(st, 0);
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
		script_reportsrc(st);
	} else if( num_vars > num_cols ) {
		ShowWarning("script:query_sql: Too many variables (%u extra).\n", (unsigned int)(num_vars-num_cols));
		script_reportsrc(st);
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
				setd_sub(st, sd, name, i, (void *)(str?str:""), reference_getref(data));
			else
				setd_sub(st, sd, name, i, (void *)__64BPTRSIZE((str?atoi(str):0)), reference_getref(data));
		}
	}
	if( i == max_rows && max_rows < SQL->NumRows(handle) ) {
		ShowWarning("script:query_sql: Only %d/%u rows have been stored.\n", max_rows, (unsigned int)SQL->NumRows(handle));
		script_reportsrc(st);
	}
	
	// Free data
	SQL->FreeResult(handle);
	script_pushint(st, i);
	
	return true;
}
BUILDIN(query_sql) {
	return buildin_query_sql_sub(st, mmysql_handle);
}

BUILDIN(query_logsql) {
	if( !logs->config.sql_logs ) {// logmysql_handle == NULL
		ShowWarning("buildin_query_logsql: SQL logs are disabled, query '%s' will not be executed.\n", script_getstr(st,2));
		script_pushint(st,-1);
		return false;
	}
	return buildin_query_sql_sub(st, logmysql_handle);
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
	SQL->EscapeStringLen(mmysql_handle, esc_str, str, len);
	script_pushstr(st, esc_str);
	return true;
}

BUILDIN(getd)
{
	char varname[100];
	const char *buffer;
	int elem;
	
	buffer = script_getstr(st, 2);
	
	if(sscanf(buffer, "%[^[][%d]", varname, &elem) < 2)
		elem = 0;
	
	// Push the 'pointer' so it's more flexible [Lance]
	push_val(st->stack, C_NAME, reference_uid(add_str(varname), elem));
	
	return true;
}

// <--- [zBuffer] List of dynamic var commands
// Pet stat [Lance]
BUILDIN(petstat)
{
	TBL_PC *sd = NULL;
	struct pet_data *pd;
	int flag = script_getnum(st,2);
	sd = script_rid2sd(st);
	if(!sd || !sd->status.pet_id || !sd->pd){
		if(flag == 2)
			script_pushconststr(st, "");
		else
			script_pushint(st,0);
		return true;
	}
	pd = sd->pd;
	switch(flag){
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
	TBL_PC *sd = NULL;
	struct npc_data *nd;
	const char *shopname;
	int flag = 0;
	sd = script_rid2sd(st);
	if (!sd) {
		script_pushint(st,0);
		return true;
	}
	shopname = script_getstr(st, 2);
	if( script_hasdata(st,3) )
		flag = script_getnum(st,3);
	nd = npc_name2id(shopname);
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
			case 1: npc_buysellsel(sd,nd->bl.id,0); break; //Buy window
			case 2: npc_buysellsel(sd,nd->bl.id,1); break; //Sell window
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
	struct npc_data* nd = npc_name2id(npcname);
	int n, i;
	int amount;
	
	if( !nd || ( nd->subtype != SHOP && nd->subtype != CASHSHOP ) )
	{	//Not found.
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
	struct npc_data* nd = npc_name2id(npcname);
	int n, i;
	int amount;
	
	if( !nd || ( nd->subtype != SHOP && nd->subtype != CASHSHOP ) )
	{	//Not found.
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
	struct npc_data* nd = npc_name2id(npcname);
	unsigned int nameid;
	int n, i;
	int amount;
	int size;
	
	if( !nd || ( nd->subtype != SHOP && nd->subtype != CASHSHOP ) )
	{	//Not found.
		script_pushint(st,0);
		return true;
	}
	
	amount = script_lastdata(st)-2;
	size = nd->u.shop.count;
	
	// remove specified items from the shop item list
	for( i = 3; i < 3 + amount; i++ )
	{
		nameid = script_getnum(st,i);
		
		ARR_FIND( 0, size, n, nd->u.shop.shop_item[n].nameid == nameid );
		if( n < size )
		{
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
BUILDIN(npcshopattach)
{
	const char* npcname = script_getstr(st,2);
	struct npc_data* nd = npc_name2id(npcname);
	int flag = 1;
	
	if( script_hasdata(st,3) )
		flag = script_getnum(st,3);
	
	if( !nd || nd->subtype != SHOP )
	{	//Not found.
		script_pushint(st,0);
		return true;
	}
	
	if (flag)
		nd->master_nd = ((struct npc_data *)map_id2bl(st->oid));
	else
		nd->master_nd = NULL;
	
	script_pushint(st,1);
	return true;
}

/*==========================================
 * Returns some values of an item [Lupus]
 * Price, Weight, etc...
 setitemscript(itemID,"{new item bonus script}",[n]);
 Where n:
 0 - script
 1 - Equip script
 2 - Unequip script
 *------------------------------------------*/
BUILDIN(setitemscript)
{
	int item_id,n=0;
	const char *new_bonus_script;
	struct item_data *i_data;
	struct script_code **dstscript;
	
	item_id	= script_getnum(st,2);
	new_bonus_script = script_getstr(st,3);
	if( script_hasdata(st,4) )
		n=script_getnum(st,4);
	i_data = itemdb_exists(item_id);
	
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
		script_free_code(*dstscript);
	
	*dstscript = new_bonus_script[0] ? parse_script(new_bonus_script, "script_setitemscript", 0, 0) : NULL;
	script_pushint(st,1);
	return true;
}

/* Work In Progress [Lupus]
 BUILDIN(addmonsterdrop)
 {
 int class_,item_id,chance;
 class_=script_getnum(st,2);
 item_id=script_getnum(st,3);
 chance=script_getnum(st,4);
 if(class_>1000 && item_id>500 && chance>0) {
 script_pushint(st,1);
 } else {
 script_pushint(st,0);
 }
 }
 
 BUILDIN(delmonsterdrop)
 {
 int class_,item_id;
 class_=script_getnum(st,2);
 item_id=script_getnum(st,3);
 if(class_>1000 && item_id>500) {
 script_pushint(st,1);
 } else {
 script_pushint(st,0);
 }
 }
 */

/*==========================================
 * Returns some values of a monster [Lupus]
 * Name, Level, race, size, etc...
 getmonsterinfo(monsterID,queryIndex);
 *------------------------------------------*/
BUILDIN(getmonsterinfo)
{
	struct mob_db *mob;
	int mob_id;
	
	mob_id	= script_getnum(st,2);
	if (!mobdb_checkid(mob_id)) {
		ShowError("buildin_getmonsterinfo: Wrong Monster ID: %i\n", mob_id);
		if ( !script_getnum(st,3) ) //requested a string
			script_pushconststr(st,"null");
		else
			script_pushint(st,-1);
		return -1;
	}
	mob = mob_db(mob_id);
	switch ( script_getnum(st,3) ) {
		case 0:  script_pushstrcopy(st,mob->jname); break;
		case 1:  script_pushint(st,mob->lv); break;
		case 2:  script_pushint(st,mob->status.max_hp); break;
		case 3:  script_pushint(st,mob->base_exp); break;
		case 4:  script_pushint(st,mob->job_exp); break;
		case 5:  script_pushint(st,mob->status.rhw.atk); break;
		case 6:  script_pushint(st,mob->status.rhw.atk2); break;
		case 7:  script_pushint(st,mob->status.def); break;
		case 8:  script_pushint(st,mob->status.mdef); break;
		case 9:  script_pushint(st,mob->status.str); break;
		case 10: script_pushint(st,mob->status.agi); break;
		case 11: script_pushint(st,mob->status.vit); break;
		case 12: script_pushint(st,mob->status.int_); break;
		case 13: script_pushint(st,mob->status.dex); break;
		case 14: script_pushint(st,mob->status.luk); break;
		case 15: script_pushint(st,mob->status.rhw.range); break;
		case 16: script_pushint(st,mob->range2); break;
		case 17: script_pushint(st,mob->range3); break;
		case 18: script_pushint(st,mob->status.size); break;
		case 19: script_pushint(st,mob->status.race); break;
		case 20: script_pushint(st,mob->status.def_ele); break;
		case 21: script_pushint(st,mob->status.mode); break;
		case 22: script_pushint(st,mob->mexp); break;
		default: script_pushint(st,-1); //wrong Index
	}
	return true;
}

BUILDIN(checkvending) // check vending [Nab4]
{
	TBL_PC *sd = NULL;
	
	if(script_hasdata(st,2))
		sd = map_nick2sd(script_getstr(st,2));
	else
		sd = script_rid2sd(st);
	
	if(sd)
		script_pushint(st, sd->state.autotrade ? 2 : sd->state.vending);
	else
		script_pushint(st,0);
	
	return true;
}


BUILDIN(checkchatting) // check chatting [Marka]
{
	TBL_PC *sd = NULL;
	
	if(script_hasdata(st,2))
		sd = map_nick2sd(script_getstr(st,2));
	else
		sd = script_rid2sd(st);
	
	if(sd)
		script_pushint(st,(sd->chatID != 0));
	else
		script_pushint(st,0);
	
	return true;
}

BUILDIN(checkidle)
{
	TBL_PC *sd = NULL;
	
	if (script_hasdata(st, 2))
		sd = map_nick2sd(script_getstr(st, 2));
	else
		sd = script_rid2sd(st);
	
	if (sd)
		script_pushint(st, DIFF_TICK(last_tick, sd->idletime));
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
	TBL_PC* sd = NULL;
	
	if ((items[0] = itemdb_exists(atoi(itemname))))
		count = 1;
	else {
		count = itemdb_searchname_array(items, ARRAYLENGTH(items), itemname);
		if (count > MAX_SEARCH) count = MAX_SEARCH;
	}
	
	if (!count) {
		script_pushint(st, 0);
		return true;
	}
	
	if( !data_isreference(data) )
	{
		ShowError("script:searchitem: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return false;// not a variable
	}
	
	id = reference_getid(data);
	start = reference_getindex(data);
	name = reference_getname(data);
	if( not_array_variable(*name) )
	{
		ShowError("script:searchitem: illegal scope\n");
		script_reportdata(data);
		st->state = END;
		return false;// not supported
	}
	
	if( not_server_variable(*name) )
	{
		sd = script_rid2sd(st);
		if( sd == NULL )
			return true;// no player attached
	}
	
	if( is_string_variable(name) )
	{// string array
		ShowError("script:searchitem: not an integer array reference\n");
		script_reportdata(data);
		st->state = END;
		return false;// not supported
	}
	
	for( i = 0; i < count; ++start, ++i )
	{// Set array
		void* v = (void*)__64BPTRSIZE((int)items[i]->nameid);
		set_reg(st, sd, reference_uid(id, start), name, v, reference_getref(data));
	}
	
	script_pushint(st, count);
	return true;
}

int axtoi(const char *hexStg)
{
	int n = 0;         // position in string
	int16 m = 0;         // position in digit[] to shift
	int count;         // loop index
	int intValue = 0;  // integer value of hex string
	int digit[11];      // hold values to convert
	while (n < 10) {
		if (hexStg[n]=='\0')
			break;
		if (hexStg[n] > 0x29 && hexStg[n] < 0x40 ) //if 0 to 9
			digit[n] = hexStg[n] & 0x0f;            //convert to int
		else if (hexStg[n] >='a' && hexStg[n] <= 'f') //if a to f
			digit[n] = (hexStg[n] & 0x0f) + 9;      //convert to int
		else if (hexStg[n] >='A' && hexStg[n] <= 'F') //if A to F
			digit[n] = (hexStg[n] & 0x0f) + 9;      //convert to int
		else break;
		n++;
	}
	count = n;
	m = n - 1;
	n = 0;
	while(n < count) {
		// digit[n] is value of hex digit at position n
		// (m << 2) is the number of positions to shift
		// OR the bits into return value
		intValue = intValue | (digit[n] << (m << 2));
		m--;   // adjust the position to set
		n++;   // next digit to process
	}
	return (intValue);
}

// [Lance] Hex string to integer converter
BUILDIN(axtoi)
{
	const char *hex = script_getstr(st,2);
	script_pushint(st,axtoi(hex));
	return true;
}

// [zBuffer] List of player cont commands --->
BUILDIN(rid2name)
{
	struct block_list *bl = NULL;
	int rid = script_getnum(st,2);
	if((bl = map_id2bl(rid)))
	{
		switch(bl->type) {
			case BL_MOB: script_pushstrcopy(st,((TBL_MOB*)bl)->name); break;
			case BL_PC:  script_pushstrcopy(st,((TBL_PC*)bl)->status.name); break;
			case BL_NPC: script_pushstrcopy(st,((TBL_NPC*)bl)->exname); break;
			case BL_PET: script_pushstrcopy(st,((TBL_PET*)bl)->pet.name); break;
			case BL_HOM: script_pushstrcopy(st,((TBL_HOM*)bl)->homunculus.name); break;
			case BL_MER: script_pushstrcopy(st,((TBL_MER*)bl)->db->name); break;
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

BUILDIN(pcblockmove)
{
	int id, flag;
	TBL_PC *sd = NULL;
	
	id = script_getnum(st,2);
	flag = script_getnum(st,3);
	
	if(id)
		sd = map_id2sd(id);
	else
		sd = script_rid2sd(st);
	
	if(sd)
		sd->state.blockedmove = flag > 0;
	
	return true;
}

BUILDIN(pcfollow)
{
	int id, targetid;
	TBL_PC *sd = NULL;
	
	
	id = script_getnum(st,2);
	targetid = script_getnum(st,3);
	
	if(id)
		sd = map_id2sd(id);
	else
		sd = script_rid2sd(st);
	
	if(sd)
		pc_follow(sd, targetid);
	
    return true;
}

BUILDIN(pcstopfollow)
{
	int id;
	TBL_PC *sd = NULL;
	
	
	id = script_getnum(st,2);
	
	if(id)
		sd = map_id2sd(id);
	else
		sd = script_rid2sd(st);
	
	if(sd)
		pc_stop_following(sd);
	
	return true;
}
// <--- [zBuffer] List of player cont commands
// [zBuffer] List of mob control commands --->
//## TODO always return if the request/whatever was successfull [FlavioJS]

/// Makes the unit walk to target position or map
/// Returns if it was successfull
///
/// unitwalk(<unit_id>,<x>,<y>) -> <bool>
/// unitwalk(<unit_id>,<map_id>) -> <bool>
BUILDIN(unitwalk)
{
	struct block_list* bl;
	
	bl = map_id2bl(script_getnum(st,2));
	if( bl == NULL )
	{
		script_pushint(st, 0);
	}
	else if( script_hasdata(st,4) )
	{
		int x = script_getnum(st,3);
		int y = script_getnum(st,4);
		script_pushint(st, unit_walktoxy(bl,x,y,0));// We'll use harder calculations.
	}
	else
	{
		int map_id = script_getnum(st,3);
		script_pushint(st, unit_walktobl(bl,map_id2bl(map_id),65025,1));
	}
	
	return true;
}

/// Kills the unit
///
/// unitkill <unit_id>;
BUILDIN(unitkill)
{
	struct block_list* bl = map_id2bl(script_getnum(st,2));
	if( bl != NULL )
		status_kill(bl);
	
	return true;
}

/// Warps the unit to the target position in the target map
/// Returns if it was successfull
///
/// unitwarp(<unit_id>,"<map name>",<x>,<y>) -> <bool>
BUILDIN(unitwarp)
{
	int unit_id;
	int map;
	short x;
	short y;
	struct block_list* bl;
	const char *mapname;
	
	unit_id = script_getnum(st,2);
	mapname = script_getstr(st, 3);
	x = (short)script_getnum(st,4);
	y = (short)script_getnum(st,5);
	
	if (!unit_id) //Warp the script's runner
		bl = map_id2bl(st->rid);
	else
		bl = map_id2bl(unit_id);
	
	if( strcmp(mapname,"this") == 0 )
		map = bl?bl->m:-1;
	else
		map = map_mapname2mapid(mapname);
	
	if( map >= 0 && bl != NULL )
		script_pushint(st, unit_warp(bl,map,x,y,CLR_OUTSIGHT));
	else
		script_pushint(st, 0);
	
	return true;
}

/// Makes the unit attack the target.
/// If the unit is a player and <action type> is not 0, it does a continuous
/// attack instead of a single attack.
/// Returns if the request was successfull.
///
/// unitattack(<unit_id>,"<target name>"{,<action type>}) -> <bool>
/// unitattack(<unit_id>,<target_id>{,<action type>}) -> <bool>
BUILDIN(unitattack)
{
	struct block_list* unit_bl;
	struct block_list* target_bl = NULL;
	struct script_data* data;
	int actiontype = 0;
	
	// get unit
	unit_bl = map_id2bl(script_getnum(st,2));
	if( unit_bl == NULL ) {
		script_pushint(st, 0);
		return true;
	}
	
	data = script_getdata(st, 3);
	get_val(st, data);
	if( data_isstring(data) )
	{
		TBL_PC* sd = map_nick2sd(script->conv_str(st, data));
		if( sd != NULL )
			target_bl = &sd->bl;
	} else
		target_bl = map_id2bl(script->conv_num(st, data));
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
			clif->pActionRequest_sub(((TBL_PC *)unit_bl), actiontype > 0 ? 0x07 : 0x00, target_bl->id, gettick());
			script_pushint(st, 1);
			return true;
		case BL_MOB:
			((TBL_MOB *)unit_bl)->target_id = target_bl->id;
			break;
		case BL_PET:
			((TBL_PET *)unit_bl)->target_id = target_bl->id;
			break;
		default:
			ShowError("script:unitattack: unsupported source unit type %d\n", unit_bl->type);
			script_pushint(st, 0);
			return false;
	}
	script_pushint(st, unit_walktobl(unit_bl, target_bl, 65025, 2));
	return true;
}

/// Makes the unit stop attacking and moving
///
/// unitstop <unit_id>;
BUILDIN(unitstop)
{
	int unit_id;
	struct block_list* bl;
	
	unit_id = script_getnum(st,2);
	
	bl = map_id2bl(unit_id);
	if( bl != NULL )
	{
		unit_stop_attack(bl);
		unit_stop_walking(bl,4);
		if( bl->type == BL_MOB )
			((TBL_MOB*)bl)->target_id = 0;
	}
	
	return true;
}

/// Makes the unit say the message
///
/// unittalk <unit_id>,"<message>";
BUILDIN(unittalk)
{
	int unit_id;
	const char* message;
	struct block_list* bl;
	
	unit_id = script_getnum(st,2);
	message = script_getstr(st, 3);
	
	bl = map_id2bl(unit_id);
	if( bl != NULL )
	{
		struct StringBuf sbuf;
		StrBuf->Init(&sbuf);
		StrBuf->Printf(&sbuf, "%s : %s", status_get_name(bl), message);
		clif->disp_overhead(bl, StrBuf->Value(&sbuf));
		if( bl->type == BL_PC )
			clif->message(((TBL_PC*)bl)->fd, StrBuf->Value(&sbuf));
		StrBuf->Destroy(&sbuf);
	}
	
	return true;
}

/// Makes the unit do an emotion
///
/// unitemote <unit_id>,<emotion>;
///
/// @see e_* in const.txt
BUILDIN(unitemote)
{
	int unit_id;
	int emotion;
	struct block_list* bl;
	
	unit_id = script_getnum(st,2);
	emotion = script_getnum(st,3);
	bl = map_id2bl(unit_id);
	if( bl != NULL )
		clif->emotion(bl, emotion);
	
	return true;
}

/// Makes the unit cast the skill on the target or self if no target is specified
///
/// unitskilluseid <unit_id>,<skill_id>,<skill_lv>{,<target_id>};
/// unitskilluseid <unit_id>,"<skill name>",<skill_lv>{,<target_id>};
BUILDIN(unitskilluseid)
{
	int unit_id;
	uint16 skill_id;
	uint16 skill_lv;
	int target_id;
	struct block_list* bl;
	
	unit_id  = script_getnum(st,2);
	skill_id = ( script_isstring(st,3) ? skill->name2id(script_getstr(st,3)) : script_getnum(st,3) );
	skill_lv = script_getnum(st,4);
	target_id = ( script_hasdata(st,5) ? script_getnum(st,5) : unit_id );
	
	bl = map_id2bl(unit_id);
	if( bl != NULL )
		unit_skilluse_id(bl, target_id, skill_id, skill_lv);
	
	return true;
}

/// Makes the unit cast the skill on the target position.
///
/// unitskillusepos <unit_id>,<skill_id>,<skill_lv>,<target_x>,<target_y>;
/// unitskillusepos <unit_id>,"<skill name>",<skill_lv>,<target_x>,<target_y>;
BUILDIN(unitskillusepos)
{
	int unit_id;
	uint16 skill_id;
	uint16 skill_lv;
	int skill_x;
	int skill_y;
	struct block_list* bl;
	
	unit_id  = script_getnum(st,2);
	skill_id = ( script_isstring(st,3) ? skill->name2id(script_getstr(st,3)) : script_getnum(st,3) );
	skill_lv = script_getnum(st,4);
	skill_x  = script_getnum(st,5);
	skill_y  = script_getnum(st,6);
	
	bl = map_id2bl(unit_id);
	if( bl != NULL )
		unit_skilluse_pos(bl, skill_x, skill_y, skill_id, skill_lv);
	
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
	script_detach_rid(st);
	
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
BUILDIN(sleep2)
{
	int ticks;
	
	ticks = script_getnum(st,2);
	
	if( ticks <= 0 )
	{// do nothing
		script_pushint(st, (map_id2sd(st->rid)!=NULL));
	}
	else if( !st->sleep.tick )
	{// sleep for the target amount of time
		st->state = RERUNLINE;
		st->sleep.tick = ticks;
	}
	else
	{// sleep time is over
		st->state = RUN;
		st->sleep.tick = 0;
		script_pushint(st, (map_id2sd(st->rid)!=NULL));
	}
	return true;
}

/// Awakes all the sleep timers of the target npc
///
/// awake "<npc name>";
BUILDIN(awake)
{
	struct npc_data* nd;
	struct linkdb_node *node = (struct linkdb_node *)sleep_db;
	
	nd = npc_name2id(script_getstr(st, 2));
	if( nd == NULL ) {
		ShowError("awake: NPC \"%s\" not found\n", script_getstr(st, 2));
		return false;
	}
	
	while( node )
	{
		if( (int)__64BPTRSIZE(node->key) == nd->bl.id )
		{// sleep timer for the npc
			struct script_state* tst = (struct script_state*)node->data;
			TBL_PC* sd = map_id2sd(tst->rid);
			
			if( tst->sleep.timer == INVALID_TIMER )
			{// already awake ???
				node = node->next;
				continue;
			}
			if( (sd && sd->status.char_id != tst->sleep.charid) || (tst->rid && !sd))
			{// char not online anymore / another char of the same account is online - Cancel execution
				tst->state = END;
				tst->rid = 0;
			}
			
			delete_timer(tst->sleep.timer, run_script_timer);
			node = script_erase_sleepdb(node);
			tst->sleep.timer = INVALID_TIMER;
			if(tst->state != RERUNLINE)
				tst->sleep.tick = 0;
			run_script_main(tst);
		}
		else
		{
			node = node->next;
		}
	}
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
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return false;
	}
	
	name = reference_getname(data);
	if( *name != '.' || name[1] == '@' )
	{// not a npc variable
		ShowError("script:getvariableofnpc: invalid scope (not npc variable)\n");
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return false;
	}
	
	nd = npc_name2id(script_getstr(st,3));
	if( nd == NULL || nd->subtype != SCRIPT || nd->u.scr.script == NULL )
	{// NPC not found or has no script
		ShowError("script:getvariableofnpc: can't find npc %s\n", script_getstr(st,3));
		script_pushnil(st);
		st->state = END;
		return false;
	}
	
	push_val2(st->stack, C_NAME, reference_getuid(data), &nd->u.scr.script->script_vars );
	return true;
}

/// Opens a warp portal.
/// Has no "portal opening" effect/sound, it opens the portal immediately.
///
/// warpportal <source x>,<source y>,"<target map>",<target x>,<target y>;
///
/// @author blackhole89
BUILDIN(warpportal)
{
	int spx;
	int spy;
	unsigned short mapindex;
	int tpx;
	int tpy;
	struct skill_unit_group* group;
	struct block_list* bl;
	
	bl = map_id2bl(st->oid);
	if( bl == NULL )
	{
		ShowError("script:warpportal: npc is needed\n");
		return false;
	}
	
	spx = script_getnum(st,2);
	spy = script_getnum(st,3);
	mapindex = mapindex_name2id(script_getstr(st, 4));
	tpx = script_getnum(st,5);
	tpy = script_getnum(st,6);
	
	if( mapindex == 0 )
		return true;// map not found
	
	group = skill->unitsetting(bl, AL_WARP, 4, spx, spy, 0);
	if( group == NULL )
		return true;// failed
	group->val1 = (group->val1<<16)|(short)0;
	group->val2 = (tpx<<16) | tpy;
	group->val3 = mapindex;
	
	return true;
}

BUILDIN(openmail)
{
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	mail_openmail(sd);
	
	return true;
}

BUILDIN(openauction)
{
	TBL_PC* sd;
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return true;
	
	clif->auction_openwindow(sd);
	
	return true;
}

/// Retrieves the value of the specified flag of the specified cell.
///
/// checkcell("<map name>",<x>,<y>,<type>) -> <bool>
///
/// @see cell_chk* constants in const.txt for the types
BUILDIN(checkcell)
{
	int16 m = map_mapname2mapid(script_getstr(st,2));
	int16 x = script_getnum(st,3);
	int16 y = script_getnum(st,4);
	cell_chk type = (cell_chk)script_getnum(st,5);
	
	script_pushint(st, map_getcell(m, x, y, type));
	
	return true;
}

/// Modifies flags of cells in the specified area.
///
/// setcell "<map name>",<x1>,<y1>,<x2>,<y2>,<type>,<flag>;
///
/// @see cell_* constants in const.txt for the types
BUILDIN(setcell)
{
	int16 m = map_mapname2mapid(script_getstr(st,2));
	int16 x1 = script_getnum(st,3);
	int16 y1 = script_getnum(st,4);
	int16 x2 = script_getnum(st,5);
	int16 y2 = script_getnum(st,6);
	cell_t type = (cell_t)script_getnum(st,7);
	bool flag = (bool)script_getnum(st,8);
	
	int x,y;
	
	if( x1 > x2 ) swap(x1,x2);
	if( y1 > y2 ) swap(y1,y2);
	
	for( y = y1; y <= y2; ++y )
		for( x = x1; x <= x2; ++x )
			map[m].setcell(m, x, y, type, flag);
	
	return true;
}

/*==========================================
 * Mercenary Commands
 *------------------------------------------*/
BUILDIN(mercenary_create)
{
	struct map_session_data *sd;
	int class_, contract_time;
	
	if( (sd = script_rid2sd(st)) == NULL || sd->md || sd->status.mer_id != 0 )
		return true;
	
	class_ = script_getnum(st,2);
	
	if( !merc_class(class_) )
		return true;
	
	contract_time = script_getnum(st,3);
	merc_create(sd, class_, contract_time);
	return true;
}

BUILDIN(mercenary_heal)
{
	struct map_session_data *sd = script_rid2sd(st);
	int hp, sp;
	
	if( sd == NULL || sd->md == NULL )
		return true;
	hp = script_getnum(st,2);
	sp = script_getnum(st,3);
	
	status_heal(&sd->md->bl, hp, sp, 0);
	return true;
}

BUILDIN(mercenary_sc_start)
{
	struct map_session_data *sd = script_rid2sd(st);
	enum sc_type type;
	int tick, val1;
	
	if( sd == NULL || sd->md == NULL )
		return true;
	
	type = (sc_type)script_getnum(st,2);
	tick = script_getnum(st,3);
	val1 = script_getnum(st,4);
	
	status_change_start(&sd->md->bl, type, 10000, val1, 0, 0, 0, tick, 2);
	return true;
}

BUILDIN(mercenary_get_calls)
{
	struct map_session_data *sd = script_rid2sd(st);
	int guild;
	
	if( sd == NULL )
		return true;
	
	guild = script_getnum(st,2);
	switch( guild )
	{
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

BUILDIN(mercenary_set_calls)
{
	struct map_session_data *sd = script_rid2sd(st);
	int guild, value, *calls;
	
	if( sd == NULL )
		return true;
	
	guild = script_getnum(st,2);
	value = script_getnum(st,3);
	
	switch( guild )
	{
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

BUILDIN(mercenary_get_faith)
{
	struct map_session_data *sd = script_rid2sd(st);
	int guild;
	
	if( sd == NULL )
		return true;
	
	guild = script_getnum(st,2);
	switch( guild )
	{
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

BUILDIN(mercenary_set_faith)
{
	struct map_session_data *sd = script_rid2sd(st);
	int guild, value, *calls;
	
	if( sd == NULL )
		return true;
	
	guild = script_getnum(st,2);
	value = script_getnum(st,3);
	
	switch( guild )
	{
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
	if( mercenary_get_guild(sd->md) == guild )
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
	
	if( (sd = script_rid2sd(st)) == NULL )
		return true;
	
	book_id = script_getnum(st,2);
	page = script_getnum(st,3);
	
	clif->readbook(sd->fd, book_id, page);
	return true;
}

/******************
 Questlog script commands
 *******************/

BUILDIN(setquest)
{
	struct map_session_data *sd = script_rid2sd(st);
	nullpo_ret(sd);
	
	quest_add(sd, script_getnum(st, 2));
	return true;
}

BUILDIN(erasequest)
{
	struct map_session_data *sd = script_rid2sd(st);
	nullpo_ret(sd);
	
	quest_delete(sd, script_getnum(st, 2));
	return true;
}

BUILDIN(completequest)
{
	struct map_session_data *sd = script_rid2sd(st);
	nullpo_ret(sd);
	
	quest_update_status(sd, script_getnum(st, 2), Q_COMPLETE);
	return true;
}

BUILDIN(changequest)
{
	struct map_session_data *sd = script_rid2sd(st);
	nullpo_ret(sd);
	
	quest_change(sd, script_getnum(st, 2),script_getnum(st, 3));
	return true;
}

BUILDIN(checkquest)
{
	struct map_session_data *sd = script_rid2sd(st);
	quest_check_type type = HAVEQUEST;
	
	nullpo_ret(sd);
	
	if( script_hasdata(st, 3) )
		type = (quest_check_type)script_getnum(st, 3);
	
	script_pushint(st, quest_check(sd, script_getnum(st, 2), type));
	
	return true;
}

BUILDIN(showevent)
{
	TBL_PC *sd = script_rid2sd(st);
	struct npc_data *nd = map_id2nd(st->oid);
	int state, color;
	
	if( sd == NULL || nd == NULL )
		return true;
	state = script_getnum(st, 2);
	color = script_getnum(st, 3);
	
	if( color < 0 || color > 3 )
		color = 0; // set default color
	
	clif->quest_show_event(sd, &nd->bl, state, color);
	return true;
}

/*==========================================
 * BattleGround System
 *------------------------------------------*/
BUILDIN(waitingroom2bg)
{
	struct npc_data *nd;
	struct chat_data *cd;
	const char *map_name, *ev = "", *dev = "";
	int x, y, i, mapindex = 0, bg_id, n;
	struct map_session_data *sd;
	
	if( script_hasdata(st,7) )
		nd = npc_name2id(script_getstr(st,7));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);
	
	if( nd == NULL || (cd = (struct chat_data *)map_id2bl(nd->chat_id)) == NULL )
	{
		script_pushint(st,0);
		return true;
	}
	
	map_name = script_getstr(st,2);
	if( strcmp(map_name,"-") != 0 )
	{
		mapindex = mapindex_name2id(map_name);
		if( mapindex == 0 )
		{ // Invalid Map
			script_pushint(st,0);
			return true;
		}
	}
	
	x = script_getnum(st,3);
	y = script_getnum(st,4);
	ev = script_getstr(st,5); // Logout Event
	dev = script_getstr(st,6); // Die Event
	
	if( (bg_id = bg_create(mapindex, x, y, ev, dev)) == 0 )
	{ // Creation failed
		script_pushint(st,0);
		return true;
	}
	
	n = cd->users;
	for( i = 0; i < n && i < MAX_BG_MEMBERS; i++ )
	{
		if( (sd = cd->usersd[i]) != NULL && bg_team_join(bg_id, sd) )
			mapreg_setreg(reference_uid(add_str("$@arenamembers"), i), sd->bl.id);
		else
			mapreg_setreg(reference_uid(add_str("$@arenamembers"), i), 0);
	}
	
	mapreg_setreg(add_str("$@arenamembersnum"), i);
	script_pushint(st,bg_id);
	return true;
}

BUILDIN(waitingroom2bg_single)
{
	const char* map_name;
	struct npc_data *nd;
	struct chat_data *cd;
	struct map_session_data *sd;
	int x, y, mapindex, bg_id;
	
	bg_id = script_getnum(st,2);
	map_name = script_getstr(st,3);
	if( (mapindex = mapindex_name2id(map_name)) == 0 )
		return true; // Invalid Map
	
	x = script_getnum(st,4);
	y = script_getnum(st,5);
	nd = npc_name2id(script_getstr(st,6));
	
	if( nd == NULL || (cd = (struct chat_data *)map_id2bl(nd->chat_id)) == NULL || cd->users <= 0 )
		return true;
	
	if( (sd = cd->usersd[0]) == NULL )
		return true;
	
	if( bg_team_join(bg_id, sd) )
	{
		pc_setpos(sd, mapindex, x, y, CLR_TELEPORT);
		script_pushint(st,1);
	}
	else
		script_pushint(st,0);
	
	return true;
}

BUILDIN(bg_team_setxy)
{
	struct battleground_data *bg;
	int bg_id;
	
	bg_id = script_getnum(st,2);
	if( (bg = bg_team_search(bg_id)) == NULL )
		return true;
	
	bg->x = script_getnum(st,3);
	bg->y = script_getnum(st,4);
	return true;
}

BUILDIN(bg_warp)
{
	int x, y, mapindex, bg_id;
	const char* map_name;
	
	bg_id = script_getnum(st,2);
	map_name = script_getstr(st,3);
	if( (mapindex = mapindex_name2id(map_name)) == 0 )
		return true; // Invalid Map
	x = script_getnum(st,4);
	y = script_getnum(st,5);
	bg_team_warp(bg_id, mapindex, x, y);
	return true;
}

BUILDIN(bg_monster)
{
	int class_ = 0, x = 0, y = 0, bg_id = 0;
	const char *str,*map, *evt="";
	
	bg_id  = script_getnum(st,2);
	map    = script_getstr(st,3);
	x      = script_getnum(st,4);
	y      = script_getnum(st,5);
	str    = script_getstr(st,6);
	class_ = script_getnum(st,7);
	if( script_hasdata(st,8) ) evt = script_getstr(st,8);
	check_event(st, evt);
	script_pushint(st, mob_spawn_bg(map,x,y,str,class_,evt,bg_id));
	return true;
}

BUILDIN(bg_monster_set_team)
{
	struct mob_data *md;
	struct block_list *mbl;
	int id = script_getnum(st,2),
	bg_id = script_getnum(st,3);
	
	if( (mbl = map_id2bl(id)) == NULL || mbl->type != BL_MOB )
		return true;
	md = (TBL_MOB *)mbl;
	md->bg_id = bg_id;
	
	mob_stop_attack(md);
	mob_stop_walking(md, 0);
	md->target_id = md->attacked_id = 0;
	clif->charnameack(0, &md->bl);
	
	return true;
}

BUILDIN(bg_leave)
{
	struct map_session_data *sd = script_rid2sd(st);
	if( sd == NULL || !sd->bg_id )
		return true;
	
	bg_team_leave(sd,0);
	return true;
}

BUILDIN(bg_destroy)
{
	int bg_id = script_getnum(st,2);
	bg_team_delete(bg_id);
	return true;
}

BUILDIN(bg_getareausers)
{
	const char *str;
	int16 m, x0, y0, x1, y1;
	int bg_id;
	int i = 0, c = 0;
	struct battleground_data *bg = NULL;
	struct map_session_data *sd;
	
	bg_id = script_getnum(st,2);
	str = script_getstr(st,3);
	
	if( (bg = bg_team_search(bg_id)) == NULL || (m = map_mapname2mapid(str)) < 0 )
	{
		script_pushint(st,0);
		return true;
	}
	
	x0 = script_getnum(st,4);
	y0 = script_getnum(st,5);
	x1 = script_getnum(st,6);
	y1 = script_getnum(st,7);
	
	for( i = 0; i < MAX_BG_MEMBERS; i++ )
	{
		if( (sd = bg->members[i].sd) == NULL )
			continue;
		if( sd->bl.m != m || sd->bl.x < x0 || sd->bl.y < y0 || sd->bl.x > x1 || sd->bl.y > y1 )
			continue;
		c++;
	}
	
	script_pushint(st,c);
	return true;
}

BUILDIN(bg_updatescore)
{
	const char *str;
	int16 m;
	
	str = script_getstr(st,2);
	if( (m = map_mapname2mapid(str)) < 0 )
		return true;
	
	map[m].bgscore_lion = script_getnum(st,3);
	map[m].bgscore_eagle = script_getnum(st,4);
	
	clif->bg_updatescore(m);
	return true;
}

BUILDIN(bg_get_data)
{
	struct battleground_data *bg;
	int bg_id = script_getnum(st,2),
	type = script_getnum(st,3);
	
	if( (bg = bg_team_search(bg_id)) == NULL )
	{
		script_pushint(st,0);
		return true;
	}
	
	switch( type )
	{
		case 0: script_pushint(st, bg->count); break;
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

BUILDIN(instance_attachmap) {
	const char *name, *map_name = NULL;
	int16 m;
	int instance_id = -1;
	bool usebasename = false;
	
	name = script_getstr(st,2);
	instance_id = script_getnum(st,3);
	if( script_hasdata(st,4) && script_getnum(st,4) > 0 )
		usebasename = true;
	
	if( script_hasdata(st, 5) )
		map_name = script_getstr(st, 5);
	
	if( (m = instance->add_map(name, instance_id, usebasename, map_name)) < 0 ) { // [Saithis]
		ShowError("buildin_instance_attachmap: instance creation failed (%s): %d\n", name, m);
		script_pushconststr(st, "");
		return true;
	}
	script_pushconststr(st, map[m].name);
	
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
	
	if( (m = map_mapname2mapid(str)) < 0 || (m = instance->map2imap(m,instance_id)) < 0 ) {
		ShowError("buildin_instance_detachmap: Trying to detach invalid map %s\n", str);
		return true;
	}
	
	instance->del_map(m);
	return true;
}

BUILDIN(instance_attach) {
	int instance_id = -1;
	
	instance_id = script_getnum(st, 2);
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
	
	if( instances[instance_id].state != INSTANCE_IDLE ) {
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
	
	for( i = 0; i < instances[instance_id].num_map; i++ )
		map_foreachinmap(buildin_announce_sub, instances[instance_id].map[i], BL_PC,
						 mes, strlen(mes)+1, flag&0xf0, fontColor, fontType, fontSize, fontAlign, fontY);
	
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
	
	if( instance_id >= 0 && (nd = npc_name2id(str)) != NULL ) {
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
	
 	str = script_getstr(st, 2);
	
	if( (m = map_mapname2mapid(str)) < 0 ) {
		script_pushconststr(st, "");
		return true;
	}
	
	if( script_hasdata(st, 3) )
		instance_id = script_getnum(st, 3);
	else if( st->instance_id >= 0 )
		instance_id = st->instance_id;
	else if( (sd = script_rid2sd(st)) != NULL ) {
		struct party_data *p;
		int i = 0, j = 0;
		if( sd->instances ) {
			for( i = 0; i < sd->instances; i++ ) {
				ARR_FIND(0, instances[sd->instance[i]].num_map, j, map[instances[sd->instance[i]].map[j]].instance_src_map == m);
				if( j != instances[sd->instance[i]].num_map )
					break;
			}
			if( i != sd->instances )
				instance_id = sd->instance[i];
		}
		if( instance_id == -1 && sd->status.party_id && (p = party_search(sd->status.party_id)) && p->instances ) {
			for( i = 0; i < p->instances; i++ ) {
				ARR_FIND(0, instances[p->instance[i]].num_map, j, map[instances[p->instance[i]].map[j]].instance_src_map == m);
				if( j != instances[p->instance[i]].num_map )
					break;
			}
			if( i != p->instances )
				instance_id = p->instance[i];
		}
		if( instance_id == -1 && sd->guild && sd->guild->instances ) {
			for( i = 0; i < sd->guild->instances; i++ ) {
				ARR_FIND(0, instances[sd->guild->instance[i]].num_map, j, map[instances[sd->guild->instance[i]].map[j]].instance_src_map == m);
				if( j != instances[sd->guild->instance[i]].num_map )
					break;
			}
			if( i != sd->guild->instances )
				instance_id = sd->guild->instance[i];
		}
	}
	
	if( !instance->valid(instance_id) || (m = instance->map2imap(m, instance_id)) < 0 ) {
		script_pushconststr(st, "");
		return true;
	}
	
	script_pushconststr(st, map[m].name);
	return true;
}
static int buildin_instance_warpall_sub(struct block_list *bl,va_list ap) {
	struct map_session_data *sd = ((TBL_PC*)bl);
	int mapindex = va_arg(ap,int);
	int x = va_arg(ap,int);
	int y = va_arg(ap,int);
	
	pc_setpos(sd,mapindex,x,y,CLR_TELEPORT);
	
	return 0;
}
BUILDIN(instance_warpall) {
	int16 m;
	int instance_id = -1;
	const char *mapn;
	int x, y;
	int mapindex;
	
	mapn = script_getstr(st,2);
	x    = script_getnum(st,3);
	y    = script_getnum(st,4);
	
	if( script_hasdata(st,5) )
		instance_id = script_getnum(st,5);
	else if( st->instance_id >= 0 )
		instance_id = st->instance_id;
	else
		return true;
	
	if( (m = map_mapname2mapid(mapn)) < 0 || (map[m].flag.src4instance && (m = instance->mapid2imapid(m, instance_id)) < 0) )
		return true;
		
	mapindex = map_id2index(m);
	
	map_foreachininstance(buildin_instance_warpall_sub, instance_id, BL_PC,mapindex,x,y);

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
BUILDIN(instance_check_party) {
	struct map_session_data *pl_sd;
	int amount, min, max, i, party_id, c = 0;
	struct party_data *p = NULL;
	
	amount = script_hasdata(st,3) ? script_getnum(st,3) : 1; // Amount of needed Partymembers for the Instance.
	min = script_hasdata(st,4) ? script_getnum(st,4) : 1; // Minimum Level needed to join the Instance.
	max  = script_hasdata(st,5) ? script_getnum(st,5) : MAX_LEVEL; // Maxium Level allowed to join the Instance.
	
	if( min < 1 || min > MAX_LEVEL){
		ShowError("instance_check_party: Invalid min level, %d\n", min);
		return true;
	}else if(  max < 1 || max > MAX_LEVEL){
		ShowError("instance_check_party: Invalid max level, %d\n", max);
		return true;
	}
	
	if( script_hasdata(st,2) )
		party_id = script_getnum(st,2);
	else return true;
	
	if( !(p = party_search(party_id)) ){
		script_pushint(st, 0); // Returns false if party does not exist.
		return true;
	}
	
	for( i = 0; i < MAX_PARTY; i++ )
		if( (pl_sd = p->data[i].sd) )
			if(map_id2bl(pl_sd->bl.id)){
				if(pl_sd->status.base_level < min){
					script_pushint(st, 0);
					return true;
				}else if(pl_sd->status.base_level > max){
					script_pushint(st, 0);
					return true;
				}
				c++;
			}
	
	if(c < amount){
		script_pushint(st, 0); // Not enough Members in the Party to join Instance.
	}else
		script_pushint(st, 1);
	
	return true;
}

/*==========================================
 * Custom Fonts
 *------------------------------------------*/
BUILDIN(setfont)
{
	struct map_session_data *sd = script_rid2sd(st);
	int font = script_getnum(st,2);
	if( sd == NULL )
		return true;
	
	if( sd->user_font != font )
		sd->user_font = font;
	else
		sd->user_font = 0;
	
	clif->font(sd);
	return true;
}

static int buildin_mobuseskill_sub(struct block_list *bl,va_list ap)
{
	TBL_MOB* md		= (TBL_MOB*)bl;
	struct block_list *tbl;
	int mobid		= va_arg(ap,int);
	uint16 skill_id		= va_arg(ap,int);
	uint16 skill_lv		= va_arg(ap,int);
	int casttime	= va_arg(ap,int);
	int cancel		= va_arg(ap,int);
	int emotion		= va_arg(ap,int);
	int target		= va_arg(ap,int);
	
	if( md->class_ != mobid )
		return 0;
	
	// 0:self, 1:target, 2:master, default:random
	switch( target )
	{
		case 0: tbl = map_id2bl(md->bl.id); break;
		case 1: tbl = map_id2bl(md->target_id); break;
		case 2: tbl = map_id2bl(md->master_id); break;
		default:tbl = battle->get_enemy(&md->bl, DEFAULT_ENEMY_TYPE(md),skill->get_range2(&md->bl, skill_id, skill_lv)); break;
	}
	
	if( !tbl )
		return 0;
	
	if( md->ud.skilltimer != INVALID_TIMER ) // Cancel the casting skill.
		unit_skillcastcancel(bl,0);
	
	if( skill->get_casttype(skill_id) == CAST_GROUND )
		unit_skilluse_pos2(&md->bl, tbl->x, tbl->y, skill_id, skill_lv, casttime, cancel);
	else
		unit_skilluse_id2(&md->bl, tbl->id, skill_id, skill_lv, casttime, cancel);
	
	clif->emotion(&md->bl, emotion);
	
	return 0;
}
/*==========================================
 * areamobuseskill "Map Name",<x>,<y>,<range>,<Mob ID>,"Skill Name"/<Skill ID>,<Skill Lv>,<Cast Time>,<Cancelable>,<Emotion>,<Target Type>;
 *------------------------------------------*/
BUILDIN(areamobuseskill)
{
	struct block_list center;
	int16 m;
	int range,mobid,skill_id,skill_lv,casttime,emotion,target,cancel;
	
	if( (m = map_mapname2mapid(script_getstr(st,2))) < 0 ) {
		ShowError("areamobuseskill: invalid map name.\n");
		return true;
	}
	
	if( map[m].flag.src4instance && st->instance_id >= 0 && (m = instance->mapid2imapid(m, st->instance_id)) < 0 )
		return true;
	
	center.m = m;
	center.x = script_getnum(st,3);
	center.y = script_getnum(st,4);
	range = script_getnum(st,5);
	mobid = script_getnum(st,6);
	skill_id = ( script_isstring(st,7) ? skill->name2id(script_getstr(st,7)) : script_getnum(st,7) );
	skill_lv = script_getnum(st,8);
	casttime = script_getnum(st,9);
	cancel = script_getnum(st,10);
	emotion = script_getnum(st,11);
	target = script_getnum(st,12);
	
	map_foreachinrange(buildin_mobuseskill_sub, &center, range, BL_MOB, mobid, skill_id, skill_lv, casttime, cancel, emotion, target);
	return true;
}


BUILDIN(progressbar)
{
	struct map_session_data * sd = script_rid2sd(st);
	const char * color;
	unsigned int second;
	
	if( !st || !sd )
		return true;
	
	st->state = STOP;
	
	color = script_getstr(st,2);
	second = script_getnum(st,3);
	
	sd->progressbar.npc_id = st->oid;
	sd->progressbar.timeout = gettick() + second*1000;
	
	clif->progressbar(sd, strtol(color, (char **)NULL, 0), second);
    return true;
}

BUILDIN(pushpc)
{
	uint8 dir;
	int cells, dx, dy;
	struct map_session_data* sd;
	
	if((sd = script_rid2sd(st))==NULL)
	{
		return true;
	}
	
	dir = script_getnum(st,2);
	cells     = script_getnum(st,3);
	
	if(dir>7)
	{
		ShowWarning("buildin_pushpc: Invalid direction %d specified.\n", dir);
		script_reportsrc(st);
		
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
	
	unit_blown(&sd->bl, dx, dy, cells, 0);
	return true;
}


/// Invokes buying store preparation window
/// buyingstore <slots>;
BUILDIN(buyingstore)
{
	struct map_session_data* sd;
	
	if( ( sd = script_rid2sd(st) ) == NULL ) {
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
	
	if( ( sd = script_rid2sd(st) ) == NULL )
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
	
	if( ( sd = script_rid2sd(st) ) == NULL )
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
BUILDIN(makerune) {
	TBL_PC* sd;
	if( (sd = script_rid2sd(st)) == NULL )
		return true;
	clif->skill_produce_mix_list(sd,RK_RUNEMASTERY,24);
	sd->itemid = script_getnum(st,2);
	return true;
}
/**
 * checkdragon() returns 1 if mounting a dragon or 0 otherwise.
 **/
BUILDIN(checkdragon) {
	TBL_PC* sd;
	if( (sd = script_rid2sd(st)) == NULL )
		return true;
	if( pc_isridingdragon(sd) )
		script_pushint(st,1);
	else
		script_pushint(st,0);
	return true;
}
/**
 * setdragon({optional Color}) returns 1 on success or 0 otherwise
 * - Toggles the dragon on a RK if he can mount;
 * @param Color - when not provided uses the green dragon;
 * - 1 : Green Dragon
 * - 2 : Brown Dragon
 * - 3 : Gray Dragon
 * - 4 : Blue Dragon
 * - 5 : Red Dragon
 **/
BUILDIN(setdragon) {
	TBL_PC* sd;
	int color = script_hasdata(st,2) ? script_getnum(st,2) : 0;
	
	if( (sd = script_rid2sd(st)) == NULL )
		return true;
	if( !pc_checkskill(sd,RK_DRAGONTRAINING) || (sd->class_&MAPID_THIRDMASK) != MAPID_RUNE_KNIGHT )
		script_pushint(st,0);//Doesn't have the skill or it's not a Rune Knight
	else if ( pc_isridingdragon(sd) ) {//Is mounted; release
		pc_setoption(sd, sd->sc.option&~OPTION_DRAGON);
		script_pushint(st,1);
	} else {//Not mounted; Mount now.
		unsigned int option = OPTION_DRAGON1;
		if( color ) {
			option = ( color == 1 ? OPTION_DRAGON1 :
					  color == 2 ? OPTION_DRAGON2 :
					  color == 3 ? OPTION_DRAGON3 :
					  color == 4 ? OPTION_DRAGON4 :
					  color == 5 ? OPTION_DRAGON5 : 0);
			if( !option ) {
				ShowWarning("script_setdragon: Unknown Color %d used; changing to green (1)\n",color);
				option = OPTION_DRAGON1;
			}
		}
		pc_setoption(sd, sd->sc.option|option);
		script_pushint(st,1);
	}
	return true;
}

/**
 * ismounting() returns 1 if mounting a new mount or 0 otherwise
 **/
BUILDIN(ismounting) {
	TBL_PC* sd;
	if( (sd = script_rid2sd(st)) == NULL )
		return true;
	if( sd->sc.data[SC_ALL_RIDING] )
		script_pushint(st,1);
	else
		script_pushint(st,0);
	return true;
}

/**
 * setmounting() returns 1 on success or 0 otherwise
 * - Toggles new mounts on a player when he can mount
 * - Will fail if the player is mounting a non-new mount, e.g. dragon, peco, wug, etc.
 * - Will unmount the player is he is already mounting
 **/
BUILDIN(setmounting) {
	TBL_PC* sd;
	if( (sd = script_rid2sd(st)) == NULL )
		return true;
	if( sd->sc.option&(OPTION_WUGRIDER|OPTION_RIDING|OPTION_DRAGON|OPTION_MADOGEAR) )
		script_pushint(st,0);//can't mount with one of these
	else {
		if( sd->sc.data[SC_ALL_RIDING] )
			status_change_end(&sd->bl, SC_ALL_RIDING, INVALID_TIMER);
		else
			sc_start(&sd->bl, SC_ALL_RIDING, 100, 0, -1);
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
BUILDIN(getcharip)
{
	struct map_session_data* sd = NULL;
	
	/* check if a character name is specified */
	if( script_hasdata(st, 2) )
	{
		if (script_isstring(st, 2))
			sd = map_nick2sd(script_getstr(st, 2));
		else if (script_isint(st, 2) || script_getnum(st, 2))
		{
			int id;
			id = script_getnum(st, 2);
			sd = (map_id2sd(id) ? map_id2sd(id) : map_charid2sd(id));
		}
	}
	else
		sd = script_rid2sd(st);
	
	/* check for sd and IP */
	if (!sd || !session[sd->fd]->client_addr)
	{
		script_pushconststr(st, "");
		return true;
	}
	
	/* return the client ip_addr converted for output */
	if (sd && sd->fd && session[sd->fd])
	{
		/* initiliaze */
		const char *ip_addr = NULL;
		uint32 ip;
		
		/* set ip, ip_addr and convert to ip and push str */
		ip = session[sd->fd]->client_addr;
		ip_addr = ip2str(ip, NULL);
		script_pushstrcopy(st, ip_addr);
	}
	
	return true;
}
/**
 * is_function(<function name>) -> 1 if function exists, 0 otherwise
 **/
BUILDIN(is_function) {
	const char* str = script_getstr(st,2);
	
	if( strdb_exists(userfunc_db, str) )
		script_pushint(st,1);
	else
		script_pushint(st,0);
	
	return true;
}
/**
 * get_revision() -> retrieves the current svn revision (if available)
 **/
BUILDIN(get_revision) {
	const char *svn = get_svn_revision();
	
	if ( svn[0] != HERC_UNKNOWN_VER )
		script_pushint(st,atoi(svn));
	else
		script_pushint(st,-1);//unknown
	
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

BUILDIN(useatcmd)
{
	TBL_PC dummy_sd;
	TBL_PC* sd;
	int fd;
	const char* cmd;
	
	cmd = script_getstr(st,2);
	
	if( st->rid )
	{
		sd = script_rid2sd(st);
		fd = sd->fd;
	}
	else
	{ // Use a dummy character.
		sd = &dummy_sd;
		fd = 0;
		
		memset(&dummy_sd, 0, sizeof(TBL_PC));
		if( st->oid )
		{
			struct block_list* bl = map_id2bl(st->oid);
			memcpy(&dummy_sd.bl, bl, sizeof(struct block_list));
			if( bl->type == BL_NPC )
				safestrncpy(dummy_sd.status.name, ((TBL_NPC*)bl)->name, NAME_LENGTH);
		}
	}
	
	// compatibility with previous implementation (deprecated!)
	if( cmd[0] != atcommand->at_symbol ) {
		cmd += strlen(sd->status.name);
		while( *cmd != atcommand->at_symbol && *cmd != 0 )
			cmd++;
	}
	
	atcommand->parse(fd, sd, cmd, 1);
	return true;
}

BUILDIN(checkre)
{
	int num;
	
	num=script_getnum(st,2);
	switch(num){
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

/* getrandgroupitem <group_id>,<quantity> */
BUILDIN(getrandgroupitem) {
	TBL_PC* sd;
	int i, get_count = 0, flag, nameid, group = script_getnum(st, 2), qty = script_getnum(st,3);
	struct item item_tmp;
	
	if( !( sd = script_rid2sd(st) ) )
		return true;
	
	if( qty <= 0 ) {
		ShowError("getrandgroupitem: qty is <= 0!\n");
		return false;
	}
	
	if(group < 1 || group >= MAX_ITEMGROUP) {
		ShowError("getrandgroupitem: Invalid group id %d\n", group);
		return false;
	}
	if (!itemgroup_db[group].qty) {
		ShowError("getrandgroupitem: group id %d is empty!\n", group);
		return false;
	}
	
	nameid = itemdb_searchrandomid(group);
	memset(&item_tmp,0,sizeof(item_tmp));
	
	item_tmp.nameid   = nameid;
	item_tmp.identify = itemdb_isidentified(nameid);
	
	//Check if it's stackable.
	if (!itemdb_isstackable(nameid))
		get_count = 1;
	else
		get_count = qty;
	
	for (i = 0; i < qty; i += get_count) {
		// if not pet egg
		if (!pet_create_egg(sd, nameid)) {
			if ((flag = pc_additem(sd, &item_tmp, get_count, LOG_TYPE_SCRIPT))) {
				clif->additem(sd, 0, 0, flag);
				if( pc_candrop(sd,&item_tmp) )
					map_addflooritem(&item_tmp,get_count,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
			}
		}
	}
	
	return true;
}

/* cleanmap <map_name>;
 * cleanarea <map_name>, <x0>, <y0>, <x1>, <y1>; */
static int atcommand_cleanfloor_sub(struct block_list *bl, va_list ap)
{
	nullpo_ret(bl);
	map_clearflooritem(bl);
	
	return 0;
}

BUILDIN(cleanmap)
{
    const char *map;
    int16 m = -1;
    int16 x0 = 0, y0 = 0, x1 = 0, y1 = 0;
	
    map = script_getstr(st, 2);
    m = map_mapname2mapid(map);
    if (!m)
        return false;
	
	if ((script_lastdata(st) - 2) < 4) {
		map_foreachinmap(atcommand_cleanfloor_sub, m, BL_ITEM);
	} else {
		x0 = script_getnum(st, 3);
		y0 = script_getnum(st, 4);
		x1 = script_getnum(st, 5);
		y1 = script_getnum(st, 6);
		if (x0 > 0 && y0 > 0 && x1 > 0 && y1 > 0) {
			map_foreachinarea(atcommand_cleanfloor_sub, m, x0, y0, x1, y1, BL_ITEM);
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
	uint16 skill_id;
	unsigned short skill_level;
	unsigned int stat_point;
	unsigned int npc_level;
	struct npc_data *nd;
	struct map_session_data *sd;
	
	skill_id	= script_isstring(st, 2) ? skill->name2id(script_getstr(st, 2)) : script_getnum(st, 2);
	skill_level	= script_getnum(st, 3);
	stat_point	= script_getnum(st, 4);
	npc_level	= script_getnum(st, 5);
	sd			= script_rid2sd(st);
	nd			= (struct npc_data *)map_id2bl(sd->npc_id);
	
	if (stat_point > battle_config.max_third_parameter) {
		ShowError("npcskill: stat point exceeded maximum of %d.\n",battle_config.max_third_parameter );
		return false;
	}
	if (npc_level > MAX_LEVEL) {
		ShowError("npcskill: level exceeded maximum of %d.\n", MAX_LEVEL);
		return false;
	}
	if (sd == NULL || nd == NULL) { //ain't possible, but I don't trust people.
		return false;
	}
	
	nd->level = npc_level;
	nd->stat_point = stat_point;
	
	if (!nd->status.hp) {
		status_calc_npc(nd, true);
	} else {
		status_calc_npc(nd, false);
	}
	
	if (skill->get_inf(skill_id)&INF_GROUND_SKILL) {
		unit_skilluse_pos(&nd->bl, sd->bl.x, sd->bl.y, skill_id, skill_level);
	} else {
		unit_skilluse_id(&nd->bl, sd->bl.id, skill_id, skill_level);
	}
	
	return true;
}
struct hQueue *script_hqueue_get(int idx) {
	if( idx < 0 || idx >= script->hqs || script->hq[idx].items == -1 )
		return NULL;
	return &script->hq[idx];
}
/* set .@id,queue(); */
/* creates queue, returns created queue id */
BUILDIN(queue) {
	int idx = script->hqs;
	int i;
	
	for(i = 0; i < script->hqs; i++) {
		if( script->hq[i].items == -1 ) {
			break;
		}
	}
	
	if( i == script->hqs ) {
		RECREATE(script->hq, struct hQueue, ++script->hqs);
		script->hq[ idx ].item = NULL;
	} else
		idx = i;
	
	script->hq[ idx ].id = idx;
	script->hq[ idx ].items = 0;
	script->hq[ idx ].onDeath[0] = '\0';
	script->hq[ idx ].onLogOut[0] = '\0';
	script->hq[ idx ].onMapChange[0] = '\0';

	script_pushint(st,idx);
	return true;
}
/* set .@length,queuesize(.@queue_id); */
/* returns queue length */
BUILDIN(queuesize) {
	int idx = script_getnum(st, 2);
	
	if( idx < 0 || idx >= script->hqs || script->hq[idx].items == -1 ) {
		ShowWarning("buildin_queuesize: unknown queue id %d\n",idx);
		script_pushint(st, 0);
	} else
		script_pushint(st, script->hq[ idx ].items );
		
	return true;
}
bool script_hqueue_add(int idx, int var) {
	if( idx < 0 || idx >= script->hqs || script->hq[idx].items == -1 ) {
		ShowWarning("script_hqueue_add: unknown queue id %d\n",idx);
		return true;
	} else {
		struct map_session_data *sd;
		int i;
		
		for(i = 0; i < script->hq[idx].items; i++) {
			if( script->hq[idx].item[i] == var ) {
				return true;
			}
		}
		
		if( i == script->hq[idx].items ) {
			
			for(i = 0; i < script->hq[idx].items; i++) {
				if( script->hq[idx].item[i] == 0 ) {
					break;
				}
			}
			
			if( i == script->hq[idx].items )
				RECREATE(script->hq[idx].item, int, ++script->hq[idx].items);
			
			script->hq[idx].item[i] = var;
			
			if( var >= START_ACCOUNT_NUM && (sd = map_id2sd(var)) ) {
				for(i = 0; i < sd->queues_count; i++) {
					if( sd->queues[i] == -1 ) {
						break;
					}
				}
				
				if( i == sd->queues_count )
					RECREATE(sd->queues, int, ++sd->queues_count);
				
				sd->queues[i] = idx;
			}
			
		}
	}
	return false;
}
/* queueadd(.@queue_id,.@var_id); */
/* adds a new entry to the queue, returns 1 if already in queue, 0 otherwise */
BUILDIN(queueadd) {
	int idx = script_getnum(st, 2);
	int var = script_getnum(st, 3);
	
	script_pushint(st,script->queue_add(idx,var)?1:0);
	
	return true;
}
bool script_hqueue_remove(int idx, int var) {
	if( idx < 0 || idx >= script->hqs || script->hq[idx].items == -1 ) {
		ShowWarning("script_hqueue_remove: unknown queue id %d (used with var %d)\n",idx,var);
		return true;
	} else {
		int i;
		
		for(i = 0; i < script->hq[idx].items; i++) {
			if( script->hq[idx].item[i] == var ) {
				return true;
			}
		}
		
		if( i != script->hq[idx].items ) {
			struct map_session_data *sd;
			script->hq[idx].item[i] = 0;
			
			if( var >= START_ACCOUNT_NUM && (sd = map_id2sd(var)) ) {
				for(i = 0; i < sd->queues_count; i++) {
					if( sd->queues[i] == var ) {
						break;
					}
				}
				
				if( i != sd->queues_count )
					sd->queues[i] = -1;
			}
			
		}
	}
	return false;
}
/* queueremove(.@queue_id,.@var_id); */
/* removes a entry from the queue, returns 1 if not in queue, 0 otherwise */
BUILDIN(queueremove) {
	int idx = script_getnum(st, 2);
	int var = script_getnum(st, 3);

	script_pushint(st, script->queue_remove(idx,var)?1:0);
	
	return true;
}

/* queueopt(.@queue_id,optionType,<optional val>); */
/* modifies the queue's options, when val is not provided the option is removed */
/* when OnMapChange event is triggered, it sets a temp char var @QMapChangeTo$ with the destination map name */
/* returns 1 when fails, 0 on success */
BUILDIN(queueopt) {
	int idx = script_getnum(st, 2);
	int var = script_getnum(st, 3);
	
	if( idx < 0 || idx >= script->hqs || script->hq[idx].items == -1 ) {
		ShowWarning("buildin_queueopt: unknown queue id %d\n",idx);
		script_pushint(st, 1);
	} else if( var <= HQO_NONE || var >= HQO_MAX ) {
		ShowWarning("buildin_queueopt: unknown optionType %d\n",var);
		script_pushint(st, 1);
	} else {
		switch( (enum hQueueOpt)var ) {
			case HQO_OnDeath:
				if( script_hasdata(st, 4) )
					safestrncpy(script->hq[idx].onDeath, script_getstr(st, 4), EVENT_NAME_LENGTH);
				else
					script->hq[idx].onDeath[0] = '\0';
				break;
			case HQO_onLogOut:
				if( script_hasdata(st, 4) )
					safestrncpy(script->hq[idx].onLogOut, script_getstr(st, 4), EVENT_NAME_LENGTH);
				else
					script->hq[idx].onLogOut[0] = '\0';
				break;
			case HQO_OnMapChange:
				if( script_hasdata(st, 4) )
					safestrncpy(script->hq[idx].onMapChange, script_getstr(st, 4), EVENT_NAME_LENGTH);
				else
					script->hq[idx].onMapChange[0] = '\0';
				break;
			default:
				ShowWarning("buildin_queueopt: unsupported optionType %d\n",var);
				script_pushint(st, 1);
				break;
		}
	}
	
	return true;
}
bool script_hqueue_del(int idx) {
	if( idx < 0 || idx >= script->hqs || script->hq[idx].items == -1 ) {
		ShowWarning("script_queue_del: unknown queue id %d\n",idx);
		return true;
	} else {
		struct map_session_data *sd;
		int i;
		
		for(i = 0; i < script->hq[idx].items; i++) {
			if( script->hq[idx].item[i] >= START_ACCOUNT_NUM && (sd = map_id2sd(script->hq[idx].item[i])) ) {
				int j;
				for(j = 0; j < sd->queues_count; j++) {
					if( sd->queues[j] == script->hq[idx].item[i] ) {
						break;
					}
				}
				
				if( j != sd->queues_count )
					sd->queues[j] = -1;
			}
		}
		
		script->hq[idx].items = -1;
	}
	return false;
}
/* queuedel(.@queue_id); */
/* deletes queue of id .@queue_id, returns 1 if id not found, 0 otherwise */
BUILDIN(queuedel) {
	int idx = script_getnum(st, 2);
	
	script_pushint(st,script->queue_del(idx)?1:0);
	
	return true;
}

/* set .@id, queueiterator(.@queue_id); */
/* creates a new queue iterator, returns its id */
BUILDIN(queueiterator) {
	int qid = script_getnum(st, 2);
	struct hQueue *queue = NULL;
	int idx = script->hqis;
	int i;
	
	if( qid < 0 || qid >= script->hqs || script->hq[idx].items == -1 || !(queue = script->queue(qid)) ) {
		ShowWarning("queueiterator: invalid queue id %d\n",qid);
		return true;
	}
	
	for(i = 0; i < script->hqis; i++) {
		if( script->hqi[i].items == -1 ) {
			break;
		}
	}
	
	if( i == script->hqis )
		RECREATE(script->hqi, struct hQueueIterator, ++script->hqis);
	else
		idx = i;
	
	RECREATE(script->hqi[ idx ].item, int, queue->items);
	
	memcpy(&script->hqi[idx].item, &queue->item, sizeof(int)*queue->items);
	
	script->hqi[ idx ].items = queue->items;
	script->hqi[ idx ].pos = 0;
	
	script_pushint(st,idx);
	return true;
}
/* Queue Iterator Get Next */
/* returns next/first member in the iterator, 0 if none */
BUILDIN(qiget) {
	int idx = script_getnum(st, 2);
	
	if( idx < 0 || idx >= script->hqis ) {
		ShowWarning("buildin_qiget: unknown queue iterator id %d\n",idx);
		script_pushint(st, 0);
	} else if ( script->hqi[idx].pos == script->hqi[idx].items ) {
		script_pushint(st, 0);
	} else {
		struct hQueueIterator *it = &script->hqi[idx];
		script_pushint(st, it->item[it->pos++]);
	}

	return true;
}
/* Queue Iterator Check */
/* returns 1:0 if there is a next member in the iterator */
BUILDIN(qicheck) {
	int idx = script_getnum(st, 2);
	
	if( idx < 0 || idx >= script->hqis ) {
		ShowWarning("buildin_qicheck: unknown queue iterator id %d\n",idx);
		script_pushint(st, 0);
	} else if ( script->hqi[idx].pos == script->hqi[idx].items ) {
		script_pushint(st, 0);
	} else {
		script_pushint(st, 1);
	}
	
	return true;
}
/* Queue Iterator Check */
BUILDIN(qiclear) {
	int idx = script_getnum(st, 2);
	
	if( idx < 0 || idx >= script->hqis ) {
		ShowWarning("buildin_qiclear: unknown queue iterator id %d\n",idx);
		script_pushint(st, 1);
	} else {
		script->hqi[idx].items = -1;
		script_pushint(st, 0);
	}
	
	return true;
}

// declarations that were supposed to be exported from npc_chat.c
#ifdef PCRE_SUPPORT
	BUILDIN(defpattern);
	BUILDIN(activatepset);
	BUILDIN(deactivatepset);
	BUILDIN(deletepset);
#endif

bool script_hp_add(char *name, char *args, bool (*func)(struct script_state *st)) {
	int n = add_str(name), i = 0;
	
	if( str_data[n].type == C_FUNC ) {
		str_data[n].func = func;
		i = str_data[n].val;
		if( args ) {
			int slen = strlen(args);
			if( script->buildin[i] ) {
				aFree(script->buildin[i]);
			}
			CREATE(script->buildin[i], char, slen + 1);
			safestrncpy(script->buildin[i], args, slen + 1);
		} else {
			if( script->buildin[i] )
				aFree(script->buildin[i]);
			script->buildin[i] = NULL;
		}

	} else {
		i = script->buildin_count;
		str_data[n].type = C_FUNC;
		str_data[n].val = i;
		str_data[n].func = func;
		
		RECREATE(script->buildin, char *, ++script->buildin_count);
				
		/* we only store the arguments, its the only thing used out of this */
		if( args != NULL ) {
			int slen = strlen(args);
			CREATE(script->buildin[i], char, slen + 1);
			safestrncpy(script->buildin[i], args, slen + 1);
		} else
			script->buildin[i] = NULL;
	}
	
	return true;
}

void script_parse_builtin(void) {
	struct script_function BUILDIN[] = {
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
		BUILDIN_DEF(warp,"sii"),
		BUILDIN_DEF(areawarp,"siiiisii??"),
		BUILDIN_DEF(warpchar,"siii"), // [LuzZza]
		BUILDIN_DEF(warpparty,"siii?"), // [Fredzilla] [Paradox924X]
		BUILDIN_DEF(warpguild,"siii"), // [Fredzilla]
		BUILDIN_DEF(setlook,"ii"),
		BUILDIN_DEF(changelook,"ii"), // Simulates but don't Store it
		BUILDIN_DEF(set,"rv"),
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
		BUILDIN_DEF(strcharinfo,"i"),
		BUILDIN_DEF(strnpcinfo,"i"),
		BUILDIN_DEF(getequipid,"i"),
		BUILDIN_DEF(getequipname,"i"),
		BUILDIN_DEF(getbrokenid,"i"), // [Valaris]
		BUILDIN_DEF(repair,"i"), // [Valaris]
		BUILDIN_DEF(repairall,""),
		BUILDIN_DEF(getequipisequiped,"i"),
		BUILDIN_DEF(getequipisenableref,"i"),
		BUILDIN_DEF(getequipisidentify,"i"),
		BUILDIN_DEF(getequiprefinerycnt,"i"),
		BUILDIN_DEF(getequipweaponlv,"i"),
		BUILDIN_DEF(getequippercentrefinery,"i"),
		BUILDIN_DEF(successrefitem,"i"),
		BUILDIN_DEF(failedrefitem,"i"),
		BUILDIN_DEF(downrefitem,"i"),
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
		BUILDIN_DEF(getgroupid,""),
		BUILDIN_DEF(end,""),
		BUILDIN_DEF(checkoption,"i"),
		BUILDIN_DEF(setoption,"i?"),
		BUILDIN_DEF(setcart,"?"),
		BUILDIN_DEF(checkcart,""),
		BUILDIN_DEF(setfalcon,"?"),
		BUILDIN_DEF(checkfalcon,""),
		BUILDIN_DEF(setriding,"?"),
		BUILDIN_DEF(checkriding,""),
		BUILDIN_DEF(checkwug,""),
		BUILDIN_DEF(checkmadogear,""),
		BUILDIN_DEF(setmadogear,"?"),
		BUILDIN_DEF2(savepoint,"save","sii"),
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
		BUILDIN_DEF(cmdothernpc,"ss"),
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
		BUILDIN_DEF(getareausers,"siiii"),
		BUILDIN_DEF(getareadropitem,"siiiiv"),
		BUILDIN_DEF(enablenpc,"s"),
		BUILDIN_DEF(disablenpc,"s"),
		BUILDIN_DEF(hideoffnpc,"s"),
		BUILDIN_DEF(hideonnpc,"s"),
		BUILDIN_DEF(sc_start,"iii?"),
		BUILDIN_DEF(sc_start2,"iiii?"),
		BUILDIN_DEF(sc_start4,"iiiiii?"),
		BUILDIN_DEF(sc_end,"i?"),
		BUILDIN_DEF(getstatus, "i?"),
		BUILDIN_DEF(getscrate,"ii?"),
		BUILDIN_DEF(debugmes,"s"),
		BUILDIN_DEF2(catchpet,"pet","i"),
		BUILDIN_DEF2(birthpet,"bpet",""),
		BUILDIN_DEF(resetlvl,"i"),
		BUILDIN_DEF(resetstatus,""),
		BUILDIN_DEF(resetskill,""),
		BUILDIN_DEF(skillpointcount,""),
		BUILDIN_DEF(changebase,"i?"),
		BUILDIN_DEF(changesex,""),
		BUILDIN_DEF(waitingroom,"si?????"),
		BUILDIN_DEF(delwaitingroom,"?"),
		BUILDIN_DEF2(waitingroomkickall,"kickwaitingroomall","?"),
		BUILDIN_DEF(enablewaitingroomevent,"?"),
		BUILDIN_DEF(disablewaitingroomevent,"?"),
		BUILDIN_DEF2(enablewaitingroomevent,"enablearena",""),		// Added by RoVeRT
		BUILDIN_DEF2(disablewaitingroomevent,"disablearena",""),	// Added by RoVeRT
		BUILDIN_DEF(getwaitingroomstate,"i?"),
		BUILDIN_DEF(warpwaitingpc,"sii?"),
		BUILDIN_DEF(attachrid,"i"),
		BUILDIN_DEF(detachrid,""),
		BUILDIN_DEF(isloggedin,"i?"),
		BUILDIN_DEF(setmapflagnosave,"ssii"),
		BUILDIN_DEF(getmapflag,"si"),
		BUILDIN_DEF(setmapflag,"si?"),
		BUILDIN_DEF(removemapflag,"si?"),
		BUILDIN_DEF(pvpon,"s"),
		BUILDIN_DEF(pvpoff,"s"),
		BUILDIN_DEF(gvgon,"s"),
		BUILDIN_DEF(gvgoff,"s"),
		BUILDIN_DEF(emotion,"i??"),
		BUILDIN_DEF(maprespawnguildid,"sii"),
		BUILDIN_DEF(agitstart,""),	// <Agit>
		BUILDIN_DEF(agitend,""),
		BUILDIN_DEF(agitcheck,""),   // <Agitcheck>
		BUILDIN_DEF(flagemblem,"i"),	// Flag Emblem
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
		BUILDIN_DEF(getskilllist,""),
		BUILDIN_DEF(clearitem,""),
		BUILDIN_DEF(classchange,"ii"),
		BUILDIN_DEF(misceffect,"i"),
		BUILDIN_DEF(playBGM,"s"),
		BUILDIN_DEF(playBGMall,"s?????"),
		BUILDIN_DEF(soundeffect,"si"),
		BUILDIN_DEF(soundeffectall,"si?????"),	// SoundEffectAll [Codemaster]
		BUILDIN_DEF(strmobinfo,"ii"),	// display mob data [Valaris]
		BUILDIN_DEF(guardian,"siisi??"),	// summon guardians
		BUILDIN_DEF(guardianinfo,"sii"),	// display guardian data [Valaris]
		BUILDIN_DEF(petskillbonus,"iiii"), // [Valaris]
		BUILDIN_DEF(petrecovery,"ii"), // [Valaris]
		BUILDIN_DEF(petloot,"i"), // [Valaris]
		BUILDIN_DEF(petheal,"iiii"), // [Valaris]
		BUILDIN_DEF(petskillattack,"viii"), // [Skotlex]
		BUILDIN_DEF(petskillattack2,"viiii"), // [Valaris]
		BUILDIN_DEF(petskillsupport,"viiii"), // [Skotlex]
		BUILDIN_DEF(skilleffect,"vi"), // skill effect [Celest]
		BUILDIN_DEF(npcskilleffect,"viii"), // npc skill effect [Valaris]
		BUILDIN_DEF(specialeffect,"i??"), // npc skill effect [Valaris]
		BUILDIN_DEF(specialeffect2,"i??"), // skill effect on players[Valaris]
		BUILDIN_DEF(nude,""), // nude command [Valaris]
		BUILDIN_DEF(mapwarp,"ssii??"),		// Added by RoVeRT
		BUILDIN_DEF(atcommand,"s"), // [MouseJstr]
		BUILDIN_DEF2(atcommand,"charcommand","s"), // [MouseJstr]
		BUILDIN_DEF(movenpc,"sii?"), // [MouseJstr]
		BUILDIN_DEF(message,"ss"), // [MouseJstr]
		BUILDIN_DEF(npctalk,"s"), // [Valaris]
		BUILDIN_DEF(mobcount,"ss"),
		BUILDIN_DEF(getlook,"i"),
		BUILDIN_DEF(getsavepoint,"i"),
		BUILDIN_DEF(npcspeed,"i"), // [Valaris]
		BUILDIN_DEF(npcwalkto,"ii"), // [Valaris]
		BUILDIN_DEF(npcstop,""), // [Valaris]
		BUILDIN_DEF(getmapxy,"rrri?"),	//by Lorky [Lupus]
		BUILDIN_DEF(checkoption1,"i"),
		BUILDIN_DEF(checkoption2,"i"),
		BUILDIN_DEF(guildgetexp,"i"),
		BUILDIN_DEF(guildchangegm,"is"),
		BUILDIN_DEF(logmes,"s"), //this command actls as MES but rints info into LOG file either SQL/TXT [Lupus]
		BUILDIN_DEF(summon,"si??"), // summons a slave monster [Celest]
		BUILDIN_DEF(isnight,""), // check whether it is night time [Celest]
		BUILDIN_DEF(isday,""), // check whether it is day time [Celest]
		BUILDIN_DEF(isequipped,"i*"), // check whether another item/card has been equipped [Celest]
		BUILDIN_DEF(isequippedcnt,"i*"), // check how many items/cards are being equipped [Celest]
		BUILDIN_DEF(cardscnt,"i*"), // check how many items/cards are being equipped in the same arm [Lupus]
		BUILDIN_DEF(getrefine,""), // returns the refined number of the current item, or an item with index specified [celest]
		BUILDIN_DEF(night,""), // sets the server to night time
		BUILDIN_DEF(day,""), // sets the server to day time
#ifdef PCRE_SUPPORT
		BUILDIN_DEF(defpattern,"iss"), // Define pattern to listen for [MouseJstr]
		BUILDIN_DEF(activatepset,"i"), // Activate a pattern set [MouseJstr]
		BUILDIN_DEF(deactivatepset,"i"), // Deactive a pattern set [MouseJstr]
		BUILDIN_DEF(deletepset,"i"), // Delete a pattern set [MouseJstr]
#endif
		BUILDIN_DEF(dispbottom,"s"), //added from jA [Lupus]
		BUILDIN_DEF(getusersname,""),
		BUILDIN_DEF(recovery,""),
		BUILDIN_DEF(getpetinfo,"i"),
		BUILDIN_DEF(gethominfo,"i"),
		BUILDIN_DEF(getmercinfo,"i?"),
		BUILDIN_DEF(checkequipedcard,"i"),
		BUILDIN_DEF(jump_zero,"il"), //for future jA script compatibility
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
		BUILDIN_DEF(getiteminfo,"ii"), //[Lupus] returns Items Buy / sell Price, etc info
		BUILDIN_DEF(setiteminfo,"iii"), //[Lupus] set Items Buy / sell Price, etc info
		BUILDIN_DEF(getequipcardid,"ii"), //[Lupus] returns CARD ID or other info from CARD slot N of equipped item
		// [zBuffer] List of mathematics commands --->
		BUILDIN_DEF(sqrt,"i"),
		BUILDIN_DEF(pow,"ii"),
		BUILDIN_DEF(distance,"iiii"),
		// <--- [zBuffer] List of mathematics commands
		BUILDIN_DEF(md5,"s"),
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
		BUILDIN_DEF(setbattleflag,"si"),
		BUILDIN_DEF(getbattleflag,"s"),
		BUILDIN_DEF(setitemscript,"is?"), //Set NEW item bonus script. Lupus
		BUILDIN_DEF(disguise,"i"), //disguise player. Lupus
		BUILDIN_DEF(undisguise,""), //undisguise player. Lupus
		BUILDIN_DEF(getmonsterinfo,"ii"), //Lupus
		BUILDIN_DEF(axtoi,"s"),
		BUILDIN_DEF(query_sql,"s*"),
		BUILDIN_DEF(query_logsql,"s*"),
		BUILDIN_DEF(escape_sql,"v"),
		BUILDIN_DEF(atoi,"s"),
		// [zBuffer] List of player cont commands --->
		BUILDIN_DEF(rid2name,"i"),
		BUILDIN_DEF(pcfollow,"ii"),
		BUILDIN_DEF(pcstopfollow,"i"),
		BUILDIN_DEF(pcblockmove,"ii"),
		// <--- [zBuffer] List of player cont commands
		// [zBuffer] List of mob control commands --->
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
		BUILDIN_DEF2(homunculus_evolution,"homevolution",""),	//[orn]
		BUILDIN_DEF2(homunculus_mutate,"hommutate","?"),
		BUILDIN_DEF2(homunculus_shuffle,"homshuffle",""),	//[Zephyrus]
		BUILDIN_DEF(eaclass,"?"),	//[Skotlex]
		BUILDIN_DEF(roclass,"i?"),	//[Skotlex]
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
		/**
		 * 3rd-related
		 **/
		BUILDIN_DEF(makerune,"i"),
		BUILDIN_DEF(checkdragon,""),//[Ind]
		BUILDIN_DEF(setdragon,"?"),//[Ind]
		BUILDIN_DEF(ismounting,""),//[Ind]
		BUILDIN_DEF(setmounting,""),//[Ind]
		BUILDIN_DEF(checkre,"i"),
		/**
		 * rAthena and beyond!
		 **/
		BUILDIN_DEF(getargcount,""),
		BUILDIN_DEF(getcharip,"?"),
		BUILDIN_DEF(is_function,"s"),
		BUILDIN_DEF(get_revision,""),
		BUILDIN_DEF(freeloop,"i"),
		BUILDIN_DEF(getrandgroupitem,"ii"),
		BUILDIN_DEF(cleanmap,"s"),
		BUILDIN_DEF2(cleanmap,"cleanarea","siiii"),
		BUILDIN_DEF(npcskill,"viii"),
		BUILDIN_DEF(itemeffect,"v"),
		BUILDIN_DEF(delequip,"i"),
		/**
		 * @commands (script based)
		 **/
		BUILDIN_DEF(bindatcmd, "ss???"),
		BUILDIN_DEF(unbindatcmd, "s"),
		BUILDIN_DEF(useatcmd, "s"),
		
		//Quest Log System [Inkfish]
		BUILDIN_DEF(setquest, "i"),
		BUILDIN_DEF(erasequest, "i"),
		BUILDIN_DEF(completequest, "i"),
		BUILDIN_DEF(checkquest, "i?"),
		BUILDIN_DEF(changequest, "ii"),
		BUILDIN_DEF(showevent, "ii"),
		
		/**
		 * hQueue [Ind/Hercules]
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
	};
	int i,n, len = ARRAYLENGTH(BUILDIN), start = script->buildin_count;
	char* p;
	RECREATE(script->buildin, char *, start + len);
	for( i = 0; i < len; i++ ) {
		// arg must follow the pattern: (v|s|i|r|l)*\?*\*?
		// 'v' - value (either string or int or reference)
		// 's' - string
		// 'i' - int
		// 'r' - reference (of a variable)
		// 'l' - label
		// '?' - one optional parameter
		// '*' - unknown number of optional parameters
		p = BUILDIN[i].arg;
		while( *p == 'v' || *p == 's' || *p == 'i' || *p == 'r' || *p == 'l' ) ++p;
		while( *p == '?' ) ++p;
		if( *p == '*' ) ++p;
		if( *p != 0 ){
			ShowWarning("script_parse_builtin: ignoring function \"%s\" with invalid arg \"%s\".\n", BUILDIN[i].name, BUILDIN[i].arg);
		} else if( *skip_word(BUILDIN[i].name) != 0 ){
			ShowWarning("script_parse_builtin: ignoring function with invalid name \"%s\" (must be a word).\n", BUILDIN[i].name);
		} else {
			int slen = strlen(BUILDIN[i].arg), offset = start + i;
			n = add_str(BUILDIN[i].name);
			
			if (!strcmp(BUILDIN[i].name, "set")) buildin_set_ref = n;
            else if (!strcmp(BUILDIN[i].name, "callsub")) buildin_callsub_ref = n;
            else if (!strcmp(BUILDIN[i].name, "callfunc")) buildin_callfunc_ref = n;
            else if (!strcmp(BUILDIN[i].name, "getelementofarray") ) buildin_getelementofarray_ref = n;
			
			if( str_data[n].func && str_data[n].func != BUILDIN[i].func )
				continue;/* something replaced it, skip. */
			
			str_data[n].type = C_FUNC;
			str_data[n].val = offset;
			str_data[n].func = BUILDIN[i].func;
			
			/* we only store the arguments, its the only thing used out of this */
			if( slen ) {
				CREATE(script->buildin[offset], char, slen + 1);
				safestrncpy(script->buildin[offset], BUILDIN[i].arg, slen + 1);
			} else
				script->buildin[offset] = NULL;
			
			script->buildin_count++;

		}
	}
}

void script_defaults(void) {
	script = &script_s;
	
	script->hq = NULL;
	script->hqi = NULL;
	script->hqs = script->hqis = 0;
	memset(&script->hqe, 0, sizeof(script->hqe));
	
	script->buildin_count = 0;
	script->buildin = NULL;
	
	script->init = do_init_script;
	script->final = do_final_script;
	
	script->parse_builtin = script_parse_builtin;
	script->addScript = script_hp_add;
	script->conv_num = conv_num;
	script->conv_str = conv_str;
	
	script->queue = script_hqueue_get;
	script->queue_add = script_hqueue_add;
	script->queue_del = script_hqueue_del;
	script->queue_remove = script_hqueue_remove;
}
