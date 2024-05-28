/*
 *	sqlite3.cpp
 *
 *	Tintirek's SQLite integration
 */


#include <iostream>
#include <string.h>
#include "sqlite3.h"
#include <../../deps/sqlite-amalgamation/sqlite3.h>

std::string TrkSqlite::GetLibVersion()
{
	return std::string(sqlite3_libversion());
}

int TrkSqlite::GetLibVersionNumber()
{
	return sqlite3_libversion_number();
}

TrkSqlite::TrkDatabaseException::TrkDatabaseException(const std::string ErrorMessage, int ReturnValue)
	: std::runtime_error(ErrorMessage)
	, errcode(ReturnValue)
	, errextndcode(-1)
{ }

TrkSqlite::TrkDatabaseException::TrkDatabaseException(struct sqlite3* Database, int ReturnValue)
	: std::runtime_error(sqlite3_errmsg(Database))
	, errcode(ReturnValue)
	, errextndcode(sqlite3_extended_errcode(Database))
{ }

TrkSqlite::TrkDatabaseException::TrkDatabaseException(struct sqlite3* Database)
	: std::runtime_error(sqlite3_errmsg(Database))
	, errcode(sqlite3_errcode(Database))
	, errextndcode(sqlite3_extended_errcode(Database))
{ }

TrkSqlite::TrkDatabase::TrkDatabase(const std::string Filename, const int Flags)
{
	sqlite3* handle;
	const int ret = sqlite3_open_v2(Filename.c_str(), &handle, Flags, nullptr);
	sqlite_db.reset(handle);
	if (SQLITE_OK != ret)
	{
		throw TrkSqlite::TrkDatabaseException(handle, ret);
	}
}

void TrkSqlite::TrkDatabase::Deleter::operator()(sqlite3* SQLite)
{
	const int ret = sqlite3_close(SQLite);

	// Avoid unreferenced variable warning when build in release mode
	(void)ret;

	if (TrkSqlite::OK != ret)
	{
		throw TrkSqlite::TrkDatabaseException("Database is locked", ret);
	}
}

int TrkSqlite::TrkDatabase::Execute(std::string Queries)
{
	const int ret = TryExecute(Queries);

	if (TrkSqlite::OK != ret)
	{
		throw TrkSqlite::TrkDatabaseException(sqlite_db.get(), ret);
	}

	return sqlite3_changes(sqlite_db.get());
}

int TrkSqlite::TrkDatabase::TryExecute(std::string Queries)
{
	return sqlite3_exec(sqlite_db.get(), Queries.c_str(), nullptr, nullptr, nullptr);
}

bool TrkSqlite::TrkDatabase::TableExists(std::string TableName) const
{
	TrkStatement query(*this, "SELECT count(*) FROM sqlite_master WHERE type='table' AND name=?");
	query.Bind(1, TableName);
	(void)query.ExecuteStep();
	return query.GetColumn(0).GetInt() == 1;
}

int64_t TrkSqlite::TrkDatabase::GetLastInsertRowID() const
{
	return sqlite3_last_insert_rowid(sqlite_db.get());
}

int TrkSqlite::TrkDatabase::GetChanges() const
{
	return sqlite3_changes(sqlite_db.get());
}

int TrkSqlite::TrkDatabase::GetTotalChanges() const
{
	return sqlite3_total_changes(sqlite_db.get());
}

int TrkSqlite::TrkDatabase::GetErrorCode() const
{
	return sqlite3_errcode(sqlite_db.get());
}

int TrkSqlite::TrkDatabase::GetExtendedErrorCode() const
{
	return sqlite3_extended_errcode(sqlite_db.get());
}

TrkSqlite::TrkValue::TrkValue(const TrkStatement::TrkSharedStatementPtr& StatementPtr, int Index)
	: statement_ptr(StatementPtr)
	, index(Index)
{
	if (!StatementPtr)
	{
		throw TrkDatabaseException("Statement was destroyed");
	}
}

const std::string TrkSqlite::TrkValue::GetName() const
{
	return sqlite3_column_name(statement_ptr.get(), index);
}

int TrkSqlite::TrkValue::GetType() const
{
	return sqlite3_column_type(statement_ptr.get(), index);
}

int32_t TrkSqlite::TrkValue::GetInt() const
{
	return sqlite3_column_int(statement_ptr.get(), index);
}

uint32_t TrkSqlite::TrkValue::GetUInt() const
{
	return static_cast<unsigned>(GetInt64());
}

int64_t TrkSqlite::TrkValue::GetInt64() const
{
	return sqlite3_column_int64(statement_ptr.get(), index);
}

double TrkSqlite::TrkValue::GetDouble() const
{
	return sqlite3_column_double(statement_ptr.get(), index);
}

std::string TrkSqlite::TrkValue::GetString() const
{
	auto Text = reinterpret_cast<const char*>(sqlite3_column_text(statement_ptr.get(), index));
	return Text ? std::string(Text) : std::string("");
}

