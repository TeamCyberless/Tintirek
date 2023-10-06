/*
 *	trk_client.h
 *
 *	General information about Tintirek's Client Library
 */


#ifndef TRK_CLIENT_H
#define TRK_CLIENT_H


#include "trk_version.h"


/* Library version definition */
TRK_VERSION_DEFINE(TrkClientVerVar, "trk_client");


/* Get current user's directory */
TrkString GetCurrentUserDir();


#endif /* TRK_CLIENT_H */