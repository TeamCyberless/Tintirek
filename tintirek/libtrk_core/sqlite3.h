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


class TrkSqliteStatement
{
public:
	TrkSqliteStatement(sqlite3_stmt* Statement);
	~TrkSqliteStatement();

	int Step();
	void Reset();

	operator sqlite3_stmt* () const;

	void BindText(int Index, TrkString Text);
	void BindInt(int Index, int Value);
	void BindDouble(int Index, double Value);
	void BindNull(int Index);

	TrkString GetText(int Index) const;
	int GetInt(int Index) const;
	double GetDouble(int Index) const;

private:
	sqlite3_stmt* statement;
};

class TrkSqliteValue
{
public:
	TrkSqliteValue(TrkSqliteStatement* Statement, int ColumnIndex);

	int GetType() const;

	TrkString GetText() const;
	int GetInt() const;
	double GetDouble() const;

private:
	TrkSqliteStatement* statement;
	int column_index;
};

class TrkSqliteQueryResult
{
public:
	TrkSqliteQueryResult(TrkSqliteStatement* Statement);
	~TrkSqliteQueryResult();

	bool Next();

	TrkString GetString(int ColumnIndex) const;
	int GetInt(int ColumnIndex) const;
	double GetDouble(int ColumnIndex) const;

private:
	TrkSqliteStatement* statement;
};

class TrkSqliteDB
{
public:
	TrkSqliteDB(TrkString DatabasePath);
	~TrkSqliteDB();

	TrkSqliteStatement* PrepareStatement(TrkString Query);

	TrkSqliteValue ExecuteScalar(TrkString Query);
	int ExecuteNonQuery(TrkString Query);
	TrkSqliteQueryResult ExecuteReader(TrkString Query);
	int ExecuteInsert(TrkString Query);
	int ExecuteDelete(TrkString Query);
	int ExecuteUpdate(TrkString Query);

	void BeginTransaction();
	void RollbackTransaction();
	void CommitTransaction();

private:
	sqlite3* database;
};


#endif /* TRK_SQLITE3_H */