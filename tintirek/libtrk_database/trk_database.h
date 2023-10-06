/*
 *	database.h
 *
 *	Interface for Tintirek's database
 */

#ifndef TRK_DATABASE_H
#define TRK_DATABASE_H

#include "sqlite3.h"
#include "trk_version.h"


 /* Library version definition */
TRK_VERSION_DEFINE(TrkDatabaseVerVar, "trk_database");


/* Initialization function for databases */
void InitDatabases(TrkString rootDir);

/* Get user from database */
TrkSqliteQueryResult GetUserFromDB(TrkString username, TrkString query = "*");


#endif /* TRK_DATABASE_H */