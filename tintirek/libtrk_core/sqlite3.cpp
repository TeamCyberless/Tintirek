/*
 *	sqlite3.cpp
 *
 *	Tintirek's SQLite integration
 */


#include "sqlite3.h"


TrkString TrkSqliteWrapper::GetCompiledVersion() const
{
	return TrkString(strdup(SQLITE_VERSION));
}

TrkString TrkSqliteWrapper::GetRuntimeVersion() const
{
	return TrkString(strdup(sqlite3_libversion()));
}
