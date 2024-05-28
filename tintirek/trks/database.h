/*
 *	database.h
 *
 *	Interface for Tintirek's database system
 */

#ifndef TRK_DATABASE_H
#define TRK_DATABASE_H

#include "sqlite3.h"


/* Initialization function for databases */
void InitDatabases(std::string rootDir);

/* Get user password information from database */
bool GetUserPasswdFromDB(std::string username, std::string& passwd, std::string& salt, int& iteration);

/* Get user ticket information from database */
bool GetUserTicketFromDB(std::string username, std::string& ticket, int64_t& endtimeunix);

/* Resets user ticket in database */
bool ResetUserTicketDB(std::string username);

/* Update user ticket also with ticket time */
bool UpdateUserTicketDB(std::string username, std::string ticket);


#endif /* TRK_DATABASE_H */