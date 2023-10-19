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

			EXPECT_FALSE(db.TableExists("false"));
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
}