/*
 *	database.h
 *
 *	Interface for Tintirek's database system
 */

#ifndef TRK_DATABASE_H
#define TRK_DATABASE_H

#include "sqlite3.h"


/* Initialization function for databases */
void InitDatabases(TrkString rootDir);

/* Get user password information from database */
bool GetUserPasswdFromDB(TrkString username, TrkString& passwd, TrkString& salt, int& iteration);

/* Get user ticket information from database */
bool GetUserTicketFromDB(TrkString username, TrkString& ticket, int64_t& endtimeunix);

/* Resets user ticket in database */
bool ResetUserTicketDB(TrkString username);

/* Update user ticket also with ticket time */
bool UpdateUserTicketDB(TrkString username, TrkString ticket);


#endif /* TRK_DATABASE_H */