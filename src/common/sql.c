/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2016  Hercules Dev Team
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

#include "sql.h"

#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/strlib.h"
#include "common/timer.h"

#ifdef WIN32
#	include "common/winapi.h" // Needed before mysql.h
#endif
#include <mysql.h>
#include <stdio.h>
#include <stdlib.h> // strtoul

void hercules_mysql_error_handler(unsigned int ecode);

int mysql_reconnect_type = 2;
int mysql_reconnect_count = 1;

struct sql_interface sql_s;
struct sql_interface *SQL;

/// Sql handle
struct Sql {
	StringBuf buf;
	MYSQL handle;
	MYSQL_RES* result;
	MYSQL_ROW row;
	unsigned long* lengths;
	int keepalive;
};

// Column length receiver.
// Takes care of the possible size mismatch between uint32 and unsigned long.
struct s_column_length {
	uint32* out_length;
	unsigned long length;
};
typedef struct s_column_length s_column_length;

/// Sql statement
struct SqlStmt {
	StringBuf buf;
	MYSQL_STMT* stmt;
	MYSQL_BIND* params;
	MYSQL_BIND* columns;
	s_column_length* column_lengths;
	size_t max_params;
	size_t max_columns;
	bool bind_params;
	bool bind_columns;
};

///////////////////////////////////////////////////////////////////////////////
// Sql Handle
///////////////////////////////////////////////////////////////////////////////

/// Allocates and initializes a new Sql handle.
struct Sql *Sql_Malloc(void)
{
	struct Sql *self;

	CREATE(self, struct Sql, 1);
	mysql_init(&self->handle);
	StrBuf->Init(&self->buf);
	self->lengths = NULL;
	self->result = NULL;
	self->keepalive = INVALID_TIMER;
	{
		my_bool reconnect = 1;
		mysql_options(&self->handle, MYSQL_OPT_RECONNECT, &reconnect);
	}
	return self;
}

static int Sql_P_Keepalive(struct Sql *self);

/// Establishes a connection.
int Sql_Connect(struct Sql *self, const char *user, const char *passwd, const char *host, uint16 port, const char *db)
{
	if( self == NULL )
		return SQL_ERROR;

	StrBuf->Clear(&self->buf);
	if( !mysql_real_connect(&self->handle, host, user, passwd, db, (unsigned int)port, NULL/*unix_socket*/, 0/*clientflag*/) )
	{
		ShowSQL("%s\n", mysql_error(&self->handle));
		return SQL_ERROR;
	}

	self->keepalive = Sql_P_Keepalive(self);
	if( self->keepalive == INVALID_TIMER )
	{
		ShowSQL("Failed to establish keepalive for DB connection!\n");
		return SQL_ERROR;
	}

	return SQL_SUCCESS;
}

/// Retrieves the timeout of the connection.
int Sql_GetTimeout(struct Sql *self, uint32 *out_timeout)
{
	if( self && out_timeout && SQL_SUCCESS == SQL->Query(self, "SHOW VARIABLES LIKE 'wait_timeout'") ) {
		char* data;
		size_t len;
		if( SQL_SUCCESS == SQL->NextRow(self) &&
			SQL_SUCCESS == SQL->GetData(self, 1, &data, &len) ) {
				*out_timeout = (uint32)strtoul(data, NULL, 10);
				SQL->FreeResult(self);
				return SQL_SUCCESS;
		}
		SQL->FreeResult(self);
	}
	return SQL_ERROR;
}

/// Retrieves the name of the columns of a table into out_buf, with the separator after each name.
int Sql_GetColumnNames(struct Sql *self, const char *table, char *out_buf, size_t buf_len, char sep)
{
	char* data;
	size_t len;
	size_t off = 0;

	nullpo_retr(SQL_ERROR, out_buf);
	if( self == NULL || SQL_ERROR == SQL->Query(self, "EXPLAIN `%s`", table) )
		return SQL_ERROR;

	out_buf[off] = '\0';
	while( SQL_SUCCESS == SQL->NextRow(self) && SQL_SUCCESS == SQL->GetData(self, 0, &data, &len) ) {
		len = strnlen(data, len);
		if( off + len + 2 > buf_len )
		{
			ShowDebug("Sql_GetColumns: output buffer is too small\n");
			*out_buf = '\0';
			return SQL_ERROR;
		}
		memcpy(out_buf+off, data, len);
		off += len;
		out_buf[off++] = sep;
	}
	out_buf[off] = '\0';
	SQL->FreeResult(self);
	return SQL_SUCCESS;
}

/// Changes the encoding of the connection.
int Sql_SetEncoding(struct Sql *self, const char *encoding)
{
	if( self && mysql_set_character_set(&self->handle, encoding) == 0 )
		return SQL_SUCCESS;
	return SQL_ERROR;
}

/// Pings the connection.
int Sql_Ping(struct Sql *self)
{
	if( self && mysql_ping(&self->handle) == 0 )
		return SQL_SUCCESS;
	return SQL_ERROR;
}

