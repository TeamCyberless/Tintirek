/*
 *	database.cpp
 *
 *	Database functions of Tintirek's Database Library
 */

#include "trk_database.h"


/* All databases */
TrkSqliteDB* userDB = nullptr;


/* Database schemes */
#define DB_USER_SCHEME TrkString("CREATE TABLE IF NOT EXISTS user (" \
                                     "id INTEGER PRIMARY KEY AUTOINCREMENT," \
                                     "username TEXT UNIQUE NOT NULL," \
                                     "fullname TEXT NOT NULL," \
                                     "email TEXT UNIQUE NOT NULL," \
                                     "ticket TEXT NULL," \
                                     "password_hash TEXT NOT NULL," \
                                     "salt TEXT NOT NULL," \
                                     "iteration INTEGER NOT NULL DEFAULT 3," \
                                     "created_at DATETIME DEFAULT CURRENT_TIMESTAMP," \
                                     "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP" \
                                     ");")


void InitDatabases(TrkString rootDir)
{
    userDB = new TrkSqliteDB(rootDir + "user.db");
    userDB->ExecuteNonQuery(DB_USER_SCHEME);
}

TrkSqliteQueryResult GetUserFromDB(TrkString username, TrkString query)
{
    TrkString sql = "SELECT ";
    sql << query << " FROM user WHERE username = ?";

    TrkSqliteStatement* statement = userDB->PrepareStatement(sql);
    statement->BindText(1, username);

    int result = statement->Step();
    if (result == SQLITE_ROW)
    {
        return TrkSqliteQueryResult(statement);
    }

    return TrkSqliteQueryResult(nullptr);
}