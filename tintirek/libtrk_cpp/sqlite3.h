/*
 *	sqlite3.h
 *
 *	Tintirek's SQLite integration
 */

#ifndef TRK_SQLITE3_H
#define TRK_SQLITE3_H

#include "trk_types.h"
#include <memory>
#include <map>
#include <stdexcept>

/* Forward declarations to avoid inclusion of sqlite3 in this header */
struct sqlite3;
struct sqlite3_stmt;
struct sqlite3_context;
struct sqlite3_value;


namespace TrkSqlite
{
	/// There's some annoying LNK problems. This should be fix but not be professional
	const int OPEN_READONLY = 0x00000001;
	const int OPEN_READWRITE = 0x00000002;
	const int OPEN_CREATE = 0x00000004;
	const int OPEN_URI = 0x00000040;
	const int OPEN_MEMORY = 0x00000080;
	const int OPEN_NOMUTEX = 0x00008000;
	const int OPEN_FULLMUTEX = 0x00010000;
	const int OPEN_SHAREDCACHE = 0x00020000;
	const int OPEN_NOFOLLOW = 0x01000000;
	const int OK = 0;

	const int ROW = 100;
	const int DONE = 101;

	const int CODE_ERROR = 1;
	const int CODE_CONSTRAINT = 19;
	const int CODE_CONSTRAINT_PRIMARYKEY = (CODE_CONSTRAINT | (6 << 8));
	const int CODE_RANGE = 25;

	const int INTEGER = 1;
	const int FLOAT = 2;
	const int TEXT = 3;
	const int BLOB = 4;
	const int Null = 5;

	/* Returns SQLite version string */
	std::string GetLibVersion();
	/* Returns SQLite version string */
	int GetLibVersionNumber();


	/* Exception class for error messages generated from SQLite3 */
	class TrkDatabaseException : public std::runtime_error
	{
	public:
		TrkDatabaseException(const std::string ErrorMessage, int ReturnValue);
		TrkDatabaseException(sqlite3* Database, int ReturnValue);
		explicit TrkDatabaseException(const std::string ErrorMessage) : TrkDatabaseException(ErrorMessage, -1) {}
		explicit TrkDatabaseException(sqlite3* Database);

		/* Returns the error code (if there's any, otherwise will return -1) */
		int GetErrorCode() const { return errcode; }
		/* Returns the extended error code (if there's any, otherwise will return -1) */
		int GetExtendedErrorCode() const { return errextndcode; }

	public:
		/* Error code value */
		int errcode;
		/* Extended error code value */
		int errextndcode;
	};

	/* Tintirek's Database Class */
	class TrkDatabase
	{
		friend class TrkStatement;

	public:
		/* Opens the database from provided filename */
		TrkDatabase(const std::string Filename, const int Flags = TrkSqlite::OPEN_READONLY);

		/* Disables copy and move */
		TrkDatabase(const TrkDatabase&) = delete;
		TrkDatabase& operator=(const TrkDatabase&) = delete;
		TrkDatabase(TrkDatabase&& Database) = delete;
		TrkDatabase& operator=(TrkDatabase&& Database) = delete;

		/* Deleter functor to use with smart pointers to close the SQLite database connection */
		struct Deleter
		{
			void operator()(sqlite3* SQLite);
		};

		
		/* Shortcut to execute statements without results. Returns the number of changes */
		int Execute(std::string Queries);
		/* Try to execute statements, returning the sqlite result code */
		int TryExecute(std::string Queries);


		/* Returns true if a table exists */
		bool TableExists(std::string TableName) const;
		/* Get the row ID of the most recent successful INSERT into the database from the current connection */
		int64_t GetLastInsertRowID() const;
		/* Get number of rows. Modified by last INSERT, UPDATE or DELETE statement (except DROP) */
		int GetChanges() const;
		/* Get total number of rows. Modified by all INSERT, UPDATE or DELETE statement since connection (except DROP) */
		int GetTotalChanges() const;
		/* Get the numeric result of error code (if any) */
		int GetErrorCode() const;
		/* Get the extended numeric result of error code (if any) */
		int GetExtendedErrorCode() const;

