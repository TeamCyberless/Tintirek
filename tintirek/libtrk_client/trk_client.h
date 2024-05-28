/*
 *	trk_client.h
 *
 *	General information about Tintirek's Client Library
 */


#ifndef TRK_CLIENT_H
#define TRK_CLIENT_H


#include "trk_version.h"


/* Library version definition */
inline const trk_version_t* trk_client_version(void)
{
    TRK_VERSION_BODY;
}


/* Get current user's directory */
class std::string GetCurrentUserDir();


#endif /* TRK_CLIENT_H */