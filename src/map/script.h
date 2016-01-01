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
#ifndef MAP_SCRIPT_H
#define MAP_SCRIPT_H

#include "map/map.h" //EVENT_NAME_LENGTH
#include "common/hercules.h"
#include "common/db.h"
#include "common/mmo.h" // struct item
#include "common/strlib.h" //StringBuf

#include <errno.h>
#include <setjmp.h>

/**
 * Declarations
 **/
struct Sql; // common/sql.h
struct eri;
struct item_data;

/**
 * Defines
 **/
// TODO: Remove temporary code
#define ENABLE_CASE_CHECK
#define get_script_source(source) ((source) ? (source) : "Unknown (Possibly source or variables stored in database")
#define DeprecationCaseWarning(func, bad, good, where) ShowError("%s: detected possible use of wrong case in a script. Found '%s', probably meant to be '%s' (in '%s').\n", (func), (bad), (good), get_script_source(where))

#define DeprecationWarning(p) disp_warning_message("This command is deprecated and it will be removed in a future update. Please see the script documentation for an alternative.\n", (p))

#define NUM_WHISPER_VAR 10

/// Maximum amount of elements in script arrays
#define SCRIPT_MAX_ARRAYSIZE (UINT_MAX - 1)

#define SCRIPT_BLOCK_SIZE 512

// Using a prime number for SCRIPT_HASH_SIZE should give better distributions
#define SCRIPT_HASH_SIZE 1021

// Specifies which string hashing method to use
//#define SCRIPT_HASH_DJB2
//#define SCRIPT_HASH_SDBM
#define SCRIPT_HASH_ELF

#define SCRIPT_EQUIP_TABLE_SIZE 20

//#define SCRIPT_DEBUG_DISP
//#define SCRIPT_DEBUG_DISASM
//#define SCRIPT_DEBUG_HASH
//#define SCRIPT_DEBUG_DUMP_STACK

///////////////////////////////////////////////////////////////////////////////
//## TODO possible enhancements: [FlavioJS]
// - 'callfunc' supporting labels in the current npc "::LabelName"
// - 'callfunc' supporting labels in other npcs "NpcName::LabelName"
// - 'function FuncName;' function declarations reverting to global functions
//   if local label isn't found
// - join callfunc and callsub's functionality
// - remove dynamic allocation in add_word()
// - remove GETVALUE / SETVALUE
// - clean up the set_reg / set_val / setd_sub mess
// - detect invalid label references at parse-time

//
// struct script_state* st;
//

/// Returns the script_data at the target index
#define script_getdata(st,i) ( &((st)->stack->stack_data[(st)->start + (i)]) )
/// Returns if the stack contains data at the target index
#define script_hasdata(st,i) ( (st)->end > (st)->start + (i) )
/// Returns the index of the last data in the stack
#define script_lastdata(st) ( (st)->end - (st)->start - 1 )
/// Pushes an int into the stack
#define script_pushint(st,val) (script->push_val((st)->stack, C_INT, (val),NULL))
/// Pushes a string into the stack (script engine frees it automatically)
#define script_pushstr(st,val) (script->push_str((st)->stack, (val)))
/// Pushes a copy of a string into the stack
#define script_pushstrcopy(st,val) (script->push_str((st)->stack, aStrdup(val)))
/// Pushes a constant string into the stack (must never change or be freed)
#define script_pushconststr(st,val) (script->push_conststr((st)->stack, (val)))
/// Pushes a nil into the stack
#define script_pushnil(st) (script->push_val((st)->stack, C_NOP, 0,NULL))
/// Pushes a copy of the data in the target index
#define script_pushcopy(st,i) (script->push_copy((st)->stack, (st)->start + (i)))

#define script_isstring(st,i) data_isstring(script_getdata((st),(i)))
#define script_isint(st,i) data_isint(script_getdata((st),(i)))
#define script_isstringtype(st,i) data_isstring(script->get_val((st), script_getdata((st),(i))))
#define script_isinttype(st,i) data_isint(script->get_val((st), script_getdata((st),(i))))

#define script_getnum(st,val) (script->conv_num((st), script_getdata((st),(val))))
#define script_getstr(st,val) (script->conv_str((st), script_getdata((st),(val))))
#define script_getref(st,val) ( script_getdata((st),(val))->ref )

