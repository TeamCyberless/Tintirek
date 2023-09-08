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

/* Commit number. */
typedef long long trk_commit_number_t;

/* invalid revision number. */
#define TRK_INVALID_REVNUM ((trk_revision_number_t)-1)

/* invalid commit number. */
#define TRK_INVALID_COMMITNUM ((trk_commit_number_t)-1)

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



/* Stores information to which revision number the given file/folder path is synchronized */
class TrkPathSyncInfoDict
{
public:
	TrkPathSyncInfoDict(TrkString Key = "", trk_revision_number_t RevNum = TRK_INVALID_REVNUM, TrkPathSyncInfoDict* NextElement = nullptr)
		: next_elem(NextElement)
		, key(Key)
		, value(RevNum)
	{ }

protected:
	/* Next element of this dictionary */
	TrkPathSyncInfoDict* next_elem;
	/* Key value of this element */
	TrkString key;
	/* Revision number */
	trk_revision_number_t value;

public:
	/* Get next element */
	TrkPathSyncInfoDict* GetNext() const { return next_elem; }
	/* Set next element */
	void SetNext(TrkPathSyncInfoDict* NewElement) { next_elem = NewElement; }

};



/* Workspace info */
class TrkWorkspaceInfo
{
public:
	/* Workspace name */
	TrkString workspace_name;

	/* Workspace path at owner computer */
	TrkString workspace_path;

	/* Workspace owner name */
	TrkString username;

	/* Workspace hostname */
	TrkString workspace_hostname;

	/* Workspace's current commit number */
	trk_commit_number_t current_commit_num;

	/* List of revision information about paths */
	TrkPathSyncInfoDict rev_info_list;

	/* Append path info about revision to list */
	void AppendToRevInfo(TrkPathSyncInfoDict* NewElement);
};



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