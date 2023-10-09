/*
 *	database.cpp
 *
 *	Database functions of Tintirek's Database Library
 */

#include "trk_database.h"

#include <chrono>


/* All databases */
TrkSqlite::TrkDatabase* userDB = nullptr;


/* Database schemes */
#define DB_USER_SCHEME TrkString("CREATE TABLE IF NOT EXISTS user (" \
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


void InitDatabases(TrkString rootDir)
{
    userDB = new TrkSqlite::TrkDatabase(rootDir + "user.db", TrkSqlite::OPEN_READWRITE | TrkSqlite::OPEN_CREATE);
    userDB->Execute(DB_USER_SCHEME);
}

bool GetUserPasswdFromDB(TrkString username, TrkString& passwd, TrkString& salt, int& iteration)
{
    TrkSqlite::TrkStatement Query(*userDB, "SELECT password_hash, salt, iteration FROM user WHERE username = ?");
    Query.Bind(1, username);

    try
    {
        Query.ExecuteStep();
        passwd = Query.GetColumn(0);
        salt = Query.GetColumn(1);
        iteration = Query.GetColumn(2);
        Query.Reset();

        return true;
    }
    catch (TrkSqlite::TrkDatabaseException&) { }

    return false;
}

bool GetUserTicketFromDB(TrkString username, TrkString& ticket, int64_t& endtimeunix)
{
    TrkSqlite::TrkStatement Query(*userDB, "SELECT ticket, ticket_end FROM user WHERE username = ?");
    Query.Bind(1, username);

    try
    {
        Query.ExecuteStep();
        ticket = Query.GetColumn(0);
        endtimeunix = Query.GetColumn(1);
        Query.Reset();

        return true;
    }
    catch (TrkSqlite::TrkDatabaseException&) {}

    return false;
}

bool UpdateUserTicketDB(TrkString username, TrkString ticket)
{
    int64_t unix_ticket_end = static_cast<int64_t>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() + std::chrono::hours(24)));

    try
    {
        TrkSqlite::TrkStatement Query(*userDB, TrkString("UPDATE user SET ticket_end = ?, ticket = ? WHERE username = ?"));
        Query.Bind(1, unix_ticket_end);
        Query.Bind(2, ticket);
        Query.Bind(3, username);

        return Query.Execute() > 0;
        Query.Reset();
    }
    catch (TrkSqlite::TrkDatabaseException& ex) 
    {
        std::cout << ex.what() << " (code: " << ex.GetErrorCode() << ", extended: " << ex.GetExtendedErrorCode() << ")" << std::endl;
    }

    return false;
}