/*
 *	sqlite3.h
 *
 *	Tintirek's SQLite integration
 */

#ifndef TRK_SQLITE3_H
#define TRK_SQLITE3_H


#define SQLITE_OMIT_DEPRECATED 1
#define SQLITE_DEFAULT_MEMSTATUS 0
#define SQLITE_OMIT_WAL 1

#include "../../deps/sqlite-amalgamation/sqlite3.h"

#include "trk_types.h"


#define BUSY_TIMEOUT 10000
#define EXEC_SQL(db, sql) TrkSqliteWrapper::ExecSql((db), (sql), SQLITE_OK)


class TrkSqliteDb
{
public:
	sqlite3* database;
	const TrkString* statement_strings;
	int number_of_statements;
	class TrKSqliteStatement** prepared_statements;
};

class TrkSqliteStatement
{
public:
	sqlite3_stmt* statement;
	TrkSqliteDb* database;
};

class TrkSqliteContext
{
public:
	sqlite3_context* context;
};

class TrkSqliteValue
{
public:
	sqlite3_value* value;
};


class TrkSqliteWrapper
{
public:
	static TrkString GetCompiledVersion() const;
	static TrkString GetRuntimeVersion() const;

	static bool ExecSql(TrkSqliteDb* Database, TrkString* SQL, int IgnoredError);
	static bool PrepareStatement(TrkSqliteStatement** Statements, TrkSqliteDb* Database, TrkString Text);
	static bool ExecStatements(TrkSqliteDb* Statements, int StatementIndex);
	static bool GetStatement(TrkSqliteStatement** Statements, TrkSqliteDb* Database, int StatementIndex);
	static bool StepWithExpectation(TrkSqliteStatement* Statement, bool ExpectingRow);
	static bool StepDone(TrkSqliteStatement* Statement);
	static bool StepRow(TrkSqliteStatement* Statement);
	static bool Step(TrkSqliteStatement* Statement, bool& GotRow);
	static bool Insert(int64_t& RowID, TrkSqliteStatement* Statement);
	static bool Update(int AffectedRows, TrkSqliteStatement* Statement);
	static bool Bind(TrkSqliteStatement* Statement, TrkString Format);
	static bool BindInt(TrkSqliteStatement* Statement, int Slot, int Value);
	static bool BindInt64(TrkSqliteStatement* Statement, int Slot, int64_t Value);
	static bool BindString(TrkSqliteStatement* Statement, int Slot, TrkString Value);
	static bool BindRevNum(TrkSqliteStatement* Statement, int Slot, trk_revision_number_t Value);
	static bool ColumnInt(TrkSqliteStatement* Statement, int Column);
	static bool ColumnInt64(TrkSqliteStatement* Statement, int Column);
	static bool ColumnString(TrkSqliteStatement* Statement, int Column);
	static bool ColumnRevNum(TrkSqliteStatement* Statement, int Column);
	static bool ColumnBoolean(TrkSqliteStatement* Statement, int Column);
	static bool ColumnIsNull(TrkSqliteStatement* Statement, int Column);
	static int ColumnBytes(TrkSqliteStatement* Statement, int Column);
	static bool Finalize(TrkSqliteStatement* Statement);
	static bool Reset(TrkSqliteStatement* Statement);
	/*
	 *
	 *	func sqlite_compiled_version
	 *	func sqlite_runtime_version
	 *	func sqlite_tracer
	 *	func sqlite_profiler
	 *	func sqlite_error_log
	 *	func sqlite_dbg_enable_error
	 *	struct sqlite_db_t
	 *	struct sqlite_stmt_t
	 *	struct sqlite_context_t
	 *	struct sqlite_value_t
	 *	#define BUSY_TIMEOUT
	 *	func exec_sql
	 *	func prepare_statement
	 *	func sqlite_exec_statements
	 *	func sqlite_get_statements
	 *	func get_internal_statement
	 *	func step_with_expectation
	 *	func sqlite_step_done
	 *	func sqlite_step_row
	 *	func sqlite_step
	 *	func sqlite_insert
	 *	func sqlite_update
	 *	func vbindf
	 *	func sqlite_bind_text
	 *	func sqlite_bindf
	 *	func sqlite_bind_int
	 *	func sqlite_bind_int64
	 *	func sqlite_bind_text
	 *	func sqlite_bind_blob
	 *	func sqlite_bind_token
	 *	func sqlite_bind_revnum
	 *	func sqlite_bind_properties
	 *	func sqlite_bind_iprops
	 *	func sqlite_bind_checksum
	 *	func sqlite_column_blob
	 *	func sqlite_column_text
	 *	func sqlite_column_revnum
	 *	func sqlite_column_int
	 *	func sqlite_column_int64
	 *	func sqlite_column_token_null
	 *	func sqlite_column_properties
	 *	func sqlite_column_iprops
	 *	func sqlite_column_cheksum
	 *	func sqlite_column_is_null
	 *	func sqlite_column_bytes
	 *	func sqlite_finalize
	 *	func sqlite_reset
	 *	func sqlite_read_schema_version
	 *	func init_sqlite
	 *	func internal_open
	 *	func sqlite_open
	 *	func sqlite_close
	 *	func reset_all_Statements
	 *	func rollback_transaction
	 *	func sqlite_begin_transaction
	 *	func sqlite_begin_immediate_transaction
	 *	func sqlite_begin_savepoint
	 *	func sqlite_finish_transaction
	 *	func sqlite_finish_savepoint
	 *	func sqlite_with_transaction
	 *	func sqlite_with_immediate_transaction
	 *	func sqlite_with_lock
	 *	func sqlite_hotcopy
	 *	func wrapped_func
	 *	func sqlite_create_scalar_function
	 *	func sqlite_value_type
	 *	func sqlite_value_text
	 *	func sqlite_value_null
	 *	func sqlite_value_int64
	 *	func sqlite_value_error
	 */
};


#endif /* TRK_SQLITE3_H */