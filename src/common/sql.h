// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _COMMON_SQL_H_
#define _COMMON_SQL_H_

#include <stdarg.h>// va_list

#include "../common/cbasetypes.h"

// Return codes
#define SQL_ERROR (-1)
#define SQL_SUCCESS 0
#define SQL_NO_DATA 100

// macro definition to determine whether the mySQL engine is running on InnoDB (rather than MyISAM)
// uncomment this line if the your mySQL tables have been changed to run on InnoDB
// this macro will adjust how logs are recorded in the database to accommodate the change
//#define SQL_INNODB

/// Data type identifier.
/// String, enum and blob data types need the buffer length specified.
enum SqlDataType {
	SQLDT_NULL,
	// fixed size
	SQLDT_INT8,
	SQLDT_INT16,
	SQLDT_INT32,
	SQLDT_INT64,
	SQLDT_UINT8,
	SQLDT_UINT16,
	SQLDT_UINT32,
	SQLDT_UINT64,
	// platform dependent size
	SQLDT_CHAR,
	SQLDT_SHORT,
	SQLDT_INT,
	SQLDT_LONG,
	SQLDT_LONGLONG,
	SQLDT_UCHAR,
	SQLDT_USHORT,
	SQLDT_UINT,
	SQLDT_ULONG,
	SQLDT_ULONGLONG,
	// floating point
	SQLDT_FLOAT,
	SQLDT_DOUBLE,
	// other
	SQLDT_STRING,
	SQLDT_ENUM,
	// Note: An ENUM is a string with restricted values. When an invalid value
	//       is inserted, it is saved as an empty string (numerical value 0).
	SQLDT_BLOB,
	SQLDT_LASTID
};

struct Sql;// Sql handle (private access)
struct SqlStmt;// Sql statement (private access)

typedef enum SqlDataType SqlDataType;
typedef struct Sql Sql;
typedef struct SqlStmt SqlStmt;

struct sql_interface {
	/// Establishes a connection.
	///
	/// @return SQL_SUCCESS or SQL_ERROR
	int (*Connect) (Sql* self, const char* user, const char* passwd, const char* host, uint16 port, const char* db);
	/// Retrieves the timeout of the connection.
	///
	/// @return SQL_SUCCESS or SQL_ERROR
	int (*GetTimeout) (Sql* self, uint32* out_timeout);
	/// Retrieves the name of the columns of a table into out_buf, with the separator after each name.
	///
	/// @return SQL_SUCCESS or SQL_ERROR
	int (*GetColumnNames) (Sql* self, const char* table, char* out_buf, size_t buf_len, char sep);
	/// Changes the encoding of the connection.
	///
	/// @return SQL_SUCCESS or SQL_ERROR
	int (*SetEncoding) (Sql* self, const char* encoding);
	/// Pings the connection.
	///
	/// @return SQL_SUCCESS or SQL_ERROR
	int (*Ping) (Sql* self);
	/// Escapes a string.
	/// The output buffer must be at least strlen(from)*2+1 in size.
	///
	/// @return The size of the escaped string
	size_t (*EscapeString) (Sql* self, char* out_to, const char* from);
	/// Escapes a string.
	/// The output buffer must be at least from_len*2+1 in size.
	///
	/// @return The size of the escaped string
	size_t (*EscapeStringLen) (Sql* self, char* out_to, const char* from, size_t from_len);
	/// Executes a query.
	/// Any previous result is freed.
	/// The query is constructed as if it was sprintf.
	///
	/// @return SQL_SUCCESS or SQL_ERROR
	int (*Query) (Sql* self, const char* query, ...);
	/// Executes a query.
	/// Any previous result is freed.
	/// The query is constructed as if it was svprintf.
	///
	/// @return SQL_SUCCESS or SQL_ERROR
	int (*QueryV) (Sql* self, const char* query, va_list args);
	/// Executes a query.
	/// Any previous result is freed.
	/// The query is used directly.
	///
	/// @return SQL_SUCCESS or SQL_ERROR
	int (*QueryStr) (Sql* self, const char* query);
	/// Returns the number of the AUTO_INCREMENT column of the last INSERT/UPDATE query.
	///
	/// @return Value of the auto-increment column
	uint64 (*LastInsertId) (Sql* self);
	/// Returns the number of columns in each row of the result.
	///
	/// @return Number of columns
	uint32 (*NumColumns) (Sql* self);
	/// Returns the number of rows in the result.
	///
	/// @return Number of rows
	uint64 (*NumRows) (Sql* self);
	/// Fetches the next row.
	/// The data of the previous row is no longer valid.
	///
	/// @return SQL_SUCCESS, SQL_ERROR or SQL_NO_DATA
	int (*NextRow) (Sql* self);
	/// Gets the data of a column.
	/// The data remains valid until the next row is fetched or the result is freed.
	///
	/// @return SQL_SUCCESS or SQL_ERROR
	int (*GetData) (Sql* self, size_t col, char** out_buf, size_t* out_len);
	/// Frees the result of the query.
	void (*FreeResult) (Sql* self);
	/// Shows debug information (last query).
	void (*ShowDebug_) (Sql* self, const char* debug_file, const unsigned long debug_line);
	/// Frees a Sql handle returned by Sql_Malloc.
	void (*Free) (Sql* self);
	/// Allocates and initializes a new Sql handle.
	struct Sql *(*Malloc) (void);