const void* TrkSqlite::TrkValue::GetBlob() const
{
	return sqlite3_column_blob(statement_ptr.get(), index);
}

int TrkSqlite::TrkValue::GetBytes() const
{
	return sqlite3_column_bytes(statement_ptr.get(), index);
}

TrkSqlite::TrkStatement::TrkStatement(const TrkDatabase& Database, std::string Query)
	: query(Query)
	, sqlite_db(Database.sqlite_db.get())
	, prepared_statement(PrepareStatement())
{
	column_count = sqlite3_column_count(prepared_statement.get());
}

bool TrkSqlite::TrkStatement::ExecuteStep()
{
	const int ret = TryExecuteStep();
	if (SQLITE_ROW != ret && SQLITE_DONE != ret)
	{
		if (ret == sqlite3_errcode(sqlite_db))
		{
			throw TrkDatabaseException(sqlite_db, ret);
		}
		else
		{
			throw TrkDatabaseException("Statement needs to be reseted", ret);
		}
	}

	return has_row;
}

int TrkSqlite::TrkStatement::TryExecuteStep()
{
	if (done)
	{
		// Statement needs to be reseted
		return SQLITE_MISUSE;
	}

	const int ret = sqlite3_step(prepared_statement.get());
	if (SQLITE_ROW == ret)
	{
		has_row = true;
	}
	else
	{
		has_row = false;
		done = SQLITE_DONE == ret;
	}

	return ret;
}

int TrkSqlite::TrkStatement::Execute()
{
	const int ret = TryExecuteStep();
	if (SQLITE_DONE != ret)
	{
		if (SQLITE_ROW == ret)
		{
			throw TrkDatabaseException("Execute() does not expect results. Use ExecuteStep().");
		}
		else if (ret == sqlite3_errcode(sqlite_db))
		{
			throw TrkDatabaseException(sqlite_db, ret);
		}
		else
		{
			throw TrkDatabaseException("Statement needs to be reseted", ret);
		}
	}

	return sqlite3_changes(sqlite_db);
}

void TrkSqlite::TrkStatement::Reset()
{
	const int ret = TryReset();

	if (SQLITE_OK != ret)
	{
		throw TrkDatabaseException(sqlite_db, ret);
	}
}

int TrkSqlite::TrkStatement::TryReset()
{
	has_row = false;
	done = false;
	return sqlite3_reset(prepared_statement.get());
}

void TrkSqlite::TrkStatement::ClearBindings()
{
	const int ret = sqlite3_clear_bindings(GetPreparedStatement());
	
	if (SQLITE_OK != ret)
	{
		throw TrkDatabaseException(sqlite_db, ret);
	}
}

int TrkSqlite::TrkStatement::GetIndex(const std::string Name) const
{
	return sqlite3_bind_parameter_index(GetPreparedStatement(), Name.c_str());
}

int TrkSqlite::TrkStatement::GetChanges() const
{
	return sqlite3_changes(sqlite_db);
}

bool TrkSqlite::TrkStatement::HasRow() const
{
	return has_row;
}

bool TrkSqlite::TrkStatement::Done() const
{
	return done;
}

const std::string TrkSqlite::TrkStatement::GetQuery() const
{
	return query;
}

const std::string TrkSqlite::TrkStatement::GetExpandedQuery() const
{
	auto expanded = sqlite3_expanded_sql(GetPreparedStatement());
	std::string expandedStr(expanded);
	sqlite3_free(expanded);
	return expandedStr;
}

void TrkSqlite::TrkStatement::Bind(const int Index, const int32_t Value)
{
	const int ret = sqlite3_bind_int(GetPreparedStatement(), Index, Value);

	if (SQLITE_OK != ret)
	{
		throw TrkDatabaseException(sqlite_db, ret);
	}
}

void TrkSqlite::TrkStatement::Bind(const int Index, const uint32_t Value)
{
	const int ret = sqlite3_bind_int64(GetPreparedStatement(), Index, Value);

	if (SQLITE_OK != ret)
	{
		throw TrkDatabaseException(sqlite_db, ret);
	}
}

void TrkSqlite::TrkStatement::Bind(const int Index, const int64_t Value)
{
	const int ret = sqlite3_bind_int64(GetPreparedStatement(), Index, Value);

	if (SQLITE_OK != ret)
	{
		throw TrkDatabaseException(sqlite_db, ret);
	}
}

void TrkSqlite::TrkStatement::Bind(const int Index, const double Value)
{
	const int ret = sqlite3_bind_double(GetPreparedStatement(), Index, Value);

	if (SQLITE_OK != ret)
	{
		throw TrkDatabaseException(sqlite_db, ret);
	}
}

void TrkSqlite::TrkStatement::Bind(const int Index, const std::string Value)
{
	const int ret = sqlite3_bind_text(GetPreparedStatement(), Index, Value.c_str(), Value.size(), SQLITE_TRANSIENT);

	if (SQLITE_OK != ret)
	{
		throw TrkDatabaseException(sqlite_db, ret);
	}
}