	private:
		/* Pointer to SQLite database connection */
		std::unique_ptr<sqlite3, Deleter> sqlite_db;
	};

	/* Tintirek's Database Statement Class */
	class TrkStatement
	{
	public:
		TrkStatement(const TrkDatabase& Database, std::string Query);
		~TrkStatement() = default;

		/* Disables copy and move */
		TrkStatement(const TrkStatement&) = delete;
		TrkStatement& operator=(const TrkStatement&) = delete;
		TrkStatement(TrkStatement&& Database) = delete;
		TrkStatement& operator=(TrkStatement&& Database) = delete;

		/* Execute a step of the prepared query to fetch one row of results */
		bool ExecuteStep();
		/* Try to execute a step of the prepared query to fetch one row of results */
		int TryExecuteStep();
		/* Execute a one-step query with no expected result and return the number of changes */
		int Execute();
		/* Reset the statement to make it ready for a new execution */
		void Reset();
		/* Try reset the statement. Returns the result code */
		int TryReset();

		/* Clears away all the bindings of a prepared statement */
		void ClearBindings();
		/* Returns index from given name */
		int GetIndex(const std::string Name) const;
		/* Get number of rows modified by last INSERT, UPDATE or DELETE (except DROP) */
		int GetChanges() const;
		/* True when a row has been fetched with ExecuteStep, false otherwise */
		bool HasRow() const;
		/* True when the last ExecuteStep had no more row to fetch */
		bool Done() const;
		/* Returns the SQL query */
		const std::string GetQuery() const;
		/* Returns a string containing the SQL query of prepared statement with bound parameters expanded */
		const std::string GetExpandedQuery() const;

		/* Bind an 32 bits signed int value to a parameter */
		void Bind(const int Index, const int32_t Value);
		/* Bind a 32 bits unsigned int value to a parameter */
		void Bind(const int Index, const uint32_t Value);
		/* Bind a 64 bits signed int value to a parameter */
		void Bind(const int Index, const int64_t Value);
		/* Bind a 64 bits float value to a parameter */
		void Bind(const int Index, const double Value);
		/* Bind a string value to a parameter */
		void Bind(const int Index, const std::string Value);
		/* Bind a binary blob value to a parameter */
		void Bind(const int Index, const void* Value, const int Size);
		/* Bind a null value to a parameter */
		void Bind(const int Index);

		/* Bind an 32 bits signed int value to a named parameter */
		void Bind(const std::string Name, const int32_t Value) { Bind(GetIndex(Name), Value); }
		/* Bind a 32 bits unsigned int value to a named parameter */
		void Bind(const std::string Name, const uint32_t Value) { Bind(GetIndex(Name), Value); }
		/* Bind a 64 bits signed int value to a named parameter */
		void Bind(const std::string Name, const int64_t Value) { Bind(GetIndex(Name), Value); }
		/* Bind a 64 bits float value to a named parameter */
		void Bind(const std::string Name, const double Value) { Bind(GetIndex(Name), Value); }
		/* Bind a string value to a named parameter */
		void Bind(const std::string Name, const std::string Value) { Bind(GetIndex(Name), Value); }
		/* Bind a binary blob value to a named parameter */
		void Bind(const std::string Name, const void* Value, const int Size) { Bind(GetIndex(Name), Value, Size); }
		/* Bind a null value to a named parameter */
		void Bind(const std::string Name) { Bind(GetIndex(Name)); }

		/* Return a copy of the column data specified by its index */
		class TrkValue GetColumn(const int Index) const;
		/* Return a copy of the column data specified by its column name */
		class TrkValue GetColumn(const std::string Name) const;
		/* Returns name of the specified result column */
		std::string GetColumnName(const int Index) const;
		/* Checks if the column value is NULL */
		bool IsColumnNull(const int Index) const;
		/* Checks if the column value is NULL */
		bool IsColumnNull(const std::string Name) const;
		/* Returns the number of columns in the result set */
		int GetColumnCount() const;
		/* Returns the index of specified column name */
		int GetColumnIndex(const std::string Name) const;

		/* Get the numeric result of error code (if any) */
		int GetErrorCode() const;
		/* Get the extended numeric result of error code (if any) */
		int GetExtendedErrorCode() const;
		