	///////////////////////////////////////////////////////////////////////////////
	// Prepared Statements
	///////////////////////////////////////////////////////////////////////////////
	// Parameters are placed in the statement by embedding question mark ('?')
	// characters into the query at the appropriate positions.
	// The markers are legal only in places where they represent data.
	// The markers cannot be inside quotes. Quotes will be added automatically
	// when they are required.
	//
	// example queries with parameters:
	// 1) SELECT col FROM table WHERE id=?
	// 2) INSERT INTO table(col1,col2) VALUES(?,?)


	/*=====================================
	SQL Statement interface [Susu]
	*-------------------------------------*/

	/// Allocates and initializes a new SqlStmt handle.
	/// It uses the connection of the parent Sql handle.
	/// Queries in Sql and SqlStmt are independent and don't affect each other.
	///
	/// @return SqlStmt handle or NULL if an error occured
	struct SqlStmt* (*StmtMalloc)(Sql* sql);



	/// Prepares the statement.
	/// Any previous result is freed and all parameter bindings are removed.
	/// The query is constructed as if it was sprintf.
	///
	/// @return SQL_SUCCESS or SQL_ERROR
	int (*StmtPrepare)(SqlStmt* self, const char* query, ...);

	/// Prepares the statement.
	/// Any previous result is freed and all parameter bindings are removed.
	/// The query is constructed as if it was svprintf.
	///
	/// @return SQL_SUCCESS or SQL_ERROR
	int (*StmtPrepareV)(SqlStmt* self, const char* query, va_list args);



	/// Prepares the statement.
	/// Any previous result is freed and all parameter bindings are removed.
	/// The query is used directly.
	///
	/// @return SQL_SUCCESS or SQL_ERROR
	int (*StmtPrepareStr)(SqlStmt* self, const char* query);



	/// Returns the number of parameters in the prepared statement.
	///
	/// @return Number or paramenters
	size_t (*StmtNumParams)(SqlStmt* self);



	/// Binds a parameter to a buffer.
	/// The buffer data will be used when the statement is executed.
	/// All parameters should have bindings.
	///
	/// @return SQL_SUCCESS or SQL_ERROR
	int (*StmtBindParam)(SqlStmt* self, size_t idx, SqlDataType buffer_type, void* buffer, size_t buffer_len);



	/// Executes the prepared statement.
	/// Any previous result is freed and all column bindings are removed.
	///
	/// @return SQL_SUCCESS or SQL_ERROR
	int (*StmtExecute)(SqlStmt* self);



	/// Returns the number of the AUTO_INCREMENT column of the last INSERT/UPDATE statement.
	///
	/// @return Value of the auto-increment column
	uint64 (*StmtLastInsertId)(SqlStmt* self);



	/// Returns the number of columns in each row of the result.
	///
	/// @return Number of columns
	size_t (*StmtNumColumns)(SqlStmt* self);



	/// Binds the result of a column to a buffer.
	/// The buffer will be filled with data when the next row is fetched.
	/// For string/enum buffer types there has to be enough space for the data
	/// and the nul-terminator (an extra byte).
	///
	/// @return SQL_SUCCESS or SQL_ERROR
	int (*StmtBindColumn)(SqlStmt* self, size_t idx, SqlDataType buffer_type, void* buffer, size_t buffer_len, uint32* out_length, int8* out_is_null);



	/// Returns the number of rows in the result.
	///
	/// @return Number of rows
	uint64 (*StmtNumRows)(SqlStmt* self);



	/// Fetches the next row.
	/// All column bindings will be filled with data.
	///
	/// @return SQL_SUCCESS, SQL_ERROR or SQL_NO_DATA
	int (*StmtNextRow)(SqlStmt* self);



	/// Frees the result of the statement execution.
	void (*StmtFreeResult)(SqlStmt* self);

	/// Frees a SqlStmt returned by SqlStmt_Malloc.
	void (*StmtFree)(SqlStmt* self);

	void (*StmtShowDebug_)(SqlStmt* self, const char* debug_file, const unsigned long debug_line);

} sql_s;

struct sql_interface *SQL;

void sql_defaults(void);

#if defined(SQL_REMOVE_SHOWDEBUG)
#define Sql_ShowDebug(self) (void)0
#else
#define Sql_ShowDebug(self) (SQL->ShowDebug_((self), __FILE__, __LINE__))
#endif

void Sql_HerculesUpdateCheck(Sql* self);
void Sql_HerculesUpdateSkip(Sql* self,const char *filename);

#if defined(SQL_REMOVE_SHOWDEBUG)
#define SqlStmt_ShowDebug(self) (void)0
#else
/// Shows debug information (with statement).
#define SqlStmt_ShowDebug(self) (SQL->StmtShowDebug_((self), __FILE__, __LINE__))
#endif

void Sql_Init(void);

#endif /* _COMMON_SQL_H_ */