// Note: "top" functions/defines use indexes relative to the top of the stack
//       -1 is the index of the data at the top

/// Returns the script_data at the target index relative to the top of the stack
#define script_getdatatop(st,i) ( &((st)->stack->stack_data[(st)->stack->sp + (i)]) )
/// Pushes a copy of the data in the target index relative to the top of the stack
#define script_pushcopytop(st,i) script->push_copy((st)->stack, (st)->stack->sp + (i))
/// Removes the range of values [start,end[ relative to the top of the stack
#define script_removetop(st,start,end) ( script->pop_stack((st), ((st)->stack->sp + (start)), (st)->stack->sp + (end)) )

//
// struct script_data* data;
//

/// Returns if the script data is a string
#define data_isstring(data) ( (data)->type == C_STR || (data)->type == C_CONSTSTR )
/// Returns if the script data is an int
#define data_isint(data) ( (data)->type == C_INT )
/// Returns if the script data is a reference
#define data_isreference(data) ( (data)->type == C_NAME )
/// Returns if the script data is a label
#define data_islabel(data) ( (data)->type == C_POS )
/// Returns if the script data is an internal script function label
#define data_isfunclabel(data) ( (data)->type == C_USERFUNC_POS )

/// Returns if this is a reference to a constant
#define reference_toconstant(data) ( script->str_data[reference_getid(data)].type == C_INT )
/// Returns if this a reference to a param
#define reference_toparam(data) ( script->str_data[reference_getid(data)].type == C_PARAM )
/// Returns if this a reference to a variable
//##TODO confirm it's C_NAME [FlavioJS]
#define reference_tovariable(data) ( script->str_data[reference_getid(data)].type == C_NAME )
/// Returns the unique id of the reference (id and index)
#define reference_getuid(data) ( (data)->u.num )
/// Returns the id of the reference
#define reference_getid(data) ( (int32)(int64)(reference_getuid(data) & 0xFFFFFFFF) )
/// Returns the array index of the reference
#define reference_getindex(data) ( (uint32)(int64)((reference_getuid(data) >> 32) & 0xFFFFFFFF) )
/// Returns the name of the reference
#define reference_getname(data) ( script->str_buf + script->str_data[reference_getid(data)].str )
/// Returns the linked list of uid-value pairs of the reference (can be NULL)
#define reference_getref(data) ( (data)->ref )
/// Returns the value of the constant
#define reference_getconstant(data) ( script->str_data[reference_getid(data)].val )
/// Returns the type of param
#define reference_getparamtype(data) ( script->str_data[reference_getid(data)].val )

/// Composes the uid of a reference from the id and the index
#define reference_uid(id,idx) ( (int64) ((uint64)(id) & 0xFFFFFFFF) | ((uint64)(idx) << 32) )

/// Checks whether two references point to the same variable (or array)
#define is_same_reference(data1, data2) \
	(  reference_getid(data1) == reference_getid(data2) \
	&& ( (data1->ref == data2->ref && data1->ref == NULL) \
	  || (data1->ref != NULL && data2->ref != NULL && data1->ref->vars == data2->ref->vars \
	     ) ) )

#define script_getvarid(var)  ( (int32)(int64)(var & 0xFFFFFFFF) )
#define script_getvaridx(var) ( (uint32)(int64)((var >> 32) & 0xFFFFFFFF) )

#define not_server_variable(prefix) ( (prefix) != '$' && (prefix) != '.' && (prefix) != '\'')
#define is_string_variable(name) ( (name)[strlen(name) - 1] == '$' )

#define BUILDIN(x) bool buildin_ ## x (struct script_state* st)

#define script_fetch(st, n, t) do { \
	if( script_hasdata((st),(n)) ) \
		(t)=script_getnum((st),(n)); \
	else \
		(t) = 0; \
} while(0)


/**
 * Enumerations
 **/