/// Wrapper function for Sql_Ping.
///
/// @private
static int Sql_P_KeepaliveTimer(int tid, int64 tick, int id, intptr_t data)
{
	struct Sql *self = (struct Sql *)data;
	ShowInfo("Pinging SQL server to keep connection alive...\n");
	Sql_Ping(self);
	return 0;
}

/// Establishes keepalive (periodic ping) on the connection.
///
/// @return the keepalive timer id, or INVALID_TIMER
/// @private
static int Sql_P_Keepalive(struct Sql *self)
{
	uint32 timeout, ping_interval;

	// set a default value first
	timeout = 28800; // 8 hours

	// request the timeout value from the mysql server
	Sql_GetTimeout(self, &timeout);

	if( timeout < 60 )
		timeout = 60;

	// establish keepalive
	ping_interval = timeout - 30; // 30-second reserve
	//add_timer_func_list(Sql_P_KeepaliveTimer, "Sql_P_KeepaliveTimer");
	return timer->add_interval(timer->gettick() + ping_interval*1000, Sql_P_KeepaliveTimer, 0, (intptr_t)self, ping_interval*1000);
}

/// Escapes a string.
size_t Sql_EscapeString(struct Sql *self, char *out_to, const char *from)
{
	if (self != NULL)
		return (size_t)mysql_real_escape_string(&self->handle, out_to, from, (unsigned long)strlen(from));
	else
		return (size_t)mysql_escape_string(out_to, from, (unsigned long)strlen(from));
}

/// Escapes a string.
size_t Sql_EscapeStringLen(struct Sql *self, char *out_to, const char *from, size_t from_len)
{
	if (self != NULL)
		return (size_t)mysql_real_escape_string(&self->handle, out_to, from, (unsigned long)from_len);
	else
		return (size_t)mysql_escape_string(out_to, from, (unsigned long)from_len);
}

/// Executes a query.
int Sql_Query(struct Sql *self, const char *query, ...) __attribute__((format(printf, 2, 3)));
int Sql_Query(struct Sql *self, const char *query, ...)
{
	int res;
	va_list args;

	va_start(args, query);
	res = SQL->QueryV(self, query, args);
	va_end(args);

	return res;
}

/// Executes a query.
int Sql_QueryV(struct Sql *self, const char *query, va_list args)
{
	if( self == NULL )
		return SQL_ERROR;

	SQL->FreeResult(self);
	StrBuf->Clear(&self->buf);
	StrBuf->Vprintf(&self->buf, query, args);
	if( mysql_real_query(&self->handle, StrBuf->Value(&self->buf), (unsigned long)StrBuf->Length(&self->buf)) )
	{
		ShowSQL("DB error - %s\n", mysql_error(&self->handle));
		hercules_mysql_error_handler(mysql_errno(&self->handle));
		return SQL_ERROR;
	}
	self->result = mysql_store_result(&self->handle);
	if( mysql_errno(&self->handle) != 0 )
	{
		ShowSQL("DB error - %s\n", mysql_error(&self->handle));
		hercules_mysql_error_handler(mysql_errno(&self->handle));
		return SQL_ERROR;
	}
	return SQL_SUCCESS;
}

/// Executes a query.
int Sql_QueryStr(struct Sql *self, const char *query)
{
	if( self == NULL )
		return SQL_ERROR;

	SQL->FreeResult(self);
	StrBuf->Clear(&self->buf);
	StrBuf->AppendStr(&self->buf, query);
	if( mysql_real_query(&self->handle, StrBuf->Value(&self->buf), (unsigned long)StrBuf->Length(&self->buf)) )
	{
		ShowSQL("DB error - %s\n", mysql_error(&self->handle));
		hercules_mysql_error_handler(mysql_errno(&self->handle));
		return SQL_ERROR;
	}
	self->result = mysql_store_result(&self->handle);
	if( mysql_errno(&self->handle) != 0 )
	{
		ShowSQL("DB error - %s\n", mysql_error(&self->handle));
		hercules_mysql_error_handler(mysql_errno(&self->handle));
		return SQL_ERROR;
	}
	return SQL_SUCCESS;
}

/// Returns the number of the AUTO_INCREMENT column of the last INSERT/UPDATE query.
uint64 Sql_LastInsertId(struct Sql *self)
{
	if (self != NULL)
		return (uint64)mysql_insert_id(&self->handle);
	else
		return 0;
}

/// Returns the number of columns in each row of the result.
uint32 Sql_NumColumns(struct Sql *self)
{
	if (self != NULL && self->result != NULL)
		return (uint32)mysql_num_fields(self->result);
	return 0;
}

/// Returns the number of rows in the result.
uint64 Sql_NumRows(struct Sql *self)
{
	if (self != NULL && self->result != NULL)
		return (uint64)mysql_num_rows(self->result);
	return 0;
}

