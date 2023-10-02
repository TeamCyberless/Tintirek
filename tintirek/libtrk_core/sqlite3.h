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


/* Represents an SQLite statement and provides methods to execute and retrieve results */
class TrkSqliteStatement
{
public:
	TrkSqliteStatement(sqlite3_stmt* Statement);
	~TrkSqliteStatement();

	/* Execute the statement */
	int Step();
	/* Reset the statement for re-execution */
	void Reset();

	/* Implicit conversion operator to retrieve the underlying SQLite statement */
	operator sqlite3_stmt* () const;

	/* Bind a text value to the specified parameter index */
	void BindText(int Index, TrkString Text);
	/* Bind an integer value to the specified parameter index */
	void BindInt(int Index, int Value);
	/* Bind a double value to the specified parameter index */
	void BindDouble(int Index, double Value);
	/* Bind a null value to the specified parameter index */
	void BindNull(int Index);

	/* Get the text value from the specified result column */
	TrkString GetText(int Index) const;
	/* Get the integer value from the specified result column */
	int GetInt(int Index) const;
	/* Get the double value from the specified result column */
	double GetDouble(int Index) const;

private:
	sqlite3_stmt* statement;
};


/* Represents a single value from an SQLite query result */
class TrkSqliteValue
{
public:
	TrkSqliteValue(TrkSqliteStatement* Statement, int ColumnIndex);

	/* Get the data type of the value */
	int GetType() const;
	/* Get the text representation of the value */
	TrkString GetText() const;
	/* Get the integer value */
	int GetInt() const;
	/* Get the double value */
	double GetDouble() const;

private:
	TrkSqliteStatement* statement;
	int column_index;
};


/* Represents the result of an SQLite query */
class TrkSqliteQueryResult
{
public:
	TrkSqliteQueryResult(TrkSqliteStatement* Statement);
	~TrkSqliteQueryResult();

	/* Move to the next row in the result set */
	bool Next();

	/* Get the string value from the specified column in the current row */
	TrkString GetString(int ColumnIndex) const;
	/* Get the integer value from the specified column in the current row */
	int GetInt(int ColumnIndex) const;
	/* Get the double value from the specified column in the current row */
	double GetDouble(int ColumnIndex) const;

private:
	TrkSqliteStatement* statement;
};


/* Represents an SQLite database and provides methods for database operations */
class TrkSqliteDB
{
public:
	TrkSqliteDB(TrkString DatabasePath);
	~TrkSqliteDB();

	/* Prepare an SQL statement for execution */
	TrkSqliteStatement* PrepareStatement(TrkString Query);

	/* Execute an SQL query and return a single value */
	TrkSqliteValue ExecuteScalar(TrkString Query);

	/* Execute a non-query SQL statement (e.g., INSERT, DELETE) and return the affected row count */
	int ExecuteNonQuery(TrkString Query);
	/* Execute an SQL query and return a query result */
	TrkSqliteQueryResult ExecuteReader(TrkString Query);
	/* Execute an INSERT statement and return the last inserted row ID */
	int ExecuteInsert(TrkString Query);
	/* Execute a DELETE statement and return the affected row count */
	int ExecuteDelete(TrkString Query);
	/* Execute an UPDATE statement and return the affected row count */
	int ExecuteUpdate(TrkString Query);

	/* Begin an SQLite transaction */
	void BeginTransaction();
	/* Rollback an SQLite transaction */
	void RollbackTransaction();
	/* Commit an SQLite transaction */
	void CommitTransaction();

private:
	sqlite3* database;
};


#endif /* TRK_SQLITE3_H */