typedef enum c_op {
	C_NOP, // end of script/no value (nil)
	C_POS,
	C_INT, // number
	C_PARAM, // parameter variable (see pc_readparam/pc_setparam)
	C_FUNC, // buildin function call
	C_STR, // string (free'd automatically)
	C_CONSTSTR, // string (not free'd)
	C_ARG, // start of argument list
	C_NAME,
	C_EOL, // end of line (extra stack values are cleared)
	C_RETINFO,
	C_USERFUNC, // internal script function
	C_USERFUNC_POS, // internal script function label
	C_REF, // the next call to c_op2 should push back a ref to the left operand
	C_LSTR, //Language Str (struct script_code_str)

	// operators
	C_OP3, // a ? b : c
	C_LOR, // a || b
	C_LAND, // a && b
	C_LE, // a <= b
	C_LT, // a < b
	C_GE, // a >= b
	C_GT, // a > b
	C_EQ, // a == b
	C_NE, // a != b
	C_XOR, // a ^ b
	C_OR, // a | b
	C_AND, // a & b
	C_ADD, // a + b
	C_SUB, // a - b
	C_MUL, // a * b
	C_DIV, // a / b
	C_MOD, // a % b
	C_NEG, // - a
	C_LNOT, // ! a
	C_NOT, // ~ a
	C_R_SHIFT, // a >> b
	C_L_SHIFT, // a << b
	C_ADD_POST, // a++
	C_SUB_POST, // a--
	C_ADD_PRE, // ++a
	C_SUB_PRE, // --a
	C_RE_EQ, // ~=
	C_RE_NE, // ~!
} c_op;

/// Script queue options
enum ScriptQueueOptions {
	SQO_NONE,        ///< No options set
	SQO_ONLOGOUT,    ///< Execute event on logout
	SQO_ONDEATH,     ///< Execute event on death
	SQO_ONMAPCHANGE, ///< Execute event on map change
	SQO_MAX,
};

enum e_script_state { RUN,STOP,END,RERUNLINE,GOTO,RETFUNC,CLOSE };

enum script_parse_options {
	SCRIPT_USE_LABEL_DB = 0x1,// records labels in scriptlabel_db
	SCRIPT_IGNORE_EXTERNAL_BRACKETS = 0x2,// ignores the check for {} brackets around the script
	SCRIPT_RETURN_EMPTY_SCRIPT = 0x4// returns the script object instead of NULL for empty scripts
};

enum { LABEL_NEXTLINE=1,LABEL_START };

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

enum e_arglist {
	ARGLIST_UNDEFINED = 0,
	ARGLIST_NO_PAREN  = 1,
	ARGLIST_PAREN     = 2,
};

/*==========================================
 * (Only those needed) local declaration prototype
 * - those could be used server-wide so that the scans are done once during processing and never again,
 * - doing so would also improve map zone processing and storage [Ind]
 *------------------------------------------*/

enum {
	MF_NOMEMO, //0
	MF_NOTELEPORT,
	MF_NOSAVE,
	MF_NOBRANCH,
	MF_NOPENALTY,
	MF_NOZENYPENALTY,
	MF_PVP,
	MF_PVP_NOPARTY,
	MF_PVP_NOGUILD,
	MF_GVG,
	MF_GVG_NOPARTY, //10
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
	MF_NOJOBEXP, //30
	MF_NOMOBLOOT,
	MF_NOMVPLOOT,
	MF_NORETURN,
	MF_NOWARPTO,
	MF_NIGHTMAREDROP,
	MF_ZONE,
	MF_NOCOMMAND,
	MF_NODROP,
	MF_JEXP,
	MF_BEXP, //40
	MF_NOVENDING,
	MF_LOADEVENT,
	MF_NOCHAT,
	MF_NOEXPPENALTY,
	MF_GUILDLOCK,
	MF_TOWN,
	MF_AUTOTRADE,
	MF_ALLOWKS,
	MF_MONSTER_NOTELEPORT,
	MF_PVP_NOCALCRANK, //50
	MF_BATTLEGROUND,
	MF_RESET,
	MF_NOTOMB,
	MF_NOCASHSHOP,
	MF_NOVIEWID
};

/**
 * Structures
 **/

struct Script_Config {
	unsigned warn_func_mismatch_argtypes : 1;
	unsigned warn_func_mismatch_paramnum : 1;
	int check_cmdcount;
	int check_gotocount;
	int input_min_value;
	int input_max_value;

	const char *die_event_name;
	const char *kill_pc_event_name;
	const char *kill_mob_event_name;
	const char *login_event_name;
	const char *logout_event_name;
	const char *loadmap_event_name;
	const char *baselvup_event_name;
	const char *joblvup_event_name;

