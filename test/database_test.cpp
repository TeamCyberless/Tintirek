/*
 *	database_test.cpp
 */

#include <sqlite3.h>
#include <cstdio>
#include <fstream>
#include <filesystem>
#include <gtest/gtest.h>
#include "memory_leak.h"

using namespace TrkSqlite;
namespace fs = std::filesystem;

namespace TrkCpp
{

	/*
	 *
	 *	TrkDatabaseException Tests
	 * 
	 */


	TEST(Exception, Copy)
	{
		MemoryLeakDetector leakDetector;

		const TrkDatabaseException ex1("error message", 1);
		const TrkDatabaseException ex2 = ex1;

		EXPECT_STREQ(ex2.what(), ex1.what());
		EXPECT_EQ(ex2.GetErrorCode(), ex1.GetErrorCode());
		EXPECT_EQ(ex2.GetExtendedErrorCode(), ex1.GetExtendedErrorCode());
	}

	TEST(Exception, Assignment)
	{
		MemoryLeakDetector leakDetector;

		TrkString message = "error message";
		const TrkDatabaseException ex1(message, 1);
		TrkDatabaseException ex2("another error message", 2);

		ex2 = ex1;

		EXPECT_STREQ(ex2.what(), message);
		EXPECT_EQ(ex2.GetErrorCode(), 1);
		EXPECT_EQ(ex2.GetExtendedErrorCode(), -1);
	}

	TEST(Exception, ThrowCatch)
	{
		MemoryLeakDetector leakDetector;

		const TrkString message = "Some Error";
		try
		{
			throw TrkDatabaseException(message);
		}
		catch (const std::runtime_error& ex)
		{
			EXPECT_STREQ(ex.what(), message);
		}
	}

	TEST(Exception, Constructors)
	{
		MemoryLeakDetector leakDetector;

		const char msg1[] = "some error";
		const TrkString msg2 = "another error";
		{
			const TrkDatabaseException ex(msg1);
			EXPECT_STREQ(ex.what(), msg1);
			EXPECT_EQ(ex.GetErrorCode(), -1);
			EXPECT_EQ(ex.GetExtendedErrorCode(), -1);
		}
		{
			const TrkDatabaseException ex(msg2);
			EXPECT_STREQ(ex.what(), msg2);
			EXPECT_EQ(ex.GetErrorCode(), -1);
			EXPECT_EQ(ex.GetExtendedErrorCode(), -1);
		}
		{
			const TrkDatabaseException ex(msg1, 1);
			EXPECT_STREQ(ex.what(), msg1);
			EXPECT_EQ(ex.GetErrorCode(), 1);
			EXPECT_EQ(ex.GetExtendedErrorCode(), -1);
		}
		{
			const TrkDatabaseException ex(msg2, 2);
			EXPECT_STREQ(ex.what(), msg2);
			EXPECT_EQ(ex.GetErrorCode(), 2);
			EXPECT_EQ(ex.GetExtendedErrorCode(), -1);
		}
	}


	/*
	 *
	 *	TrkDatabase Tests
	 *
	 */


	TEST(Database, ExecCreateDropExist) {
		MemoryLeakDetector leakDetector;
		remove("test.db");

		{
			EXPECT_THROW(TrkDatabase not_found("test.db"), TrkDatabaseException);

			TrkDatabase db("test.db", OPEN_READWRITE | OPEN_CREATE);

			EXPECT_FALSE(db.TableExists("test"));
			EXPECT_EQ(db.GetLastInsertRowID(), 0);

			EXPECT_EQ(db.Execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)"), 0);
			EXPECT_TRUE(db.TableExists("test"));
			EXPECT_EQ(db.GetLastInsertRowID(), 0);

			EXPECT_EQ(db.Execute("DROP TABLE IF EXISTS test"), 0);
			EXPECT_FALSE(db.TableExists("test"));
			EXPECT_EQ(db.GetLastInsertRowID(), 0);
		}

		remove("test.db");
	}

	TEST(Database, CreateCloseReopen) {
		MemoryLeakDetector leakDetector;
		remove("test.db");

		{
			EXPECT_THROW(TrkDatabase not_found("test.db"), TrkDatabaseException);

			TrkDatabase db("test.db", OPEN_READWRITE | OPEN_CREATE);
			EXPECT_FALSE(db.TableExists("test"));
			db.Execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");
			EXPECT_TRUE(db.TableExists("test"));
		}
		{
			TrkDatabase db("test.db", OPEN_READWRITE | OPEN_CREATE);
			EXPECT_TRUE(db.TableExists("test"));
		}

		remove("test.db");
	}

	TEST(Database, InMemory)
	{
		MemoryLeakDetector leakDetector;

		{
			TrkDatabase db(":memory:", OPEN_READWRITE);
			EXPECT_FALSE(db.TableExists("test"));
			db.Execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");
			EXPECT_TRUE(db.TableExists("test"));
			TrkDatabase db2(":memory:");
			EXPECT_FALSE(db2.TableExists("test"));
		}
		{
			TrkDatabase db(":memory:", OPEN_READWRITE);
			EXPECT_FALSE(db.TableExists("test"));
		}
	}