		using TrkSharedStatementPtr = std::shared_ptr<sqlite3_stmt>;

	private:
		/* Check if there is a row of result returned by ExecuteStep() */
		void CheckRow() const;
		/* Check if there is a value index is in ther ange of columns in the result */
		void CheckIndex(const int Index) const;

		/* Prepare a statement object */
		TrkSharedStatementPtr PrepareStatement();
		/* Returns a prepared statement object */
		sqlite3_stmt* GetPreparedStatement() const;


		std::string query;								// SQL query
		sqlite3* sqlite_db;								// Pointer to SQLite database connection handle
		TrkSharedStatementPtr prepared_statement;		// Shared pointer to the prepared SQLite statement object
		int column_count = 0;							// Number of columns in the result of the prepared statement
		bool has_row = false;							// True when a row has been fetched with ExecuteStep()
		bool done = false;								// True when the last ExecuteStep() had no more row to fetch
		mutable std::map<std::string, int> column_names;	// Map of columns index by name
	};

	/* Tintirek's Value Class */
	class TrkValue
	{
	public:
		TrkValue(const TrkStatement::TrkSharedStatementPtr& StatementPtr, int Index);


		/* Returns name of this result */
		const std::string GetName() const;
		/* Return the type of the value */
		int GetType() const;

		/* Return the 32 bits signed integer value of the column */
		int32_t GetInt() const;
		/* Return the 32 bits unsigned integer value of the column */
		uint32_t GetUInt() const;
		/* Return the 64 bits integer value of the column */
		int64_t GetInt64() const;
		/* Return the 64 bits float value of the column */
		double GetDouble() const;
		/* Return the string value of the column */
		std::string GetString() const;
		/* Return the binary blob value of the column */
		const void* GetBlob() const;

		/* Check if the value is an integer type value */
		bool IsInt() const { return TrkSqlite::INTEGER == GetType(); }
		/* Check if the value is an integer type value */
		bool IsFloat() const { return TrkSqlite::FLOAT == GetType(); }
		/* Check if the value is an integer type value */
		bool IsString() const { return TrkSqlite::TEXT == GetType(); }
		/* Check if the value is an integer type value */
		bool IsBlob() const { return TrkSqlite::BLOB == GetType(); }
		/* Check if the value is an integer type value */
		bool IsNull() const { return TrkSqlite::Null == GetType(); }

		/* Return the number of bytes used by the text (or blob) value of this */
		int GetBytes() const;


		/*
		 *	Inline casting operators
		 */
		operator char() const
		{
			return static_cast<char>(GetInt());
		}
		operator int8_t() const
		{
			return static_cast<int8_t>(GetInt());
		}
		operator uint8_t() const
		{
			return static_cast<uint8_t>(GetInt());
		}
		operator int16_t() const
		{
			return static_cast<int16_t>(GetInt());
		}
		operator uint16_t() const
		{
			return static_cast<uint16_t>(GetInt());
		}
		operator int32_t() const
		{
			return GetInt();
		}
		operator uint32_t() const
		{
			return GetUInt();
		}
		operator int64_t() const
		{
			return GetInt64();
		}
		operator double() const
		{
			return GetDouble();
		}
		operator std::string() const
		{
			return GetString();
		}
		operator const void* () const
		{
			return GetBlob();
		}

	private:
		/* Shared pointer to the prepared SQLite statement object */
		TrkStatement::TrkSharedStatementPtr statement_ptr;
		/* Index of the column in the row of result */
		int index;
	};

	/* Tintirek's Database Transaction Class */
	class TrkTransaction
	{
	public:
		TrkTransaction(TrkDatabase& Database);
		~TrkTransaction();

		/* Disables copy */
		TrkTransaction(const TrkTransaction&) = delete;
		TrkTransaction& operator=(const TrkTransaction&) = delete;

		/* Commit the transaction */
		void Commit();
		/* Rollback the transaction */
		void Rollback();

	private:
		TrkDatabase& database;	// Reference to the SQLite database connection
		bool Commited;			// True when commit has been called
	};
}

#endif /* TRK_SQLITE3_H */