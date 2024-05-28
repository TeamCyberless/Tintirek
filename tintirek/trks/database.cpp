/*
 *	database.cpp
 *
 *	Database functions of Tintirek's system
 */

#include "database.h"

#include <chrono>


/* All databases */
TrkSqlite::TrkDatabase* userDB = nullptr;


/* Database schemes */
#define DB_USER_SCHEME std::string("CREATE TABLE IF NOT EXISTS user (" \
                                     "id INTEGER PRIMARY KEY AUTOINCREMENT," \
                                     "username TEXT UNIQUE NOT NULL," \
                                     "fullname TEXT NOT NULL," \
                                     "email TEXT UNIQUE NOT NULL," \
                                     "ticket TEXT NULL," \
                                     "ticket_end DATETIME NULL," \
                                     "password_hash TEXT NOT NULL," \
                                     "salt TEXT NOT NULL," \
                                     "iteration INTEGER NOT NULL DEFAULT 3," \
                                     "created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP," \
                                     "updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP" \
                                     ");")


void InitDatabases(std::string rootDir)
{
    userDB = new TrkSqlite::TrkDatabase(rootDir + "user.db", TrkSqlite::OPEN_READWRITE | TrkSqlite::OPEN_CREATE);
    userDB->Execute(DB_USER_SCHEME);
}

bool GetUserPasswdFromDB(std::string username, std::string& passwd, std::string& salt, int& iteration)
{
    TrkSqlite::TrkStatement Query(*userDB, "SELECT password_hash, salt, iteration FROM user WHERE username = ?");
    Query.Bind(1, username);

    try
    {
        Query.ExecuteStep();
        passwd = Query.GetColumn(0).GetString();
        salt = Query.GetColumn(1).GetString();
        iteration = Query.GetColumn(2);

        return true;
    }
    catch (TrkSqlite::TrkDatabaseException&) { }

    return false;
}

bool GetUserTicketFromDB(std::string username, std::string& ticket, int64_t& endtimeunix)
{
    TrkSqlite::TrkStatement Query(*userDB, "SELECT ticket, ticket_end FROM user WHERE username = ?");
    Query.Bind(1, username);

    try
    {
        Query.ExecuteStep();
        ticket = Query.GetColumn(0).GetString();
        endtimeunix = Query.GetColumn(1);

        return true;
    }
    catch (TrkSqlite::TrkDatabaseException&) {}

    return false;
}

bool ResetUserTicketDB(std::string username)
{
    TrkSqlite::TrkStatement Query(*userDB, "UPDATE user SET ticket = NULL, ticket_end = NULL WHERE username = ?");
    Query.Bind(1, username);

    try
    {
        return Query.Execute() > 0;
    }
    catch (TrkSqlite::TrkDatabaseException&) {}

    return false;
}

bool UpdateUserTicketDB(std::string username, std::string ticket)
{
    int64_t unix_ticket_end = static_cast<int64_t>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() + std::chrono::hours(24)));

    TrkSqlite::TrkStatement Query(*userDB, std::string("UPDATE user SET ticket_end = ?, ticket = ? WHERE username = ?"));
    Query.Bind(1, unix_ticket_end);
    Query.Bind(2, ticket);
    Query.Bind(3, username);

    try
    {
        return Query.Execute() > 0;
    }
    catch (TrkSqlite::TrkDatabaseException&) { }

    return false;
}