	TEST(Database, Exec)
	{
		MemoryLeakDetector leakDetector;

		{
			TrkDatabase db(":memory:", OPEN_READWRITE);

			db.Execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");
			EXPECT_EQ(db.GetChanges(), 0);
			EXPECT_EQ(db.GetLastInsertRowID(), 0);
			EXPECT_EQ(db.GetTotalChanges(), 0);

			db.Execute("INSERT INTO test VALUES(NULL, \"first\")");
			EXPECT_EQ(db.GetChanges(), 1);
			EXPECT_EQ(db.GetLastInsertRowID(), 1);
			EXPECT_EQ(db.GetTotalChanges(), 1);

			db.Execute("INSERT INTO test VALUES(NULL, \"second\")");
			EXPECT_EQ(db.GetChanges(), 1);
			EXPECT_EQ(db.GetLastInsertRowID(), 2);
			EXPECT_EQ(db.GetTotalChanges(), 2);

			db.Execute("INSERT INTO test VALUES(NULL, \"third\")");
			EXPECT_EQ(db.GetChanges(), 1);
			EXPECT_EQ(db.GetLastInsertRowID(), 3);
			EXPECT_EQ(db.GetTotalChanges(), 3);

			db.Execute("UPDATE test SET value=\"second-updated\" WHERE id='2'");
			EXPECT_EQ(db.GetChanges(), 1);
			EXPECT_EQ(db.GetLastInsertRowID(), 3);
			EXPECT_EQ(db.GetTotalChanges(), 4);

			db.Execute("DELETE FROM test WHERE id='3'");
			EXPECT_EQ(db.GetChanges(), 1);
			EXPECT_EQ(db.GetLastInsertRowID(), 3);
			EXPECT_EQ(db.GetTotalChanges(), 5);

			db.Execute("DROP TABLE IF EXISTS test");
			EXPECT_FALSE(db.TableExists("test"));
			EXPECT_EQ(5, db.GetTotalChanges());

			db.Execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");
			EXPECT_EQ(5, db.GetTotalChanges());

			EXPECT_EQ(1, db.Execute("INSERT INTO test VALUES (NULL, \"first\");INSERT INTO test VALUES (NULL, \"second\");"));
			EXPECT_EQ(1, db.GetChanges());
			EXPECT_EQ(2, db.GetLastInsertRowID());
			EXPECT_EQ(7, db.GetTotalChanges());

			EXPECT_EQ(2, db.Execute("INSERT INTO test VALUES (NULL, \"third\"), (NULL, \"fourth\");"));
			EXPECT_EQ(2, db.GetChanges());
			EXPECT_EQ(4, db.GetLastInsertRowID());
			EXPECT_EQ(9, db.GetTotalChanges());
		}
	}

	TEST(Database, TryExec)
	{
		MemoryLeakDetector leakDetector;

		{
			TrkDatabase db(":memory:", OPEN_READWRITE);

			EXPECT_EQ(TrkSqlite::OK, db.TryExecute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)"));
			EXPECT_EQ(0, db.GetChanges());
			EXPECT_EQ(0, db.GetLastInsertRowID());
			EXPECT_EQ(0, db.GetTotalChanges());

			EXPECT_EQ(TrkSqlite::OK, db.TryExecute("INSERT INTO test VALUES (NULL, \"first\")"));
			EXPECT_EQ(1, db.GetChanges());
			EXPECT_EQ(1, db.GetLastInsertRowID());
			EXPECT_EQ(1, db.GetTotalChanges());

			EXPECT_EQ(TrkSqlite::OK, db.TryExecute("INSERT INTO test VALUES (NULL, \"second\")"));
			EXPECT_EQ(1, db.GetChanges());
			EXPECT_EQ(2, db.GetLastInsertRowID());
			EXPECT_EQ(2, db.GetTotalChanges());

			TrkString insert("INSERT INTO test VALUES (NULL, \"third\")");
			EXPECT_EQ(TrkSqlite::OK, db.TryExecute(insert));
			EXPECT_EQ(1, db.GetChanges());
			EXPECT_EQ(3, db.GetLastInsertRowID());
			EXPECT_EQ(3, db.GetTotalChanges());

			EXPECT_EQ(TrkSqlite::OK, db.TryExecute("UPDATE test SET value=\"second-updated\" WHERE id='2'"));
			EXPECT_EQ(1, db.GetChanges());
			EXPECT_EQ(3, db.GetLastInsertRowID());
			EXPECT_EQ(4, db.GetTotalChanges());

			EXPECT_EQ(TrkSqlite::OK, db.TryExecute("DELETE FROM test WHERE id='3'"));
			EXPECT_EQ(1, db.GetChanges());
			EXPECT_EQ(3, db.GetLastInsertRowID());
			EXPECT_EQ(5, db.GetTotalChanges());

			EXPECT_EQ(TrkSqlite::OK, db.TryExecute("DROP TABLE IF EXISTS test"));
			EXPECT_FALSE(db.TableExists("test"));
			EXPECT_EQ(5, db.GetTotalChanges());

			EXPECT_EQ(TrkSqlite::OK, db.TryExecute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)"));
			EXPECT_EQ(5, db.GetTotalChanges());

			EXPECT_EQ(TrkSqlite::OK, db.TryExecute("INSERT INTO test VALUES (NULL, \"first\");INSERT INTO test VALUES (NULL, \"second\");"));
			EXPECT_EQ(1, db.GetChanges());
			EXPECT_EQ(2, db.GetLastInsertRowID());
			EXPECT_EQ(7, db.GetTotalChanges());

			EXPECT_EQ(TrkSqlite::OK, db.TryExecute("INSERT INTO test VALUES (NULL, \"third\"), (NULL, \"fourth\");"));
			EXPECT_EQ(2, db.GetChanges());
			EXPECT_EQ(4, db.GetLastInsertRowID());
			EXPECT_EQ(9, db.GetTotalChanges());
		}
	}

