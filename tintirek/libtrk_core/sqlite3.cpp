/*
 *	sqlite3.cpp
 *
 *	Tintirek's SQLite integration
 */


#include <iostream>

#include "sqlite3.h"


TrkSqliteStatement::TrkSqliteStatement(sqlite3_stmt* Statement)
	: statement(Statement)
{ }

TrkSqliteStatement::~TrkSqliteStatement()
{
	if (statement)
	{
		sqlite3_finalize(statement);
	}
}

int TrkSqliteStatement::Step()
{
	return sqlite3_step(statement);
}

void TrkSqliteStatement::Reset()
{
	sqlite3_reset(statement);
}

TrkSqliteStatement::operator sqlite3_stmt* () const
{
	return statement;
}

void TrkSqliteStatement::BindText(int Index, TrkString Text)
{
	sqlite3_bind_text(statement, Index, Text, -1, SQLITE_STATIC);
}

void TrkSqliteStatement::BindInt(int Index, int Value)
{
	sqlite3_bind_int(statement, Index, Value);
}

void TrkSqliteStatement::BindDouble(int Index, double Value)
{
	sqlite3_bind_double(statement, Index, Value);
}

void TrkSqliteStatement::BindNull(int Index)
{
	sqlite3_bind_null(statement, Index);
}

TrkString TrkSqliteStatement::GetText(int Index) const
{
	if (Index >= 0 && Index < sqlite3_column_count(statement)) {
		const char* text = reinterpret_cast<const char*>(sqlite3_column_text(statement, Index));
		if (text) {
			return TrkString(text);
		}
	}
	return TrkString();
}

int TrkSqliteStatement::GetInt(int Index) const
{
	if (Index >= 0 && Index < sqlite3_column_count(statement)) {
		return sqlite3_column_int(statement, Index);
	}
	return 0;
}

double TrkSqliteStatement::GetDouble(int Index) const
{
	if (Index >= 0 && Index < sqlite3_column_count(statement)) {
		return sqlite3_column_double(statement, Index);
	}
	return 0;
}

TrkSqliteValue::TrkSqliteValue(TrkSqliteStatement* Statement, int ColumnIndex)
	: statement(Statement)
	, column_index(ColumnIndex)
{ }

bool TrkSqliteValue::IsValid() const
{
	return statement != nullptr;
}

int TrkSqliteValue::GetType() const
{
	return sqlite3_column_type((sqlite3_stmt*)*statement, column_index);
}

TrkString TrkSqliteValue::GetText() const
{
	return TrkString(reinterpret_cast<const char*>(sqlite3_column_text((sqlite3_stmt*)statement, column_index)));
}

int TrkSqliteValue::GetInt() const
{
	return sqlite3_column_int((sqlite3_stmt*)statement, column_index);
}

double TrkSqliteValue::GetDouble() const
{
	return sqlite3_column_double((sqlite3_stmt*)statement, column_index);
}

TrkSqliteQueryResult::TrkSqliteQueryResult(TrkSqliteStatement* Statement)
	: statement(Statement)
{ }

TrkSqliteQueryResult::~TrkSqliteQueryResult()
{
	if (statement)
	{
		delete statement;
	}
}

bool TrkSqliteQueryResult::Next()
{
	return statement->Step() == SQLITE_ROW;
}

bool TrkSqliteQueryResult::IsValid() const
{
	return statement != nullptr;
}

TrkString TrkSqliteQueryResult::GetString(int ColumnIndex) const
{
	return TrkString(statement->GetText(ColumnIndex));
}

int TrkSqliteQueryResult::GetInt(int ColumnIndex) const
{
	return statement->GetInt(ColumnIndex);
}

double TrkSqliteQueryResult::GetDouble(int ColumnIndex) const
{
	return statement->GetDouble(ColumnIndex);
}

TrkSqliteDB::TrkSqliteDB(TrkString DatabasePath)
	: database(nullptr)
{ 
	int rc = sqlite3_open_v2(DatabasePath, &database, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
	if (rc != SQLITE_OK)
	{
		std::cerr << "Cannot open database: " << sqlite3_errmsg(database) << std::endl;
		database = nullptr;
	}
}

TrkSqliteDB::~TrkSqliteDB()
{
	if (database)
	{
		sqlite3_close(database);
	}
}

TrkSqliteStatement* TrkSqliteDB::PrepareStatement(TrkString Query)
{
	sqlite3_stmt* stmt = nullptr;
	int rc = sqlite3_prepare_v2(database, Query, -1, &stmt, nullptr);
	if (rc != SQLITE_OK)
	{
		std::cerr << "Cannot prepare statement: " << sqlite3_errmsg(database) << std::endl;
		return nullptr;
	}

	return new TrkSqliteStatement(stmt);
}

TrkSqliteValue TrkSqliteDB::ExecuteScalar(TrkString Query)
{
	TrkSqliteStatement* stmt = PrepareStatement(Query);
	if (stmt)
	{
		if (stmt->Step() == SQLITE_ROW)
		{
			return TrkSqliteValue(stmt, 0);
		}
	}

	return TrkSqliteValue(nullptr, -1);
}

int TrkSqliteDB::ExecuteNonQuery(TrkString Query)
{
	TrkSqliteStatement* stmt = PrepareStatement(Query);
	if (stmt)
	{
		return stmt->Step();
	}

	return SQLITE_ERROR;
}

TrkSqliteQueryResult TrkSqliteDB::ExecuteReader(TrkString Query)
{
	TrkSqliteStatement* stmt = PrepareStatement(Query);
	return TrkSqliteQueryResult(stmt);
}

int TrkSqliteDB::ExecuteInsert(TrkString Query)
{
	TrkSqliteStatement* stmt = PrepareStatement(Query);
	if (stmt)
	{
		if (stmt->Step() == SQLITE_DONE)
		{
			return sqlite3_last_insert_rowid(database);
		}
	}

	return -1;
}

int TrkSqliteDB::ExecuteDelete(TrkString Query)
{
	TrkSqliteStatement* stmt = PrepareStatement(Query);
	if (stmt)
	{
		if (stmt->Step() == SQLITE_DONE)
		{
			return sqlite3_changes(database);
		}
	}

	return -1;
}

int TrkSqliteDB::ExecuteUpdate(TrkString Query)
{
	TrkSqliteStatement* stmt = PrepareStatement(Query);
	if (stmt)
	{
		if (stmt->Step() == SQLITE_DONE)
		{
			return sqlite3_changes(database);
		}
	}

	return -1;
}

void TrkSqliteDB::BeginTransaction()
{
	ExecuteNonQuery("BEGIN TRANSACTON");
}

void TrkSqliteDB::RollbackTransaction()
{
	ExecuteNonQuery("ROLLBACK TRANSACTON");
}

void TrkSqliteDB::CommitTransaction()
{
	ExecuteNonQuery("COMMIT");
}