	const char* ontouch_name;
	const char* ontouch2_name;
	const char* onuntouch_name;
};

/**
 * Generic reg database abstraction to be used with various types of regs/script variables.
 */
struct reg_db {
	struct DBMap *vars;
	struct DBMap *arrays;
};

struct script_retinfo {
	struct reg_db scope;        ///< scope variables
	struct script_code* script; ///< script code
	int pos;                    ///< script location
	int nargs;                  ///< argument count
	int defsp;                  ///< default stack pointer
};

/**
 * Represents a variable in the script stack.
 */
struct script_data {
	enum c_op type;     ///< Data type
	union script_data_val {
		int64 num;                 ///< Numeric data
		char *mutstr;              ///< Mutable string
		const char *str;           ///< Constant string
		struct script_retinfo *ri; ///< Function return information
	} u;                ///< Data (field depends on `type`)
	struct reg_db *ref; ///< Reference to the scope's variables
};

/**
 * A script string buffer, used to hold strings used by the script engine.
 */
VECTOR_STRUCT_DECL(script_string_buf, char);

/**
 * Script buffer, used to hold parsed script data.
 */
VECTOR_STRUCT_DECL(script_buf, unsigned char);

// Moved defsp from script_state to script_stack since
// it must be saved when script state is RERUNLINE. [Eoe / jA 1094]
struct script_code {
	struct script_buf script_buf;
	struct reg_db local; ///< Local (npc) vars
	unsigned short instances;
};

struct script_stack {
	int sp;                         ///< number of entries in the stack
	int sp_max;                     ///< capacity of the stack
	int defsp;
	struct script_data *stack_data; ///< stack
	struct reg_db scope;            ///< scope variables
};

/**
 * Data structure to represent a script queue.
 * @author Ind/Hercules
 */
struct script_queue {
	int id;                              ///< Queue identifier
	VECTOR_DECL(int) entries;            ///< Items in the queue.
	bool valid;                          ///< Whether the queue is valid.
	/// Events
	char event_logout[EVENT_NAME_LENGTH];    ///< Logout event
	char event_death[EVENT_NAME_LENGTH];     ///< Death event
	char event_mapchange[EVENT_NAME_LENGTH]; ///< Map change event
};

/**
 * Iterator for a struct script_queue.
 */
struct script_queue_iterator {
	VECTOR_DECL(int) entries; ///< Entries in the queue (iterator's cached copy)
	bool valid;               ///< Whether the queue is valid (initialized - not necessarily having entries available)
	int pos;                  ///< Iterator's cursor
};

struct script_state {
	struct script_stack* stack;
	struct reg_db **pending_refs; ///< References to .vars returned by sub-functions, pending deletion.
	int pending_ref_count;        ///< Amount of pending_refs currently stored.
	int start,end;
	int pos;
	enum e_script_state state;
	int rid,oid;
	struct script_code *script;
	struct sleep_data {
		int tick,timer,charid;
	} sleep;
	int instance_id;
	//For backing up purposes
	struct script_state *bk_st;
	unsigned char hIterator;
	int bk_npcid;
	unsigned freeloop : 1;// used by buildin_freeloop
	unsigned op2ref : 1;// used by op_2
	unsigned npc_item_flag : 1;
	unsigned int id;
};

struct script_function {
	bool (*func)(struct script_state *st);
	char *name;
	char *arg;
	bool deprecated;
};

// String buffer structures.
// str_data stores string information
struct str_data_struct {
	enum c_op type;
	int str;
	int backpatch;
	int label;
	bool (*func)(struct script_state *st);
	int val;
	int next;
	uint8 deprecated : 1;
};

struct script_label_entry {
	int key,pos;
};

struct script_syntax_data {
	struct {
		enum curly_type type;
		int index;
		int count;
		int flag;
		struct linkdb_node *case_label;
	} curly[256]; // Information right parenthesis
	int curly_count; // The number of right brackets
	int index; // Number of the syntax used in the script
	int last_func; // buildin index of the last parsed function
	unsigned int nested_call; //Dont really know what to call this
	bool lang_macro_active;
	struct DBMap *strings; // string map parsed (used when exporting strings only)
	struct DBMap *translation_db; //non-null if this npc has any translated strings to be linked
};