	TEST(Database, ExecException)
	{
		MemoryLeakDetector leakDetector;

		{
			TrkDatabase db(":memory:", OPEN_READWRITE);

			EXPECT_EQ(TrkSqlite::OK, db.GetErrorCode());
			EXPECT_EQ(TrkSqlite::OK, db.GetExtendedErrorCode());

			EXPECT_THROW(db.Execute("INSERT INTO test VALUES (NULL, \"first\",  3)"), TrkDatabaseException);
			EXPECT_EQ(TrkSqlite::CODE_ERROR, db.GetErrorCode());
			EXPECT_EQ(TrkSqlite::CODE_ERROR, db.GetExtendedErrorCode());

			db.Execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT, weight INTEGER)");
			EXPECT_EQ(TrkSqlite::OK, db.GetErrorCode());
			EXPECT_EQ(TrkSqlite::OK, db.GetExtendedErrorCode());

			EXPECT_THROW(db.Execute("INSERT INTO test VALUES (NULL,  3)"), TrkDatabaseException);
			EXPECT_EQ(TrkSqlite::CODE_ERROR, db.GetErrorCode());
			EXPECT_EQ(TrkSqlite::CODE_ERROR, db.GetExtendedErrorCode());

			EXPECT_EQ(1, db.Execute("INSERT INTO test VALUES (NULL, \"first\",  3)"));

			EXPECT_THROW(db.Execute("INSERT INTO test VALUES (NULL, \"first\", 123, 0.123)"), TrkDatabaseException);
			EXPECT_EQ(TrkSqlite::CODE_ERROR, db.GetErrorCode());
			EXPECT_EQ(TrkSqlite::CODE_ERROR, db.GetExtendedErrorCode());
		}
	}

	TEST(Database, TryExecError)
	{
		MemoryLeakDetector leakDetector;

		{
			TrkDatabase db(":memory:", OPEN_READWRITE);
			EXPECT_EQ(TrkSqlite::OK, db.GetErrorCode());
			EXPECT_EQ(TrkSqlite::OK, db.GetExtendedErrorCode());

			EXPECT_EQ(TrkSqlite::CODE_ERROR, db.TryExecute("INSERT INTO test VALUES (NULL, \"first\", 3)"));
			EXPECT_EQ(TrkSqlite::CODE_ERROR, db.GetErrorCode());
			EXPECT_EQ(TrkSqlite::CODE_ERROR, db.GetExtendedErrorCode());

			EXPECT_EQ(TrkSqlite::OK, db.TryExecute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT, weight INTEGER)"));
			EXPECT_EQ(TrkSqlite::OK, db.GetErrorCode());
			EXPECT_EQ(TrkSqlite::OK, db.GetExtendedErrorCode());

			EXPECT_EQ(TrkSqlite::CODE_ERROR, db.TryExecute("INSERT INTO test VALUES (NULL, 3)"));
			EXPECT_EQ(TrkSqlite::CODE_ERROR, db.GetErrorCode());
			EXPECT_EQ(TrkSqlite::CODE_ERROR, db.GetExtendedErrorCode());

			EXPECT_EQ(TrkSqlite::CODE_ERROR, db.TryExecute("INSERT INTO test VALUES (NULL, \"firsst\", 3, 0.123)"));
			EXPECT_EQ(TrkSqlite::CODE_ERROR, db.GetErrorCode());
			EXPECT_EQ(TrkSqlite::CODE_ERROR, db.GetExtendedErrorCode());

			EXPECT_EQ(TrkSqlite::OK, db.TryExecute("INSERT INTO test VALUES (NULL, \"firsst\", 3)"));
			EXPECT_EQ(1, db.GetLastInsertRowID());

			EXPECT_EQ(TrkSqlite::CODE_CONSTRAINT, db.TryExecute("INSERT INTO test VALUES (1, \"impossible\", 456)"));
			EXPECT_EQ(TrkSqlite::CODE_CONSTRAINT, db.GetErrorCode());
			EXPECT_EQ(TrkSqlite::CODE_CONSTRAINT_PRIMARYKEY, db.GetExtendedErrorCode());
		}
	}


	/*
	 *
	 *	TrkStatement Tests
	 *
	 */


	TEST(Statement, Invalid)
	{
		MemoryLeakDetector leakDetector;

		{
			TrkDatabase db(":memory:", TrkSqlite::OPEN_READWRITE | TrkSqlite::OPEN_CREATE);
			EXPECT_EQ(TrkSqlite::OK, db.GetErrorCode());
			EXPECT_EQ(TrkSqlite::OK, db.GetExtendedErrorCode());

			EXPECT_THROW(TrkStatement query(db, "SELECT * FROM test"), TrkDatabaseException);
			EXPECT_EQ(TrkSqlite::CODE_ERROR, db.GetErrorCode());
			EXPECT_EQ(TrkSqlite::CODE_ERROR, db.GetExtendedErrorCode());

			EXPECT_EQ(0, db.Execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)"));
			EXPECT_EQ(TrkSqlite::OK, db.GetErrorCode());
			EXPECT_EQ(TrkSqlite::OK, db.GetExtendedErrorCode());

			TrkStatement query(db, "SELECT * FROM test");
			EXPECT_STREQ("SELECT * FROM test", query.GetQuery());
			EXPECT_EQ(2, query.GetColumnCount());
			EXPECT_FALSE(query.HasRow());
			EXPECT_FALSE(query.Done());
			EXPECT_EQ(TrkSqlite::OK, query.GetErrorCode());
			EXPECT_EQ(TrkSqlite::OK, query.GetErrorCode());
			EXPECT_THROW(query.IsColumnNull(-1), TrkDatabaseException);
			EXPECT_THROW(query.IsColumnNull(0), TrkDatabaseException);
			EXPECT_THROW(query.IsColumnNull(1), TrkDatabaseException);
			EXPECT_THROW(query.IsColumnNull(2), TrkDatabaseException);
			EXPECT_THROW(query.GetColumn(-1), TrkDatabaseException);
			EXPECT_THROW(query.GetColumn(0), TrkDatabaseException);
			EXPECT_THROW(query.GetColumn(1), TrkDatabaseException);
			EXPECT_THROW(query.GetColumn(2), TrkDatabaseException);

			query.Reset();
			EXPECT_FALSE(query.HasRow());
			EXPECT_FALSE(query.Done());

			query.ExecuteStep();
			EXPECT_FALSE(query.HasRow());
			EXPECT_TRUE(query.Done());
			query.Reset();
			EXPECT_FALSE(query.HasRow());
			EXPECT_FALSE(query.Done());

			query.Reset();
			EXPECT_THROW(query.Bind(-1, 123), TrkDatabaseException);
			EXPECT_THROW(query.Bind(0, 123), TrkDatabaseException);
			EXPECT_THROW(query.Bind(1, 123), TrkDatabaseException);
			EXPECT_THROW(query.Bind(2, 123), TrkDatabaseException);
			EXPECT_THROW(query.Bind(0, "abc"), TrkDatabaseException);
			EXPECT_THROW(query.Bind(0), TrkDatabaseException);
			EXPECT_EQ(TrkSqlite::CODE_RANGE, db.GetErrorCode());
			EXPECT_EQ(TrkSqlite::CODE_RANGE, db.GetExtendedErrorCode());
			EXPECT_EQ(TrkSqlite::CODE_RANGE, query.GetErrorCode());
			EXPECT_EQ(TrkSqlite::CODE_RANGE, query.GetExtendedErrorCode());

			query.Execute();
			EXPECT_THROW(query.IsColumnNull(0), TrkDatabaseException);
			EXPECT_THROW(query.GetColumn(0), TrkDatabaseException);

			EXPECT_THROW(query.Execute(), TrkDatabaseException);

			EXPECT_EQ(1, db.Execute("INSERT INTO test VALUES (NULL, \"first\")"));
			EXPECT_EQ(1, db.GetLastInsertRowID());
			EXPECT_EQ(1, db.GetTotalChanges());

			query.Reset();
			EXPECT_FALSE(query.HasRow());
			EXPECT_FALSE(query.Done());

			EXPECT_THROW(query.Execute(), TrkDatabaseException);
		}
	}

	TEST(Statement, ExecuteStep)
	{
		MemoryLeakDetector leakDetector;

		{
			TrkDatabase db(":memory:", TrkSqlite::OPEN_READWRITE | TrkSqlite::OPEN_CREATE);
			EXPECT_EQ(TrkSqlite::OK, db.GetErrorCode());

			EXPECT_EQ(0, db.Execute("CREATE TABLE test (id INTEGER PRIMARY KEY, msg TEXT, int INTEGER, double REAL)"));
			EXPECT_EQ(TrkSqlite::OK, db.GetErrorCode());

			EXPECT_EQ(1, db.Execute("INSERT INTO test VALUES (NULL, \"first\", 123, 0.123)"));
			EXPECT_EQ(1, db.GetLastInsertRowID());

			TrkStatement query(db, "SELECT * FROM test");
			EXPECT_STREQ("SELECT * FROM test", query.GetQuery());
			EXPECT_EQ(4, query.GetColumnCount());

			query.ExecuteStep();
			EXPECT_TRUE(query.HasRow());
			const int64_t		id = query.GetColumn(0);
			const TrkString		msg = query.GetColumn(1);
			const int			integer = query.GetColumn(2);
			const int64_t		integer2 = query.GetColumn(2);
			const double		real = query.GetColumn(3);
			EXPECT_EQ(1, id);
			EXPECT_STREQ("first", msg);
			EXPECT_EQ(123, integer);
			EXPECT_EQ(123, integer2);
			EXPECT_DOUBLE_EQ(0.123, real);

			query.ExecuteStep();
			EXPECT_FALSE(query.HasRow());

			EXPECT_THROW(query.ExecuteStep(), TrkDatabaseException);

			TrkStatement insert(db, "INSERT INTO test VALUES (1, \"impossible\", 456, 0.456)");
			EXPECT_THROW(insert.ExecuteStep(), TrkDatabaseException);
			EXPECT_THROW(insert.Reset(), TrkDatabaseException);

			TrkStatement insert2(db, "INSERT INTO test VALUES (1, \"impossible\", 456, 0.456)");
			EXPECT_THROW(insert2.Execute(), TrkDatabaseException);
		}
	}

	TEST(Statement, tryExecuteStep)
	{
		MemoryLeakDetector leakDetector;

		{
			TrkDatabase db(":memory:", TrkSqlite::OPEN_READWRITE | TrkSqlite::OPEN_CREATE);
			EXPECT_EQ(TrkSqlite::OK, db.GetErrorCode());

			EXPECT_EQ(0, db.Execute("CREATE TABLE test (id INTEGER PRIMARY KEY, msg TEXT, int INTEGER, double REAL)"));
			EXPECT_EQ(TrkSqlite::OK, db.GetErrorCode());

			EXPECT_EQ(1, db.Execute("INSERT INTO test VALUES (NULL, \"first\", 123, 0.123)"));
			EXPECT_EQ(1, db.GetLastInsertRowID());

			TrkStatement query(db, "SELECT * FROM test");
			EXPECT_STREQ("SELECT * FROM test", query.GetQuery());
			EXPECT_EQ(4, query.GetColumnCount());

			EXPECT_EQ(query.TryExecuteStep(), TrkSqlite::ROW);
			EXPECT_TRUE(query.HasRow());
			const int64_t		id = query.GetColumn(0);
			const TrkString		msg = query.GetColumn(1);
			const int			integer = query.GetColumn(2);
			const int64_t		integer2 = query.GetColumn(2);
			const double		real = query.GetColumn(3);
			EXPECT_EQ(1, id);
			EXPECT_STREQ("first", msg);
			EXPECT_EQ(123, integer);
			EXPECT_EQ(123, integer2);
			EXPECT_DOUBLE_EQ(0.123, real);

			EXPECT_EQ(query.TryExecuteStep(), TrkSqlite::DONE);
			EXPECT_FALSE(query.HasRow());

			TrkStatement insert(db, "INSERT INTO test VALUES (1, \"impossible\", 456, 0.456)");
			EXPECT_EQ(insert.TryExecuteStep(), TrkSqlite::CODE_CONSTRAINT);
			EXPECT_EQ(insert.TryReset(), TrkSqlite::CODE_CONSTRAINT);
		}
	}

	TEST(Statement, Bindings)
	{
		MemoryLeakDetector leakDetector;

		{
			TrkDatabase db(":memory:", TrkSqlite::OPEN_READWRITE | TrkSqlite::OPEN_CREATE);
			EXPECT_EQ(TrkSqlite::OK, db.GetErrorCode());

			EXPECT_EQ(0, db.Execute("CREATE TABLE test (id INTEGER PRIMARY KEY, msg TEXT, int INTEGER, double REAL)"));
			EXPECT_EQ(TrkSqlite::OK, db.GetErrorCode());

			TrkStatement insert(db, "INSERT INTO test VALUES (NULL, ?, ?, ?)");

			TrkStatement query(db, "SELECT * FROM test");
			EXPECT_STREQ("SELECT * FROM test", query.GetQuery());
			EXPECT_EQ(4, query.GetColumnCount());

			{
				const char* text = "first";
				const int integer = -123;
				const double dbl = 0.123;
				insert.Bind(1, text);
				insert.Bind(2, integer);
				insert.Bind(3, dbl);
				EXPECT_EQ(insert.GetExpandedQuery(), "INSERT INTO test VALUES (NULL, 'first', -123, 0.123)");
				EXPECT_EQ(1, insert.Execute());
				EXPECT_EQ(TrkSqlite::DONE, db.GetErrorCode());

				query.ExecuteStep();
				EXPECT_TRUE(query.HasRow());
				EXPECT_EQ(1, query.GetColumn(0).GetInt64());
				EXPECT_STREQ("first", query.GetColumn(1).GetString());
				EXPECT_EQ(-123, query.GetColumn(2).GetInt());
				EXPECT_EQ(0.123, query.GetColumn(3).GetDouble());
			}

			insert.Reset();

			{
				EXPECT_EQ(1, insert.Execute());
				EXPECT_EQ(TrkSqlite::DONE, db.GetErrorCode());

				query.ExecuteStep();
				EXPECT_TRUE(query.HasRow());
				EXPECT_EQ(2, query.GetColumn(0).GetInt64());
				EXPECT_STREQ("first", query.GetColumn(1).GetString());
				EXPECT_EQ(-123, query.GetColumn(2).GetInt());
				EXPECT_EQ(0.123, query.GetColumn(3).GetDouble());
			}

			insert.Reset();
			insert.ClearBindings();

			{
				EXPECT_EQ(1, insert.Execute());
				EXPECT_EQ(TrkSqlite::DONE, db.GetErrorCode());

				query.ExecuteStep();
				EXPECT_TRUE(query.HasRow());
				EXPECT_EQ(3, query.GetColumn(0).GetInt64());
				EXPECT_TRUE(query.IsColumnNull(1));
				EXPECT_STREQ("", query.GetColumn(1).GetString());
				EXPECT_TRUE(query.IsColumnNull(2));
				EXPECT_EQ(0, query.GetColumn(2).GetInt());
				EXPECT_TRUE(query.IsColumnNull(3));
				EXPECT_EQ(0.0, query.GetColumn(3).GetDouble());
			}

			insert.Reset();
			insert.ClearBindings();

			{
				const TrkString   fourth("fourth");
				const int64_t       int64 = 12345678900000LL;
				const float         float32 = 0.234f;
				insert.Bind(1, fourth);
				insert.Bind(2, int64);
				insert.Bind(3, float32);
				EXPECT_EQ(1, insert.Execute());
				EXPECT_EQ(TrkSqlite::DONE, db.GetErrorCode());

				query.ExecuteStep();
				EXPECT_TRUE(query.HasRow());
				EXPECT_EQ(4, query.GetColumn(0).GetInt64());
				EXPECT_EQ(fourth, query.GetColumn(1).GetString());
				EXPECT_EQ(12345678900000LL, query.GetColumn(2).GetInt64());
				EXPECT_FLOAT_EQ(0.234f, (float)query.GetColumn(3).GetDouble());
			}

			insert.Reset();

			{
				const char buffer[] = "binary";
				insert.Bind(1, buffer, sizeof(buffer));
				insert.Bind(2);
				EXPECT_EQ(1, insert.Execute());

				// Check the result
				query.ExecuteStep();
				EXPECT_TRUE(query.HasRow());
				EXPECT_EQ(5, query.GetColumn(0).GetInt64());
				EXPECT_STREQ(buffer, query.GetColumn(1).GetString());
				EXPECT_TRUE(query.IsColumnNull(2));
				EXPECT_EQ(0, query.GetColumn(2).GetInt());
				EXPECT_FLOAT_EQ(0.234f, (float)query.GetColumn(3).GetDouble());
			}


			insert.Reset();

			{
				const uint32_t  uint32 = 4294967295U;
				const int64_t   integer = -123;
				insert.Bind(2, uint32);
				insert.Bind(3, integer);
				EXPECT_EQ(1, insert.Execute());
				EXPECT_EQ(TrkSqlite::DONE, db.GetErrorCode());

				query.ExecuteStep();
				EXPECT_TRUE(query.HasRow());
				EXPECT_EQ(6, query.GetColumn(0).GetInt64());
				EXPECT_EQ(4294967295U, query.GetColumn(2).GetUInt());
				EXPECT_EQ(-123, query.GetColumn(3).GetInt());
			}


			insert.Reset();

			{
				const int64_t   int64 = 12345678900000LL;
				insert.Bind(2, int64);
				EXPECT_EQ(1, insert.Execute());
				EXPECT_EQ(TrkSqlite::DONE, db.GetErrorCode());

				query.ExecuteStep();
				EXPECT_TRUE(query.HasRow());
				EXPECT_EQ(7, query.GetColumn(0).GetInt64());
				EXPECT_EQ(12345678900000LL, query.GetColumn(2).GetInt64());
			}
		}
	}

	TEST(Statement, BindByName)
	{
		MemoryLeakDetector leakDetector;

		{
			TrkDatabase db(":memory:", TrkSqlite::OPEN_READWRITE | TrkSqlite::OPEN_CREATE);
			EXPECT_EQ(TrkSqlite::OK, db.GetErrorCode());

			EXPECT_EQ(0, db.Execute("CREATE TABLE test (id INTEGER PRIMARY KEY, msg TEXT, int INTEGER, long INTEGER, double REAL)"));
			EXPECT_EQ(TrkSqlite::OK, db.GetErrorCode());

			TrkStatement insert(db, "INSERT INTO test VALUES (NULL, @msg, @int, @long, @double)");

			insert.Bind("@msg", "first");
			insert.Bind("@int", 123);
			insert.Bind("@long", -123);
			insert.Bind("@double", 0.123);
			EXPECT_EQ(1, insert.Execute());
			EXPECT_EQ(TrkSqlite::DONE, db.GetErrorCode());

			TrkStatement query(db, "SELECT * FROM test");
			EXPECT_STREQ("SELECT * FROM test", query.GetQuery());
			EXPECT_EQ(5, query.GetColumnCount());

			query.ExecuteStep();
			EXPECT_TRUE(query.HasRow());
			EXPECT_EQ(1, query.GetColumn(0).GetInt64());
			EXPECT_STREQ("first", query.GetColumn(1).GetString());
			EXPECT_EQ(123, query.GetColumn(2).GetInt());
			EXPECT_EQ(-123, query.GetColumn(3).GetInt());
			EXPECT_EQ(0.123, query.GetColumn(4).GetDouble());

			insert.Reset();
			insert.ClearBindings();

			{
				const TrkString		second("second");
				const int32_t		int32 = -123;
				const int64_t		int64 = 12345678900000LL;
				const float			float32 = 0.234f;
				insert.Bind("@msg", second);
				insert.Bind("@int", int32);
				insert.Bind("@long", int64);
				insert.Bind("@double", float32);
				EXPECT_EQ(1, insert.Execute());
				EXPECT_EQ(TrkSqlite::DONE, db.GetErrorCode());

				query.ExecuteStep();
				EXPECT_TRUE(query.HasRow());
				EXPECT_EQ(2, query.GetColumn(0).GetInt64());
				EXPECT_EQ(second, query.GetColumn(1).GetString());
				EXPECT_EQ(-123, query.GetColumn(2).GetInt());
				EXPECT_EQ(12345678900000LL, query.GetColumn(3).GetInt64());
				EXPECT_FLOAT_EQ(0.234f, (float)query.GetColumn(4).GetDouble());
			}

			insert.Reset();

			{
				const char buffer[] = "binary";
				insert.Bind("@msg", buffer, sizeof(buffer));
				insert.Bind("@int");
				EXPECT_EQ(1, insert.Execute());

				query.ExecuteStep();
				EXPECT_TRUE(query.HasRow());
				EXPECT_EQ(3, query.GetColumn(0).GetInt64());
				EXPECT_STREQ(buffer, query.GetColumn(1).GetString());
				EXPECT_TRUE(query.IsColumnNull(2));
				EXPECT_EQ(0, query.GetColumn(2).GetInt());
				EXPECT_FLOAT_EQ(0.234f, (float)query.GetColumn(4).GetDouble());
			}
			
			insert.Reset();

			{
				const uint32_t  uint32 = 4294967295U;
				const int64_t   int64 = 12345678900000LL;
				insert.Bind("@int", uint32);
				insert.Bind("@long", int64);
				EXPECT_EQ(1, insert.Execute());
				EXPECT_EQ(TrkSqlite::DONE, db.GetErrorCode());

				query.ExecuteStep();
				EXPECT_TRUE(query.HasRow());
				EXPECT_EQ(4, query.GetColumn(0).GetInt64());
				EXPECT_EQ(4294967295U, query.GetColumn(2).GetUInt());
				EXPECT_EQ(12345678900000LL, query.GetColumn(3).GetInt64());
			}
		}
	}


	TEST(Statement, BindByNameString)
	{
		MemoryLeakDetector leakDetector;

		{
			TrkDatabase db(":memory:", TrkSqlite::OPEN_READWRITE | TrkSqlite::OPEN_CREATE);
			EXPECT_EQ(TrkSqlite::OK, db.GetErrorCode());

			EXPECT_EQ(0, db.Execute("CREATE TABLE test (id INTEGER PRIMARY KEY, msg TEXT, int INTEGER, double REAL, long INTEGER)"));
			EXPECT_EQ(TrkSqlite::OK, db.GetErrorCode());

			TrkStatement insert(db, "INSERT INTO test VALUES (NULL, @msg, @int, @double, @long)");

			const TrkString amsg = "@msg";
			const TrkString aint = "@int";
			const TrkString along = "@long";
			const TrkString adouble = "@double";

			insert.Bind(amsg, "first");
			insert.Bind(aint, 123);
			insert.Bind(along, -123);
			insert.Bind(adouble, 0.123);
			EXPECT_EQ(1, insert.Execute());
			EXPECT_EQ(TrkSqlite::DONE, db.GetErrorCode());

			TrkStatement query(db, "SELECT * FROM test");
			EXPECT_STREQ("SELECT * FROM test", query.GetQuery());
			EXPECT_EQ(5, query.GetColumnCount());

			query.ExecuteStep();
			EXPECT_TRUE(query.HasRow());
			EXPECT_EQ(1, query.GetColumn(0).GetInt64());
			EXPECT_STREQ("first", query.GetColumn(1).GetString());
			EXPECT_EQ(123, query.GetColumn(2).GetInt());
			EXPECT_DOUBLE_EQ(0.123, query.GetColumn(3).GetDouble());
			EXPECT_EQ(-123, query.GetColumn(4).GetInt());

			insert.Reset();
			insert.ClearBindings();

			{
				const TrkString		second("second");
				const int64_t		int64 = 12345678900000LL;
				const int64_t		integer = -123;
				const float			float32 = 0.234f;
				insert.Bind(amsg, second);
				insert.Bind(aint, int64);
				insert.Bind(adouble, float32);
				insert.Bind(along, integer);
				EXPECT_EQ(1, insert.Execute());
				EXPECT_EQ(TrkSqlite::DONE, db.GetErrorCode());

				query.ExecuteStep();
				EXPECT_TRUE(query.HasRow());
				EXPECT_EQ(2, query.GetColumn(0).GetInt64());
				EXPECT_EQ(second, query.GetColumn(1).GetString());
				EXPECT_EQ(12345678900000LL, query.GetColumn(2).GetInt64());
				EXPECT_FLOAT_EQ(0.234f, (float)query.GetColumn(3).GetDouble());
				EXPECT_EQ(-123, query.GetColumn(4).GetInt());
			}

			insert.Reset();

			{
				const char buffer[] = "binary";
				insert.Bind(amsg, buffer, sizeof(buffer));
				insert.Bind(aint);
				EXPECT_EQ(1, insert.Execute());

				query.ExecuteStep();
				EXPECT_TRUE(query.HasRow());
				EXPECT_EQ(3, query.GetColumn(0).GetInt64());
				EXPECT_STREQ(buffer, query.GetColumn(1).GetString());
				EXPECT_TRUE(query.IsColumnNull(2));
				EXPECT_EQ(0, query.GetColumn(2).GetInt());
				EXPECT_FLOAT_EQ(0.234f, (float)query.GetColumn(3).GetDouble());
			}

			insert.Reset();

			{
				const uint32_t  uint32 = 4294967295U;
				const int64_t   int64 = 12345678900000LL;
				insert.Bind(aint, uint32);
				insert.Bind(along, int64);
				EXPECT_EQ(1, insert.Execute());
				EXPECT_EQ(TrkSqlite::DONE, db.GetErrorCode());

				query.ExecuteStep();
				EXPECT_TRUE(query.HasRow());
				EXPECT_EQ(4, query.GetColumn(0).GetInt64());
				EXPECT_EQ(4294967295U, query.GetColumn(2).GetUInt());
				EXPECT_EQ(12345678900000LL, query.GetColumn(4).GetInt64());
			}
		}
	}

	TEST(Statement, IsColumnNull)
	{
		MemoryLeakDetector leakDetector;

		{
			TrkDatabase db(":memory:", TrkSqlite::OPEN_READWRITE | TrkSqlite::OPEN_CREATE);
			ASSERT_EQ(TrkSqlite::OK, db.GetErrorCode());

			EXPECT_EQ(0, db.Execute("CREATE TABLE test (msg TEXT, int INTEGER, double REAL)"));
			ASSERT_EQ(TrkSqlite::OK, db.GetErrorCode());

			ASSERT_EQ(1, db.Execute("INSERT INTO test VALUES (\"first\", 123,  0.123)"));
			ASSERT_EQ(1, db.Execute("INSERT INTO test VALUES (NULL,      123,  0.123)"));
			ASSERT_EQ(1, db.Execute("INSERT INTO test VALUES (\"first\", NULL, 0.123)"));
			ASSERT_EQ(1, db.Execute("INSERT INTO test VALUES (\"first\", 123,  NULL)"));

			const TrkString select("SELECT * FROM test");
			TrkStatement query(db, select);
			EXPECT_EQ(select, query.GetQuery());
			EXPECT_EQ(3, query.GetColumnCount());

			query.ExecuteStep();
			EXPECT_TRUE(query.HasRow());
			EXPECT_THROW(query.IsColumnNull(-1), TrkDatabaseException);
			EXPECT_EQ(false, query.IsColumnNull(0));
			EXPECT_EQ(false, query.IsColumnNull(1));
			EXPECT_EQ(false, query.IsColumnNull(2));
			EXPECT_THROW(query.IsColumnNull(3), TrkDatabaseException);

			query.ExecuteStep();
			EXPECT_TRUE(query.HasRow());
			EXPECT_THROW(query.IsColumnNull(-1), TrkDatabaseException);
			EXPECT_EQ(true, query.IsColumnNull(0));
			EXPECT_EQ(false, query.IsColumnNull(1));
			EXPECT_EQ(false, query.IsColumnNull(2));
			EXPECT_THROW(query.IsColumnNull(3), TrkDatabaseException);

			query.ExecuteStep();
			EXPECT_TRUE(query.HasRow());
			EXPECT_THROW(query.IsColumnNull(-1), TrkDatabaseException);
			EXPECT_EQ(false, query.IsColumnNull(0));
			EXPECT_EQ(true, query.IsColumnNull(1));
			EXPECT_EQ(false, query.IsColumnNull(2));
			EXPECT_THROW(query.IsColumnNull(3), TrkDatabaseException);

			query.ExecuteStep();
			EXPECT_TRUE(query.HasRow());
			EXPECT_THROW(query.IsColumnNull(-1), TrkDatabaseException);
			EXPECT_EQ(false, query.IsColumnNull(0));
			EXPECT_EQ(false, query.IsColumnNull(1));
			EXPECT_EQ(true, query.IsColumnNull(2));
			EXPECT_THROW(query.IsColumnNull(3), TrkDatabaseException);
		}
	}

	TEST(Statement, IsColumnNullByName)
	{
		MemoryLeakDetector leakDetector;

		{
			TrkDatabase db(":memory:", TrkSqlite::OPEN_READWRITE | TrkSqlite::OPEN_CREATE);
			ASSERT_EQ(TrkSqlite::OK, db.GetErrorCode());

			EXPECT_EQ(0, db.Execute("CREATE TABLE test (msg TEXT, int INTEGER, double REAL)"));
			ASSERT_EQ(TrkSqlite::OK, db.GetErrorCode());

			ASSERT_EQ(1, db.Execute("INSERT INTO test VALUES (\"first\", 123,  0.123)"));
			ASSERT_EQ(1, db.Execute("INSERT INTO test VALUES (NULL,      123,  0.123)"));
			ASSERT_EQ(1, db.Execute("INSERT INTO test VALUES (\"first\", NULL, 0.123)"));
			ASSERT_EQ(1, db.Execute("INSERT INTO test VALUES (\"first\", 123,  NULL)"));

			const TrkString select("SELECT * FROM test");
			TrkStatement query(db, select);
			EXPECT_EQ(select, query.GetQuery());
			EXPECT_EQ(3, query.GetColumnCount());

			query.ExecuteStep();
			EXPECT_TRUE(query.HasRow());
			EXPECT_THROW(query.IsColumnNull(""), TrkDatabaseException);
			EXPECT_EQ(false, query.IsColumnNull("msg"));
			EXPECT_EQ(false, query.IsColumnNull("int"));
			EXPECT_EQ(false, query.IsColumnNull("double"));
			EXPECT_THROW(query.IsColumnNull(3), TrkDatabaseException);

			query.ExecuteStep();
			EXPECT_TRUE(query.HasRow());
			EXPECT_THROW(query.IsColumnNull(""), TrkDatabaseException);
			EXPECT_EQ(true, query.IsColumnNull("msg"));
			EXPECT_EQ(false, query.IsColumnNull(1));
			EXPECT_EQ(false, query.IsColumnNull("double"));
			EXPECT_THROW(query.IsColumnNull(3), TrkDatabaseException);

			query.ExecuteStep();
			EXPECT_TRUE(query.HasRow());
			EXPECT_THROW(query.IsColumnNull(""), TrkDatabaseException);
			EXPECT_EQ(false, query.IsColumnNull("msg"));
			EXPECT_EQ(true, query.IsColumnNull("int"));
			EXPECT_EQ(false, query.IsColumnNull("double"));
			EXPECT_THROW(query.IsColumnNull(3), TrkDatabaseException);

			query.ExecuteStep();
			EXPECT_TRUE(query.HasRow());
			EXPECT_THROW(query.IsColumnNull(""), TrkDatabaseException);
			EXPECT_EQ(false, query.IsColumnNull("msg"));
			EXPECT_EQ(false, query.IsColumnNull("int"));
			EXPECT_EQ(true, query.IsColumnNull("double"));
			EXPECT_THROW(query.IsColumnNull(3), TrkDatabaseException);
		}
	}

	TEST(Statement, GetColumnByName)
	{
		MemoryLeakDetector leakDetector;

		{
			TrkDatabase db(":memory:", TrkSqlite::OPEN_READWRITE | TrkSqlite::OPEN_CREATE);
			EXPECT_EQ(TrkSqlite::OK, db.GetErrorCode());
			EXPECT_EQ(TrkSqlite::OK, db.GetExtendedErrorCode());

			EXPECT_EQ(0, db.Execute("CREATE TABLE test (id INTEGER PRIMARY KEY, msg TEXT, int INTEGER, double REAL)"));
			EXPECT_EQ(TrkSqlite::OK, db.GetErrorCode());
			EXPECT_EQ(TrkSqlite::OK, db.GetExtendedErrorCode());

			EXPECT_EQ(1, db.Execute("INSERT INTO test VALUES (NULL, \"first\", 123, 0.123)"));
			EXPECT_EQ(1, db.GetLastInsertRowID());
			EXPECT_EQ(1, db.GetTotalChanges());

			TrkStatement query(db, "SELECT * FROM test");
			EXPECT_STREQ("SELECT * FROM test", query.GetQuery());
			EXPECT_EQ(4, query.GetColumnCount());
			query.ExecuteStep();
			EXPECT_TRUE(query.HasRow());

			EXPECT_THROW(query.GetColumn("unknown"), TrkDatabaseException);
			EXPECT_THROW(query.GetColumn(""), TrkDatabaseException);

			const TrkString		msg = query.GetColumn("msg");
			const int			integer = query.GetColumn("int");
			const double		real = query.GetColumn("double");
			EXPECT_STREQ("first", msg);
			EXPECT_EQ(123, integer);
			EXPECT_DOUBLE_EQ(0.123, real);
		}
	}

	TEST(Statement, GetName)
	{
		MemoryLeakDetector leakDetector;

		{
			TrkDatabase db(":memory:", TrkSqlite::OPEN_READWRITE | TrkSqlite::OPEN_CREATE);
			EXPECT_EQ(0, db.Execute("CREATE TABLE test (id INTEGER PRIMARY KEY, msg TEXT)"));

			TrkStatement query(db, "SELECT id, msg as value FROM test");
			query.ExecuteStep();

			const TrkString name0 = query.GetColumnName(0);
			const TrkString name1 = query.GetColumnName(1);
			EXPECT_STREQ("id", name0);
			EXPECT_STREQ("value", name1);
		}
	}

	/*
	 *
	 *	TrkTransaction Tests
	 *
	 */

	TEST(Transaction, commitRollback)
	{

		{
			TrkDatabase db(":memory:", TrkSqlite::OPEN_READWRITE | TrkSqlite::OPEN_CREATE);
			EXPECT_EQ(TrkSqlite::OK, db.GetErrorCode());

			{
				TrkTransaction transaction(db);

				EXPECT_EQ(0, db.Execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)"));
				EXPECT_EQ(TrkSqlite::OK, db.GetErrorCode());

				EXPECT_EQ(1, db.Execute("INSERT INTO test VALUES (NULL, \"first\")"));
				EXPECT_EQ(1, db.GetLastInsertRowID());

				transaction.Commit();

				EXPECT_THROW(transaction.Commit(), TrkDatabaseException);
			}

			{
				TrkTransaction transaction(db);

				EXPECT_EQ(1, db.Execute("INSERT INTO test VALUES (NULL, \"third\")"));
				EXPECT_EQ(2, db.GetLastInsertRowID());
			}

			try
			{
				TrkTransaction transaction(db);

				EXPECT_EQ(1, db.Execute("INSERT INTO test VALUES (NULL, \"second\")"));
				EXPECT_EQ(2, db.GetLastInsertRowID());

				db.Execute("DesiredSyntaxError to raise an exception to rollback the transaction");

				GTEST_FATAL_FAILURE_("we should never get there");
				transaction.Commit();
			}
			catch (TrkDatabaseException& e)
			{
				// There's nothing to do.
			}

			{
				TrkTransaction transaction(db);

				EXPECT_EQ(1, db.Execute("INSERT INTO test VALUES (NULL, \"third\")"));
				EXPECT_EQ(2, db.GetLastInsertRowID());

				transaction.Rollback();
			}

			TrkStatement query(db, "SELECT * FROM test");
			int nbRows = 0;
			while (query.ExecuteStep())
			{
				nbRows++;
				EXPECT_EQ(1, query.GetColumn(0).GetInt());
				EXPECT_STREQ("first", query.GetColumn(1).GetString());
			}
			EXPECT_EQ(1, nbRows);
		}
	}
}