/// Fetches the next row.
int Sql_NextRow(struct Sql *self)
{
	if (self != NULL && self->result != NULL) {
		self->row = mysql_fetch_row(self->result);
		if( self->row ) {
			self->lengths = mysql_fetch_lengths(self->result);
			return SQL_SUCCESS;
		}
		self->lengths = NULL;
		if( mysql_errno(&self->handle) == 0 )
			return SQL_NO_DATA;
	}
	return SQL_ERROR;
}

/// Gets the data of a column.
int Sql_GetData(struct Sql *self, size_t col, char **out_buf, size_t *out_len)
{
	if( self && self->row ) {
		if( col < SQL->NumColumns(self) ) {
			if( out_buf ) *out_buf = self->row[col];
			if( out_len ) *out_len = (size_t)self->lengths[col];
		} else {// out of range - ignore
			if( out_buf ) *out_buf = NULL;
			if( out_len ) *out_len = 0;
		}
		return SQL_SUCCESS;
	}
	return SQL_ERROR;
}

/// Frees the result of the query.
void Sql_FreeResult(struct Sql *self)
{
	if( self && self->result ) {
		mysql_free_result(self->result);
		self->result = NULL;
		self->row = NULL;
		self->lengths = NULL;
	}
}

/// Shows debug information (last query).
void Sql_ShowDebug_(struct Sql *self, const char *debug_file, const unsigned long debug_line)
{
	if( self == NULL )
		ShowDebug("at %s:%lu - self is NULL\n", debug_file, debug_line);
	else if( StrBuf->Length(&self->buf) > 0 )
		ShowDebug("at %s:%lu - %s\n", debug_file, debug_line, StrBuf->Value(&self->buf));
	else
		ShowDebug("at %s:%lu\n", debug_file, debug_line);
}

