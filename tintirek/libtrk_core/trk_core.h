/*
 *	trk_core.h
 * 
 *	General information about Tintirek's Core Library
 */


#ifndef TRK_CORE_H
#define TRK_CORE_H


#include "trk_version.h"


/* Library version definition */
inline const trk_version_t* trk_core_version(void)
{
    TRK_VERSION_BODY;
}


#endif /* TRK_CORE_H */