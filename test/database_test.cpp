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
	TEST(Exception, Copy)
	{
		MemoryLeakDetector leakDetector;

		{
			/*TrkString message = "error message";
			const TrkSqlite::TrkDatabaseException ex1(message, 1);
			TrkSqlite::TrkDatabaseException ex2("another error message", 2);

			ex2 = ex1;

			EXPECT_STREQ(ex2.what(), message);
			EXPECT_EQ(ex2.GetErrorCode(), 1);
			EXPECT_EQ(ex2.GetExtendedErrorCode(), -1);*/

			const TrkSqlite::TrkDatabaseException ex1("error message", 1);
			const TrkSqlite::TrkDatabaseException ex2 = ex1;

			EXPECT_STREQ(ex2.what(), ex1.what());
			EXPECT_EQ(ex2.GetErrorCode(), ex1.GetErrorCode());
			EXPECT_EQ(ex2.GetExtendedErrorCode(), ex1.GetExtendedErrorCode());
		}
	}

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

			EXPECT_THROW(db.Execute("INSERT INTO test VALUES (NULL, \"first\",  3)"), TrkSqlite::TrkDatabaseException);
			EXPECT_EQ(TrkSqlite::CODE_ERROR, db.GetErrorCode());
			EXPECT_EQ(TrkSqlite::CODE_ERROR, db.GetExtendedErrorCode());

			db.Execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT, weight INTEGER)");
			EXPECT_EQ(TrkSqlite::OK, db.GetErrorCode());
			EXPECT_EQ(TrkSqlite::OK, db.GetExtendedErrorCode());

			EXPECT_THROW(db.Execute("INSERT INTO test VALUES (NULL,  3)"), TrkSqlite::TrkDatabaseException);
			EXPECT_EQ(TrkSqlite::CODE_ERROR, db.GetErrorCode());
			EXPECT_EQ(TrkSqlite::CODE_ERROR, db.GetExtendedErrorCode());

			EXPECT_EQ(1, db.Execute("INSERT INTO test VALUES (NULL, \"first\",  3)"));

			EXPECT_THROW(db.Execute("INSERT INTO test VALUES (NULL, \"first\", 123, 0.123)"), TrkSqlite::TrkDatabaseException);
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
}