void TrkSqlite::TrkStatement::Bind(const int Index, const void* Value, const int Size)
{
	const int ret = sqlite3_bind_blob(GetPreparedStatement(), Index, Value, Size, SQLITE_TRANSIENT);

	if (SQLITE_OK != ret)
	{
		throw TrkDatabaseException(sqlite_db, ret);
	}
}

void TrkSqlite::TrkStatement::Bind(const int Index)
{
	const int ret = sqlite3_bind_null(GetPreparedStatement(), Index);

	if (SQLITE_OK != ret)
	{
		throw TrkDatabaseException(sqlite_db, ret);
	}
}

TrkSqlite::TrkValue TrkSqlite::TrkStatement::GetColumn(const int Index) const
{
	CheckRow();
	CheckIndex(Index);
	return TrkValue(prepared_statement, Index);
}

TrkSqlite::TrkValue TrkSqlite::TrkStatement::GetColumn(const std::string Name) const
{
	CheckRow();
	const int index = GetColumnIndex(Name);
	return TrkValue(prepared_statement, index);
}

std::string TrkSqlite::TrkStatement::GetColumnName(const int Index) const
{
	CheckIndex(Index);
	return sqlite3_column_name(GetPreparedStatement(), Index);
}

bool TrkSqlite::TrkStatement::IsColumnNull(const int Index) const
{
	CheckRow();
	CheckIndex(Index);
	return SQLITE_NULL == sqlite3_column_type(GetPreparedStatement(), Index);
}

bool TrkSqlite::TrkStatement::IsColumnNull(const std::string Name) const
{
	CheckRow();
	const int index = GetColumnIndex(Name);
	return SQLITE_NULL == sqlite3_column_type(GetPreparedStatement(), index);
}

int TrkSqlite::TrkStatement::GetColumnCount() const
{
	return column_count;
}

int TrkSqlite::TrkStatement::GetColumnIndex(const std::string Name) const
{
	if (column_names.empty())
	{
		for (int i = 0; i < column_count; i++)
		{
			const std::string ColumnName = sqlite3_column_name(GetPreparedStatement(), i);
			column_names[ColumnName] = i;
		}
	}

	const auto Elem = column_names.find(Name);
	if (Elem == column_names.end())
	{
		throw TrkDatabaseException("Unknown column name.");
	}

	return Elem->second;
}

int TrkSqlite::TrkStatement::GetErrorCode() const
{
	return sqlite3_errcode(sqlite_db);
}

int TrkSqlite::TrkStatement::GetExtendedErrorCode() const
{
	return sqlite3_extended_errcode(sqlite_db);
}

void TrkSqlite::TrkStatement::CheckRow() const
{
	if (!has_row)
	{
		throw TrkSqlite::TrkDatabaseException("No row to get a column from. ExecuteStep() was not called, or returned false.");
	}
}

void TrkSqlite::TrkStatement::CheckIndex(const int Index) const
{
	if (Index < 0 || Index >= column_count)
	{
		throw TrkSqlite::TrkDatabaseException("Column index out of range.");
	}
}

TrkSqlite::TrkStatement::TrkSharedStatementPtr TrkSqlite::TrkStatement::PrepareStatement()
{
	sqlite3_stmt* statement;
	const int ret = sqlite3_prepare_v2(sqlite_db, query.c_str(), query.size(), &statement, nullptr);
	if (SQLITE_OK != ret)
	{
		throw TrkDatabaseException(sqlite_db, ret);
	}
	return TrkStatement::TrkSharedStatementPtr(statement, [](sqlite3_stmt* stmt)
		{
			sqlite3_finalize(stmt);
		});
}

sqlite3_stmt* TrkSqlite::TrkStatement::GetPreparedStatement() const
{
	sqlite3_stmt* ret = prepared_statement.get();
	if (ret)
	{
		return ret;
	}
	throw TrkDatabaseException("Statement was not prepared.");
}

TrkSqlite::TrkTransaction::TrkTransaction(TrkDatabase& Database)
	: database(Database)
	, Commited(false)
{
	database.Execute("BEGIN TRANSACTION");
}


TrkSqlite::TrkTransaction::~TrkTransaction()
{
	if (!Commited)
	{
		try
		{
			database.Execute("ROLLBACK TRANSACTION");
		}
		catch (TrkDatabaseException&)
		{
			// Never throw an exception in a destructor
		}
	}
}

void TrkSqlite::TrkTransaction::Commit()
{
	if (!Commited)
	{
		database.Execute("COMMIT TRANSACTION");
		Commited = true;
	}
	else
	{
		throw TrkDatabaseException("Transaction already committed.");
	}
}

void TrkSqlite::TrkTransaction::Rollback()
{
	if (!Commited)
	{
		database.Execute("ROLLBACK TRANSACTION");
		Commited = true;
	}
	else
	{
		throw TrkDatabaseException("Transaction already committed.");
	}
}