/// Frees a Sql handle returned by Sql_Malloc.
void Sql_Free(struct Sql *self)
{
	if( self )
	{
		SQL->FreeResult(self);
		StrBuf->Destroy(&self->buf);
		if( self->keepalive != INVALID_TIMER ) timer->delete(self->keepalive, Sql_P_KeepaliveTimer);
		mysql_close(&self->handle);
		aFree(self);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Prepared Statements
///////////////////////////////////////////////////////////////////////////////

/// Returns the mysql integer type for the target size.
///
/// @private
static enum enum_field_types Sql_P_SizeToMysqlIntType(int sz)
{
	switch( sz )
	{
	case 1: return MYSQL_TYPE_TINY;
	case 2: return MYSQL_TYPE_SHORT;
	case 4: return MYSQL_TYPE_LONG;
	case 8: return MYSQL_TYPE_LONGLONG;
	default:
		ShowDebug("SizeToMysqlIntType: unsupported size (%d)\n", sz);
		return MYSQL_TYPE_NULL;
	}
}

/// Binds a parameter/result.
///
/// @private
static int Sql_P_BindSqlDataType(MYSQL_BIND* bind, enum SqlDataType buffer_type, void* buffer, size_t buffer_len, unsigned long* out_length, int8* out_is_null)
{
	nullpo_retr(SQL_ERROR, bind);
	memset(bind, 0, sizeof(MYSQL_BIND));
	switch( buffer_type )
	{
	case SQLDT_NULL: bind->buffer_type = MYSQL_TYPE_NULL;
		buffer_len = 0;// FIXME length = ? [FlavioJS]
		break;
		// fixed size
	case SQLDT_UINT8: bind->is_unsigned = 1;
		FALLTHROUGH
	case SQLDT_INT8: bind->buffer_type = MYSQL_TYPE_TINY;
		buffer_len = 1;
		break;
	case SQLDT_UINT16: bind->is_unsigned = 1;
		FALLTHROUGH
	case SQLDT_INT16: bind->buffer_type = MYSQL_TYPE_SHORT;
		buffer_len = 2;
		break;
	case SQLDT_UINT32: bind->is_unsigned = 1;
		FALLTHROUGH
	case SQLDT_INT32: bind->buffer_type = MYSQL_TYPE_LONG;
		buffer_len = 4;
		break;
	case SQLDT_UINT64: bind->is_unsigned = 1;
		FALLTHROUGH
	case SQLDT_INT64: bind->buffer_type = MYSQL_TYPE_LONGLONG;
		buffer_len = 8;
		break;
		// platform dependent size
	case SQLDT_UCHAR: bind->is_unsigned = 1;
		FALLTHROUGH
	case SQLDT_CHAR: bind->buffer_type = Sql_P_SizeToMysqlIntType(sizeof(char));
		buffer_len = sizeof(char);
		break;
	case SQLDT_USHORT: bind->is_unsigned = 1;
		FALLTHROUGH
	case SQLDT_SHORT: bind->buffer_type = Sql_P_SizeToMysqlIntType(sizeof(short));
		buffer_len = sizeof(short);
		break;
	case SQLDT_UINT: bind->is_unsigned = 1;
		FALLTHROUGH
	case SQLDT_INT: bind->buffer_type = Sql_P_SizeToMysqlIntType(sizeof(int));
		buffer_len = sizeof(int);
		break;
	case SQLDT_ULONG: bind->is_unsigned = 1;
		FALLTHROUGH
	case SQLDT_LONG: bind->buffer_type = Sql_P_SizeToMysqlIntType(sizeof(long));
		buffer_len = sizeof(long);
		break;
	case SQLDT_ULONGLONG: bind->is_unsigned = 1;
		FALLTHROUGH
	case SQLDT_LONGLONG: bind->buffer_type = Sql_P_SizeToMysqlIntType(sizeof(int64));
		buffer_len = sizeof(int64);
		break;
		// floating point
	case SQLDT_FLOAT: bind->buffer_type = MYSQL_TYPE_FLOAT;
		buffer_len = 4;
		break;
	case SQLDT_DOUBLE: bind->buffer_type = MYSQL_TYPE_DOUBLE;
		buffer_len = 8;
		break;
		// other
	case SQLDT_STRING:
	case SQLDT_ENUM: bind->buffer_type = MYSQL_TYPE_STRING;
		break;
	case SQLDT_BLOB: bind->buffer_type = MYSQL_TYPE_BLOB;
		break;
	default:
		ShowDebug("Sql_P_BindSqlDataType: unsupported buffer type (%u)\n", buffer_type);
		return SQL_ERROR;
	}
	bind->buffer = buffer;
	bind->buffer_length = (unsigned long)buffer_len;
	bind->length = out_length;
	bind->is_null = (my_bool*)out_is_null;
	return SQL_SUCCESS;
}

/// Prints debug information about a field (type and length).
///
/// @private
static void Sql_P_ShowDebugMysqlFieldInfo(const char* prefix, enum enum_field_types type, int is_unsigned, unsigned long length, const char* length_postfix)
{
	const char *sign = (is_unsigned ? "UNSIGNED " : "");
	const char *type_string = NULL;
	switch (type) {
		default:
			ShowDebug("%stype=%s%u, length=%lu\n", prefix, sign, type, length);
			return;
#define SHOW_DEBUG_OF(x) case x: type_string = #x; break
		SHOW_DEBUG_OF(MYSQL_TYPE_TINY);
		SHOW_DEBUG_OF(MYSQL_TYPE_SHORT);
		SHOW_DEBUG_OF(MYSQL_TYPE_LONG);
		SHOW_DEBUG_OF(MYSQL_TYPE_INT24);
		SHOW_DEBUG_OF(MYSQL_TYPE_LONGLONG);
		SHOW_DEBUG_OF(MYSQL_TYPE_DECIMAL);
		SHOW_DEBUG_OF(MYSQL_TYPE_FLOAT);
		SHOW_DEBUG_OF(MYSQL_TYPE_DOUBLE);
		SHOW_DEBUG_OF(MYSQL_TYPE_TIMESTAMP);
		SHOW_DEBUG_OF(MYSQL_TYPE_DATE);
		SHOW_DEBUG_OF(MYSQL_TYPE_TIME);
		SHOW_DEBUG_OF(MYSQL_TYPE_DATETIME);
		SHOW_DEBUG_OF(MYSQL_TYPE_YEAR);
		SHOW_DEBUG_OF(MYSQL_TYPE_STRING);
		SHOW_DEBUG_OF(MYSQL_TYPE_VAR_STRING);
		SHOW_DEBUG_OF(MYSQL_TYPE_BLOB);
		SHOW_DEBUG_OF(MYSQL_TYPE_SET);
		SHOW_DEBUG_OF(MYSQL_TYPE_ENUM);
		SHOW_DEBUG_OF(MYSQL_TYPE_NULL);
#undef SHOW_DEBUG_TYPE_OF
	}
	ShowDebug("%stype=%s%s, length=%lu%s\n", prefix, sign, type_string, length, length_postfix);
}

/// Reports debug information about a truncated column.
///
/// @private
static void SqlStmt_P_ShowDebugTruncatedColumn(struct SqlStmt *self, size_t i)
{
	MYSQL_RES* meta;
	MYSQL_FIELD* field;
	MYSQL_BIND* column;

	nullpo_retv(self);
	meta = mysql_stmt_result_metadata(self->stmt);
	field = mysql_fetch_field_direct(meta, (unsigned int)i);
	ShowSQL("DB error - data of field '%s' was truncated.\n", field->name);
	ShowDebug("column - %lu\n", (unsigned long)i);
	Sql_P_ShowDebugMysqlFieldInfo("data   - ", field->type, field->flags&UNSIGNED_FLAG, self->column_lengths[i].length, "");
	column = &self->columns[i];
	if( column->buffer_type == MYSQL_TYPE_STRING )
		Sql_P_ShowDebugMysqlFieldInfo("buffer - ", column->buffer_type, column->is_unsigned, column->buffer_length, "+1(null-terminator)");
	else
		Sql_P_ShowDebugMysqlFieldInfo("buffer - ", column->buffer_type, column->is_unsigned, column->buffer_length, "");
	mysql_free_result(meta);
}

/// Allocates and initializes a new SqlStmt handle.
struct SqlStmt *SqlStmt_Malloc(struct Sql *sql)
{
	struct SqlStmt *self;
	MYSQL_STMT* stmt;

	if( sql == NULL )
		return NULL;

	stmt = mysql_stmt_init(&sql->handle);
	if( stmt == NULL ) {
		ShowSQL("DB error - %s\n", mysql_error(&sql->handle));
		return NULL;
	}
	CREATE(self, struct SqlStmt, 1);
	StrBuf->Init(&self->buf);
	self->stmt = stmt;
	self->params = NULL;
	self->columns = NULL;
	self->column_lengths = NULL;
	self->max_params = 0;
	self->max_columns = 0;
	self->bind_params = false;
	self->bind_columns = false;

	return self;
}

/// Prepares the statement.
int SqlStmt_Prepare(struct SqlStmt *self, const char *query, ...) __attribute__((format(printf, 2, 3)));
int SqlStmt_Prepare(struct SqlStmt *self, const char *query, ...)
{
	int res;
	va_list args;

	va_start(args, query);
	res = SQL->StmtPrepareV(self, query, args);
	va_end(args);

	return res;
}

/// Prepares the statement.
int SqlStmt_PrepareV(struct SqlStmt *self, const char *query, va_list args)
{
	if( self == NULL )
		return SQL_ERROR;

	SQL->StmtFreeResult(self);
	StrBuf->Clear(&self->buf);
	StrBuf->Vprintf(&self->buf, query, args);
	if( mysql_stmt_prepare(self->stmt, StrBuf->Value(&self->buf), (unsigned long)StrBuf->Length(&self->buf)) )
	{
		ShowSQL("DB error - %s\n", mysql_stmt_error(self->stmt));
		hercules_mysql_error_handler(mysql_stmt_errno(self->stmt));
		return SQL_ERROR;
	}
	self->bind_params = false;

	return SQL_SUCCESS;
}

/// Prepares the statement.
int SqlStmt_PrepareStr(struct SqlStmt *self, const char *query)
{
	if( self == NULL )
		return SQL_ERROR;

	SQL->StmtFreeResult(self);
	StrBuf->Clear(&self->buf);
	StrBuf->AppendStr(&self->buf, query);
	if( mysql_stmt_prepare(self->stmt, StrBuf->Value(&self->buf), (unsigned long)StrBuf->Length(&self->buf)) )
	{
		ShowSQL("DB error - %s\n", mysql_stmt_error(self->stmt));
		hercules_mysql_error_handler(mysql_stmt_errno(self->stmt));
		return SQL_ERROR;
	}
	self->bind_params = false;

	return SQL_SUCCESS;
}

/// Returns the number of parameters in the prepared statement.
size_t SqlStmt_NumParams(struct SqlStmt *self)
{
	if( self )
		return (size_t)mysql_stmt_param_count(self->stmt);
	else
		return 0;
}

/// Binds a parameter to a buffer.
int SqlStmt_BindParam(struct SqlStmt *self, size_t idx, enum SqlDataType buffer_type, const void *buffer, size_t buffer_len)
{
	if( self == NULL )
	return SQL_ERROR;

	if( !self->bind_params )
	{// initialize the bindings
		size_t i;
		size_t count;

		count = SQL->StmtNumParams(self);
		if( self->max_params < count )
		{
			self->max_params = count;
			RECREATE(self->params, MYSQL_BIND, count);
		}
		memset(self->params, 0, count*sizeof(MYSQL_BIND));
		for( i = 0; i < count; ++i )
			self->params[i].buffer_type = MYSQL_TYPE_NULL;
		self->bind_params = true;
	}
	if (idx >= self->max_params)
		return SQL_SUCCESS; // out of range - ignore

PRAGMA_GCC46(GCC diagnostic push)
PRAGMA_GCC46(GCC diagnostic ignored "-Wcast-qual")
	/*
	 * MySQL uses the same struct with a non-const buffer for both
	 * parameters (input) and columns (output).
	 * As such, we get to close our eyes and pretend we didn't see we're
	 * dropping a const qualifier here.
	 */
	return Sql_P_BindSqlDataType(self->params+idx, buffer_type, (void *)buffer, buffer_len, NULL, NULL);
PRAGMA_GCC46(GCC diagnostic pop)
}

/// Executes the prepared statement.
int SqlStmt_Execute(struct SqlStmt *self)
{
	if( self == NULL )
		return SQL_ERROR;

	SQL->StmtFreeResult(self);
	if( (self->bind_params && mysql_stmt_bind_param(self->stmt, self->params)) ||
		mysql_stmt_execute(self->stmt) )
	{
		ShowSQL("DB error - %s\n", mysql_stmt_error(self->stmt));
		hercules_mysql_error_handler(mysql_stmt_errno(self->stmt));
		return SQL_ERROR;
	}
	self->bind_columns = false;
	if( mysql_stmt_store_result(self->stmt) )// store all the data
	{
		ShowSQL("DB error - %s\n", mysql_stmt_error(self->stmt));
		hercules_mysql_error_handler(mysql_stmt_errno(self->stmt));
		return SQL_ERROR;
	}

	return SQL_SUCCESS;
}

/// Returns the number of the AUTO_INCREMENT column of the last INSERT/UPDATE statement.
uint64 SqlStmt_LastInsertId(struct SqlStmt *self)
{
	if( self )
		return (uint64)mysql_stmt_insert_id(self->stmt);
	else
		return 0;
}

/// Returns the number of columns in each row of the result.
size_t SqlStmt_NumColumns(struct SqlStmt *self)
{
	if( self )
		return (size_t)mysql_stmt_field_count(self->stmt);
	else
		return 0;
}

/// Binds the result of a column to a buffer.
int SqlStmt_BindColumn(struct SqlStmt *self, size_t idx, enum SqlDataType buffer_type, void *buffer, size_t buffer_len, uint32 *out_length, int8 *out_is_null)
{
	if (self == NULL)
		return SQL_ERROR;

	if (buffer_type == SQLDT_STRING || buffer_type == SQLDT_ENUM) {
		if (buffer_len < 1) {
			ShowDebug("SqlStmt_BindColumn: buffer_len(%"PRIuS") is too small, no room for the null-terminator\n", buffer_len);
			return SQL_ERROR;
		}
		--buffer_len;// null-terminator
	}
	if( !self->bind_columns )
	{// initialize the bindings
		size_t i;
		size_t cols;

		cols = SQL->StmtNumColumns(self);
		if( self->max_columns < cols )
		{
			self->max_columns = cols;
			RECREATE(self->columns, MYSQL_BIND, cols);
			RECREATE(self->column_lengths, s_column_length, cols);
		}
		memset(self->columns, 0, cols*sizeof(MYSQL_BIND));
		memset(self->column_lengths, 0, cols*sizeof(s_column_length));
		for( i = 0; i < cols; ++i )
			self->columns[i].buffer_type = MYSQL_TYPE_NULL;
		self->bind_columns = true;
	}
	if( idx < self->max_columns )
	{
		self->column_lengths[idx].out_length = out_length;
		return Sql_P_BindSqlDataType(self->columns+idx, buffer_type, buffer, buffer_len, &self->column_lengths[idx].length, out_is_null);
	}
	else
	{
		return SQL_SUCCESS;// out of range - ignore
	}
}

/// Returns the number of rows in the result.
uint64 SqlStmt_NumRows(struct SqlStmt *self)
{
	if (self != NULL)
		return (uint64)mysql_stmt_num_rows(self->stmt);
	else
		return 0;
}

/// Fetches the next row.
int SqlStmt_NextRow(struct SqlStmt *self)
{
	int err;
	size_t i;
	size_t cols;

	if (self == NULL)
		return SQL_ERROR;

	// bind columns
	if( self->bind_columns && mysql_stmt_bind_result(self->stmt, self->columns) )
		err = 1;// error binding columns
	else
		err = mysql_stmt_fetch(self->stmt);// fetch row

	// check for errors
	if (err == MYSQL_NO_DATA)
		return SQL_NO_DATA;
	if (err == MYSQL_DATA_TRUNCATED) {
		my_bool truncated;

		if (!self->bind_columns) {
			ShowSQL("DB error - data truncated (unknown source, columns are not bound)\n");
			return SQL_ERROR;
		}

		// find truncated column
		cols = SQL->StmtNumColumns(self);
		for (i = 0; i < cols; ++i) {
			MYSQL_BIND *column = &self->columns[i];
			column->error = &truncated;
			mysql_stmt_fetch_column(self->stmt, column, (unsigned int)i, 0);
			column->error = NULL;
			if (truncated) {
				// report truncated column
				SqlStmt_P_ShowDebugTruncatedColumn(self, i);
				return SQL_ERROR;
			}
		}
		ShowSQL("DB error - data truncated (unknown source)\n");
		return SQL_ERROR;
	}
	if (err) {
		ShowSQL("DB error - %s\n", mysql_stmt_error(self->stmt));
		hercules_mysql_error_handler(mysql_stmt_errno(self->stmt));
		return SQL_ERROR;
	}

	// propagate column lengths and clear unused parts of string/enum/blob buffers
	cols = SQL->StmtNumColumns(self);
	for (i = 0; i < cols; ++i) {
		unsigned long length = self->column_lengths[i].length;
		MYSQL_BIND *column = &self->columns[i];
		if (self->column_lengths[i].out_length)
			*self->column_lengths[i].out_length = (uint32)length;
		if (column->buffer_type == MYSQL_TYPE_STRING) {
			// clear unused part of the string/enum buffer (and null-terminate)
			memset((char*)column->buffer + length, 0, column->buffer_length - length + 1);
		} else if (column->buffer_type == MYSQL_TYPE_BLOB && length < column->buffer_length) {
			// clear unused part of the blob buffer
			memset((char*)column->buffer + length, 0, column->buffer_length - length);
		}
	}

	return SQL_SUCCESS;
}

/// Frees the result of the statement execution.
void SqlStmt_FreeResult(struct SqlStmt *self)
{
	if( self )
		mysql_stmt_free_result(self->stmt);
}

/// Shows debug information (with statement).
void SqlStmt_ShowDebug_(struct SqlStmt *self, const char *debug_file, const unsigned long debug_line)
{
	if( self == NULL )
		ShowDebug("at %s:%lu -  self is NULL\n", debug_file, debug_line);
	else if( StrBuf->Length(&self->buf) > 0 )
		ShowDebug("at %s:%lu - %s\n", debug_file, debug_line, StrBuf->Value(&self->buf));
	else
		ShowDebug("at %s:%lu\n", debug_file, debug_line);
}

/// Frees a SqlStmt returned by SqlStmt_Malloc.
void SqlStmt_Free(struct SqlStmt *self)
{
	if( self )
	{
		SqlStmt_FreeResult(self);
		StrBuf->Destroy(&self->buf);
		mysql_stmt_close(self->stmt);
		if( self->params )
			aFree(self->params);
		if( self->columns )
		{
			aFree(self->columns);
			aFree(self->column_lengths);
		}
		aFree(self);
	}
}

/* receives mysql error codes during runtime (not on first-time-connects) */
void hercules_mysql_error_handler(unsigned int ecode)
{
	switch( ecode ) {
	case 2003:/* Can't connect to MySQL (this error only happens here when failing to reconnect) */
		if( mysql_reconnect_type == 1 ) {
			static int retry = 1;
			if( ++retry > mysql_reconnect_count ) {
				ShowFatalError("MySQL has been unreachable for too long, %d reconnects were attempted. Shutting Down\n", retry);
				exit(EXIT_FAILURE);
			}
		}
		break;
	}
}

/**
 * Parses mysql_reconnect from inter_configuration.
 *
 * @param filename Path to configuration file.
 * @param imported Whether the current config is imported from another file.
 *
 * @retval false in case of error.
 */
bool Sql_inter_server_read(const char *filename, bool imported)
{
	struct config_t config;
	const struct config_setting_t *setting = NULL;
	const char *import = NULL;
	bool retval = true;

	nullpo_retr(false, filename);

	if (!libconfig->load_file(&config, filename))
		return false;

	if ((setting = libconfig->lookup(&config, "inter_configuration/mysql_reconnect")) == NULL) {
		config_destroy(&config);
		if (imported)
			return true;
		ShowError("Sql_inter_server_read: inter_configuration/mysql_reconnect was not found in %s!\n", filename);
		return false;
	}

	if (libconfig->setting_lookup_int(setting, "type", &mysql_reconnect_type) == CONFIG_TRUE) {
		if (mysql_reconnect_type != 1 && mysql_reconnect_type != 2) {
			ShowError("%s::inter_configuration/mysql_reconnect/type is set to %d which is not valid, defaulting to 1...\n", filename, mysql_reconnect_type);
			mysql_reconnect_type = 1;
		}
	}
	if (libconfig->setting_lookup_int(setting, "count", &mysql_reconnect_count) == CONFIG_TRUE) {
		if (mysql_reconnect_count < 1)
			mysql_reconnect_count = 1;
	}

	// import should overwrite any previous configuration, so it should be called last
	if (libconfig->lookup_string(&config, "import", &import) == CONFIG_TRUE) {
		if (strcmp(import, filename) == 0 || strcmp(import, "conf/common/inter-server.conf") == 0) { // FIXME: Hardcoded path
			ShowWarning("Sql_inter_server_read: Loop detected in %s! Skipping 'import'...\n", filename);
		} else {
			if (!Sql_inter_server_read(import, true))
				retval = false;
		}
	}

	libconfig->destroy(&config);
	return retval;
}

void Sql_HerculesUpdateCheck(struct Sql *self)
{
	char line[22];// "yyyy-mm-dd--hh-mm" (17) + ".sql" (4) + 1
	FILE* ifp;/* index fp */
	unsigned int performed = 0;
	StringBuf buf;

	if( self == NULL )
		return;/* return silently, build has no mysql connection */

	if( !( ifp = fopen("sql-files/upgrades/index.txt", "r") ) ) {
		ShowError("SQL upgrade index was not found!\n");
		return;
	}

	StrBuf->Init(&buf);

	while(fgets(line, sizeof(line), ifp)) {
		char path[41];// "sql-files/upgrades/" (19) + "yyyy-mm-dd--hh-mm" (17) + ".sql" (4) + 1
		char timestamp[11];// "1360186680" (10) + 1
		FILE* ufp;/* upgrade fp */

		if( line[0] == '\n' || line[0] == '\r' || ( line[0] == '/' && line[1] == '/' ) )/* skip \n, \r and "//" comments */
			continue;

		sprintf(path,"sql-files/upgrades/%s",line);

		if( !( ufp = fopen(path, "r") ) ) {
			ShowError("SQL upgrade file %s was not found!\n",path);
			continue;
		}

		if( fgetc(ufp) != '#' ) {
			fclose(ufp);
			continue;
		}

		if (fseek(ufp,1,SEEK_SET) == 0 /* woo. skip the # */
		 && fgets(timestamp,sizeof(timestamp),ufp)) {
			unsigned int timestampui = (unsigned int)atol(timestamp);
			if( SQL_ERROR == SQL->Query(self, "SELECT 1 FROM `sql_updates` WHERE `timestamp` = '%u' LIMIT 1", timestampui) )
				Sql_ShowDebug(self);
			if( Sql_NumRows(self) != 1 ) {
				StrBuf->Printf(&buf,CL_MAGENTA"[SQL]"CL_RESET": -- '"CL_WHITE"%s"CL_RESET"'\n", path);
				performed++;
			}
		}

		fclose(ufp);
	}

	fclose(ifp);

	if( performed ) {
		ShowSQL("- detected %u new "CL_WHITE"SQL updates"CL_RESET"\n",performed);
		ShowMessage("%s",StrBuf->Value(&buf));
		ShowSQL("To manually skip, type: 'sql update skip <file name>'\n");
	}

	StrBuf->Destroy(&buf);
}

void Sql_HerculesUpdateSkip(struct Sql *self, const char *filename)
{
	char path[41];// "sql-files/upgrades/" (19) + "yyyy-mm-dd--hh-mm" (17) + ".sql" (4) + 1
	char timestamp[11];// "1360186680" (10) + 1
	FILE* ifp;/* index fp */

	if( !self ) {
		ShowError("SQL not hooked!\n");
		return;
	}

	snprintf(path,41,"sql-files/upgrades/%s",filename);

	if( !( ifp = fopen(path, "r") ) ) {
		ShowError("Upgrade file '%s' was not found!\n",filename);
		return;
	}

	if (fseek (ifp,1,SEEK_SET) == 0 /* woo. skip the # */
	 && fgets(timestamp,sizeof(timestamp),ifp)) {
		unsigned int timestampui = (unsigned int)atol(timestamp);
		if( SQL_ERROR == SQL->Query(self, "SELECT 1 FROM `sql_updates` WHERE `timestamp` = '%u' LIMIT 1", timestampui) )
			Sql_ShowDebug(self);
		else if( Sql_NumRows(self) == 1 ) {
			ShowError("Upgrade '%s' has already been skipped\n",filename);
		} else {
			if( SQL_ERROR == SQL->Query(self, "INSERT INTO `sql_updates` (`timestamp`,`ignored`) VALUES ('%u','Yes') ", timestampui) )
				Sql_ShowDebug(self);
			else {
				ShowInfo("SQL Upgrade '%s' successfully skipped\n",filename);
			}
		}
	}
	fclose(ifp);
	return;
}

void Sql_Init(void)
{
	Sql_inter_server_read("conf/common/inter-server.conf", false); // FIXME: Hardcoded path
}

void sql_defaults(void)
{
	SQL = &sql_s;

	SQL->Connect = Sql_Connect;
	SQL->GetTimeout = Sql_GetTimeout;
	SQL->GetColumnNames = Sql_GetColumnNames;
	SQL->SetEncoding = Sql_SetEncoding;
	SQL->Ping = Sql_Ping;
	SQL->EscapeString = Sql_EscapeString;
	SQL->EscapeStringLen = Sql_EscapeStringLen;
	SQL->Query = Sql_Query;
	SQL->QueryV = Sql_QueryV;
	SQL->QueryStr = Sql_QueryStr;
	SQL->LastInsertId = Sql_LastInsertId;
	SQL->NumColumns = Sql_NumColumns;
	SQL->NumRows = Sql_NumRows;
	SQL->NextRow = Sql_NextRow;
	SQL->GetData = Sql_GetData;
	SQL->FreeResult = Sql_FreeResult;
	SQL->ShowDebug_ = Sql_ShowDebug_;
	SQL->Free = Sql_Free;
	SQL->Malloc = Sql_Malloc;

	/* SqlStmt defaults [Susu] */
	SQL->StmtBindColumn = SqlStmt_BindColumn;
	SQL->StmtBindParam = SqlStmt_BindParam;
	SQL->StmtExecute = SqlStmt_Execute;
	SQL->StmtFree = SqlStmt_Free;
	SQL->StmtFreeResult = SqlStmt_FreeResult;
	SQL->StmtLastInsertId = SqlStmt_LastInsertId;
	SQL->StmtMalloc = SqlStmt_Malloc;
	SQL->StmtNextRow = SqlStmt_NextRow;
	SQL->StmtNumColumns = SqlStmt_NumColumns;
	SQL->StmtNumParams = SqlStmt_NumParams;
	SQL->StmtNumRows = SqlStmt_NumRows;
	SQL->StmtPrepare = SqlStmt_Prepare;
	SQL->StmtPrepareStr = SqlStmt_PrepareStr;
	SQL->StmtPrepareV = SqlStmt_PrepareV;
	SQL->StmtShowDebug_ = SqlStmt_ShowDebug_;
}