struct casecheck_data {
	struct str_data_struct *str_data;
	int str_data_size; // size of the data
	int str_num; // next id to be assigned
	// str_buf holds the strings themselves
	char *str_buf;
	int str_size; // size of the buffer
	int str_pos; // next position to be assigned
	int str_hash[SCRIPT_HASH_SIZE];
	const char *(*add_str) (const char* p);
	void (*clear) (void);
};

struct script_array {
	unsigned int id;/* the first 32b of the 64b uid, aka the id */
	unsigned int size;/* how many members */
	unsigned int *members;/* member list */
};

struct string_translation_entry {
	uint8 lang_id;
	char string[];
};

struct string_translation {
	int string_id;
	uint8 translations;
	int len;
	uint8 *buf; // Array of struct string_translation_entry
};

/**
 * Interface
 **/
struct script_interface {
	/* */
	struct DBMap *st_db;
	unsigned int active_scripts;
	unsigned int next_id;
	struct eri *st_ers;
	struct eri *stack_ers;
	/* */
	VECTOR_DECL(struct script_queue) hq;
	VECTOR_DECL(struct script_queue_iterator) hqi;
	/*  */
	char **buildin;
	unsigned int buildin_count;
	/**
	 * used to generate quick script_array entries
	 **/
	struct eri *array_ers;
	/* */
	struct str_data_struct *str_data;
	int str_data_size; // size of the data
	int str_num; // next id to be assigned
	// str_buf holds the strings themselves
	char *str_buf;
	size_t str_size; // size of the buffer
	int str_pos; // next position to be assigned
	int str_hash[SCRIPT_HASH_SIZE];
	/* */
	char *word_buf;
	size_t word_size;
	/* Script string storage */
	char *string_list;
	int string_list_size;
	int string_list_pos;
	/*  */
	unsigned short current_item_id;
	/* */
	struct script_label_entry *labels;
	int label_count;
	int labels_size;
	/* */
	struct Script_Config config;
	/* */
	/// temporary buffer for passing around compiled bytecode
	/// @see add_scriptb, set_label, parse_script
	struct script_buf buf;
	/* */
	struct script_syntax_data syntax;
	/* */
	int parse_options;
	// important buildin function references for usage in scripts
	int buildin_set_ref;
	int buildin_callsub_ref;
	int buildin_callfunc_ref;
	int buildin_getelementofarray_ref;
	/* */
	jmp_buf     error_jump;
	char*       error_msg;
	const char* error_pos;
	int         error_report; // if the error should produce output
	// Used by disp_warning_message
	const char* parser_current_src;
	const char* parser_current_file;
	int         parser_current_line;
	int parse_syntax_for_flag;
	// aegis->athena slot position conversion table
	unsigned int equip[SCRIPT_EQUIP_TABLE_SIZE];
	/* */
	/* Caches compiled autoscript item code. */
	/* Note: This is not cleared when reloading itemdb. */
	struct DBMap *autobonus_db; // char* script -> char* bytecode
	struct DBMap *userfunc_db; // const char* func_name -> struct script_code*
	/* */
	int potion_flag; //For use on Alchemist improved potions/Potion Pitcher. [Skotlex]
	int potion_hp, potion_per_hp, potion_sp, potion_per_sp;
	int potion_target;
	/* */
	unsigned int *generic_ui_array;
	unsigned int generic_ui_array_size;
	/* Set during startup when attempting to export the lang, unset after server initialization is over */
	FILE *lang_export_fp;
	char *lang_export_file;/* for lang_export_fp */
	/* set and unset on npc_parse_script */
	const char *parser_current_npc_name;
	/* */
	int buildin_mes_offset;
	int buildin_select_offset;
	int buildin_lang_macro_offset;
	/* */
	struct DBMap *translation_db;/* npc_name => DBMap (strings) */
	VECTOR_DECL(uint8 *) translation_buf;
	/* */
	char **languages;
	uint8 max_lang_id;
	/* */
	struct script_string_buf parse_simpleexpr_str;
	struct script_string_buf lang_export_line_buf;
	struct script_string_buf lang_export_escaped_buf;
	/* */
	int parse_cleanup_timer_id;
	/*  */
	void (*init) (bool minimal);
	void (*final) (void);
	int  (*reload) (void);
	/* parse */
	struct script_code* (*parse) (const char* src,const char* file,int line,int options, int *retval);
	bool (*add_builtin) (const struct script_function *buildin, bool override);
	void (*parse_builtin) (void);
	const char* (*parse_subexpr) (const char* p,int limit);
	const char* (*skip_space) (const char* p);
	void (*error) (const char* src, const char* file, int start_line, const char* error_msg, const char* error_pos);
	void (*warning) (const char* src, const char* file, int start_line, const char* error_msg, const char* error_pos);
	/* */
	bool (*addScript) (char *name, char *args, bool (*func)(struct script_state *st), bool isDeprecated);
	int (*conv_num) (struct script_state *st,struct script_data *data);
	const char* (*conv_str) (struct script_state *st,struct script_data *data);
	struct map_session_data *(*rid2sd) (struct script_state *st);
	struct map_session_data *(*id2sd) (struct script_state *st, int account_id);
	struct map_session_data *(*charid2sd) (struct script_state *st, int char_id);
	struct map_session_data *(*nick2sd) (struct script_state *st, const char *name);
	void (*detach_rid) (struct script_state* st);
	struct script_data* (*push_val)(struct script_stack* stack, enum c_op type, int64 val, struct reg_db *ref);
	struct script_data *(*get_val) (struct script_state* st, struct script_data* data);
	char* (*get_val_ref_str) (struct script_state* st, struct reg_db *n, struct script_data* data);
	char* (*get_val_scope_str) (struct script_state* st, struct reg_db *n, struct script_data* data);
	char* (*get_val_npc_str) (struct script_state* st, struct reg_db *n, struct script_data* data);
	char* (*get_val_instance_str) (struct script_state* st, const char* name, struct script_data* data);
	int (*get_val_ref_num) (struct script_state* st, struct reg_db *n, struct script_data* data);
	int (*get_val_scope_num) (struct script_state* st, struct reg_db *n, struct script_data* data);
	int (*get_val_npc_num) (struct script_state* st, struct reg_db *n, struct script_data* data);
	int (*get_val_instance_num) (struct script_state* st, const char* name, struct script_data* data);
	const void *(*get_val2) (struct script_state *st, int64 uid, struct reg_db *ref);
	struct script_data *(*push_str) (struct script_stack *stack, char *str);
	struct script_data *(*push_conststr) (struct script_stack *stack, const char *str);
	struct script_data *(*push_copy) (struct script_stack *stack, int pos);
	void (*pop_stack) (struct script_state* st, int start, int end);
	void (*set_constant) (const char *name, int value, bool is_parameter, bool is_deprecated);
	void (*set_constant2) (const char *name, int value, bool is_parameter, bool is_deprecated);
	bool (*get_constant) (const char* name, int* value);
	void (*label_add)(int key, int pos);
	void (*run) (struct script_code *rootscript, int pos, int rid, int oid);
	void (*run_npc) (struct script_code *rootscript, int pos, int rid, int oid);
	void (*run_pet) (struct script_code *rootscript, int pos, int rid, int oid);
	void (*run_main) (struct script_state *st);
	int (*run_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*set_var) (struct map_session_data *sd, char *name, void *val);
	void (*stop_instances) (struct script_code *code);
	void (*free_code) (struct script_code* code);
	void (*free_vars) (struct DBMap *var_storage);
	struct script_state* (*alloc_state) (struct script_code* rootscript, int pos, int rid, int oid);
	void (*free_state) (struct script_state* st);
	void (*add_pending_ref) (struct script_state *st, struct reg_db *ref);
	void (*run_autobonus) (const char *autobonus,int id, int pos);
	void (*cleararray_pc) (struct map_session_data* sd, const char* varname, void* value);
	void (*setarray_pc) (struct map_session_data* sd, const char* varname, uint32 idx, void* value, int* refcache);
	int (*config_read) (char *cfgName);
	int (*add_str) (const char* p);
	const char* (*get_str) (int id);
	int (*search_str) (const char* p);
	void (*setd_sub) (struct script_state *st, struct map_session_data *sd, const char *varname, int elem, const void *value, struct reg_db *ref);
	void (*attach_state) (struct script_state* st);
	/* */
	struct script_queue *(*queue) (int idx);
	bool (*queue_add) (int idx, int var);
	bool (*queue_del) (int idx);
	bool (*queue_remove) (int idx, int var);
	int (*queue_create) (void);
	bool (*queue_clear) (int idx);
	/* */
	const char * (*parse_curly_close) (const char *p);
	const char * (*parse_syntax_close) (const char *p);
	const char * (*parse_syntax_close_sub) (const char *p, int *flag);
	const char * (*parse_syntax) (const char *p);
	c_op (*get_com) (const struct script_buf *scriptbuf, int *pos);
	int (*get_num) (const struct script_buf *scriptbuf, int *pos);
	const char* (*op2name) (int op);
	void (*reportsrc) (struct script_state *st);
	void (*reportdata) (struct script_data *data);
	void (*reportfunc) (struct script_state *st);
	void (*disp_warning_message) (const char *mes, const char *pos);
	void (*check_event) (struct script_state *st, const char *evt);
	unsigned int (*calc_hash) (const char *p);
	void (*addb) (int a);
	void (*addc) (int a);
	void (*addi) (int a);
	void (*addl) (int l);
	void (*set_label) (int l, int pos, const char *script_pos);
	const char* (*skip_word) (const char *p);
	int (*add_word) (const char *p);
	const char* (*parse_callfunc) (const char *p, int require_paren, int is_custom);
	void (*parse_nextline) (bool first, const char *p);
	const char* (*parse_variable) (const char *p);
	const char* (*parse_simpleexpr) (const char *p);
	const char* (*parse_expr) (const char *p);
	const char* (*parse_line) (const char *p);
	void (*read_constdb) (void);
	void (*constdb_comment) (const char *comment);
	void (*load_parameters) (void);
	const char* (*print_line) (StringBuf *buf, const char *p, const char *mark, int line);
	void (*errorwarning_sub) (StringBuf *buf, const char *src, const char *file, int start_line, const char *error_msg, const char *error_pos);
	int (*set_reg) (struct script_state *st, struct map_session_data *sd, int64 num, const char *name, const void *value, struct reg_db *ref);
	void (*set_reg_ref_str) (struct script_state* st, struct reg_db *n, int64 num, const char* name, const char *str);
	void (*set_reg_scope_str) (struct script_state* st, struct reg_db *n, int64 num, const char* name, const char *str);
	void (*set_reg_npc_str) (struct script_state* st, struct reg_db *n, int64 num, const char* name, const char *str);
	void (*set_reg_instance_str) (struct script_state* st, int64 num, const char* name, const char *str);
	void (*set_reg_ref_num) (struct script_state* st, struct reg_db *n, int64 num, const char* name, int val);
	void (*set_reg_scope_num) (struct script_state* st, struct reg_db *n, int64 num, const char* name, int val);
	void (*set_reg_npc_num) (struct script_state* st, struct reg_db *n, int64 num, const char* name, int val);
	void (*set_reg_instance_num) (struct script_state* st, int64 num, const char* name, int val);
	void (*stack_expand) (struct script_stack *stack);
	struct script_data* (*push_retinfo) (struct script_stack *stack, struct script_retinfo *ri, struct reg_db *ref);
	void (*op_3) (struct script_state *st, int op);
	void (*op_2str) (struct script_state *st, int op, const char *s1, const char *s2);
	void (*op_2num) (struct script_state *st, int op, int i1, int i2);
	void (*op_2) (struct script_state *st, int op);
	void (*op_1) (struct script_state *st, int op);
	void (*check_buildin_argtype) (struct script_state *st, int func);
	void (*detach_state) (struct script_state *st, bool dequeue_event);
	int (*db_free_code_sub) (union DBKey key, struct DBData *data, va_list ap);
	void (*add_autobonus) (const char *autobonus);
	int (*menu_countoptions) (const char *str, int max_count, int *total);
	int (*buildin_areawarp_sub) (struct block_list *bl, va_list ap);
	int (*buildin_areapercentheal_sub) (struct block_list *bl, va_list ap);
	void (*buildin_delitem_delete) (struct map_session_data *sd, int idx, int *amount, bool delete_items);
	bool (*buildin_delitem_search) (struct map_session_data *sd, struct item *it, bool exact_match);
	int (*buildin_killmonster_sub_strip) (struct block_list *bl, va_list ap);
	int (*buildin_killmonster_sub) (struct block_list *bl, va_list ap);
	int (*buildin_killmonsterall_sub_strip) (struct block_list *bl, va_list ap);
	int (*buildin_killmonsterall_sub) (struct block_list *bl, va_list ap);
	int (*buildin_announce_sub) (struct block_list *bl, va_list ap);
	int (*buildin_getareausers_sub) (struct block_list *bl, va_list ap);
	int (*buildin_getareadropitem_sub) (struct block_list *bl, va_list ap);
	int (*mapflag_pvp_sub) (struct block_list *bl, va_list ap);
	int (*buildin_pvpoff_sub) (struct block_list *bl, va_list ap);
	int (*buildin_maprespawnguildid_sub_pc) (struct map_session_data *sd, va_list ap);
	int (*buildin_maprespawnguildid_sub_mob) (struct block_list *bl, va_list ap);
	int (*buildin_mobcount_sub) (struct block_list *bl, va_list ap);
	int (*playbgm_sub) (struct block_list *bl, va_list ap);
	int (*playbgm_foreachpc_sub) (struct map_session_data *sd, va_list args);
	int (*soundeffect_sub) (struct block_list *bl, va_list ap);
	int (*buildin_query_sql_sub) (struct script_state *st, struct Sql *handle);
	int (*buildin_instance_warpall_sub) (struct block_list *bl, va_list ap);
	int (*buildin_mobuseskill_sub) (struct block_list *bl, va_list ap);
	int (*cleanfloor_sub) (struct block_list *bl, va_list ap);
	int (*run_func) (struct script_state *st);
	const char *(*getfuncname) (struct script_state *st);
	// for ENABLE_CASE_CHECK
	unsigned int (*calc_hash_ci) (const char *p);
	struct casecheck_data local_casecheck;
	struct casecheck_data global_casecheck;
	// end ENABLE_CASE_CHECK
	/**
	 * Array Handling
	 **/
	struct reg_db *(*array_src) (struct script_state *st, struct map_session_data *sd, const char *name, struct reg_db *ref);
	void (*array_update) (struct reg_db *src, int64 num, bool empty);
	void (*array_delete) (struct reg_db *src, struct script_array *sa);
	void (*array_remove_member) (struct reg_db *src, struct script_array *sa, unsigned int idx);
	void (*array_add_member) (struct script_array *sa, unsigned int idx);
	unsigned int (*array_size) (struct script_state *st, struct map_session_data *sd, const char *name, struct reg_db *ref);
	unsigned int (*array_highest_key) (struct script_state *st, struct map_session_data *sd, const char *name, struct reg_db *ref);
	int (*array_free_db) (union DBKey key, struct DBData *data, va_list ap);
	void (*array_ensure_zero) (struct script_state *st, struct map_session_data *sd, int64 uid, struct reg_db *ref);
	/* */
	void (*reg_destroy_single) (struct map_session_data *sd, int64 reg, struct script_reg_state *data);
	int (*reg_destroy) (union DBKey key, struct DBData *data, va_list ap);
	/* */
	void (*generic_ui_array_expand) (unsigned int plus);
	unsigned int *(*array_cpy_list) (struct script_array *sa);
	/* */
	void (*hardcoded_constants) (void);
	unsigned short (*mapindexname2id) (struct script_state *st, const char* name);
	int (*string_dup) (char *str);
	void (*load_translations) (void);
	int (*load_translation) (const char *file, uint8 lang_id);
	int (*translation_db_destroyer) (union DBKey key, struct DBData *data, va_list ap);
	void (*clear_translations) (bool reload);
	int (*parse_cleanup_timer) (int tid, int64 tick, int id, intptr_t data);
	uint8 (*add_language) (const char *name);
	const char *(*get_translation_file_name) (const char *file);
	void (*parser_clean_leftovers) (void);
	void (*run_use_script) (struct map_session_data *sd, struct item_data *data, int oid);
	void (*run_item_equip_script) (struct map_session_data *sd, struct item_data *data, int oid);
	void (*run_item_unequip_script) (struct map_session_data *sd, struct item_data *data, int oid);
};

#ifdef HERCULES_CORE
void script_defaults(void);
#endif // HERCULES_CORE

HPShared struct script_interface *script;

#endif /* MAP_SCRIPT_H */
