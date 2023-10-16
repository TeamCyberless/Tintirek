/*
 *	trk_cpp.h
 *
 *	Wrapper interface for Tintirek's core (C++)
 */

#ifndef TRK_CPP_H
#define TRK_CPP_H

#include "trk_version.h"


 /* Library version definition */
inline const trk_version_t* trk_cpp_version(void)
{
    TRK_VERSION_BODY;
}


#endif /* TRK_CPP_H */
