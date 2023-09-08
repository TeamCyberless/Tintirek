/*
 *	trk_types.h
 * 
 *	Tintirek's data types
 */



#ifndef TRK_TYPES_H
#define TRK_TYPES_H

#include <cstdint>
#include <cstddef>

#include "trk_string.h"


/* Revision number. */
typedef long int trk_revision_number_t;

/* invalid revision number. */
#define TRK_INVALID_REVNUM ((trk_revision_number_t)-1)

/* UNIX type datetime. */
typedef int64_t trk_datetime_t;

/* Forward declaration of the 'trk_version_t' structure. */
typedef struct trk_version_t trk_version_t;



/*
 *	Keyword substitution 
 * 
 *	Definitions for various version-related keywords that Tintirek recognizes.
 */

/* The keyword for revision information. */
#define TRK_KEYWORD_REVISION "LastChangedRevision"

/* The keyword for date information. */
#define TRK_KEYWORD_DATE "LastChangedDate"

/* The keyword for author of the most recent commit. */
#define TRK_KEYWORD_AUTHOR "LastChangedBy"

/* The URL for revision path. */
#define TRK_KEYWORD_URL "HeadURL"

/* The keyword for ID generated from a combination of the other four keywords. */
#define TRK_KEYWORD_ID "Id"



/* All information about a commit. */
class TrkCommitInfo
{
public:
	/* The number of revision */
	trk_revision_number_t revision;

	/* Server-side date of the commit. */
	TrkString date;

	/* Author of the commit. */
	TrkString author;
};



/* Lock object for client and server to share. */
class TrkLock
{
public:
	/* the path this lock applies to. */
	TrkString path;

	/* unique URI representing lock. */
	TrkString token;

	/* the username which owns the lock. */
	TrkString owner;

	/* when lock was made. */
	trk_datetime_t creation_date;
};



#endif /* TRK_TYPES_H */