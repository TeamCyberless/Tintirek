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
}