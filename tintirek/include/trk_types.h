/*
 *	trk_types.h
 * 
 *	Tintirek's data types
 */

#ifndef TRK_TYPES_H
#define TRK_TYPES_H

#include <string>

#ifdef __cplusplus
extern "C" {
#endif


/* Tintirek's Boolean Type */
typedef int trk_boolean_t;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


/* Revision number. */
typedef long int trk_revision_number_t;

/* Commit number. */
typedef long long trk_commit_number_t;

/* invalid revision number. */
#define TRK_INVALID_REVNUM ((trk_revision_number_t)-1)

/* invalid commit number. */
#define TRK_INVALID_COMMITNUM ((trk_commit_number_t)-1)

/* UNIX type datetime. */
typedef long long trk_datetime_t;

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
typedef struct trk_pathsyncinfo_dictionary_t
{
	/* Next element of this dictionary */
	struct trk_pathsyncinfo_dictionary_t* next_elem;
	/* Key value of this element */
	const std::string key;
	/* Revision number */
	trk_revision_number_t value;
} trk_pathsyncinfo_dictionary_t;



/* Workspace info */
typedef struct trk_workspaceinfo_t
{
	/* Workspace name */
	const std::string workspace_name;
	/* Workspace path at owner computer */
	const std::string workspace_path;
	/* Workspace owner name */
	const std::string username;
	/* Workspace hostname */
	const std::string workspace_hostname;
	/* Workspace's current commit number */
	trk_commit_number_t current_commit_num;
	/* List of revision information about paths */
	struct trk_pathsyncinfo_dictionary_t* rev_info_list;
} trk_workspaceinfo_t;


/* Append path info about revision to list */
trk_boolean_t trk_workspaceinfo_appendpath(trk_workspaceinfo_t* workspace_info, trk_pathsyncinfo_dictionary_t* element);



/* All information about a commit. */
typedef struct trk_commitinfo_t
{
	/* The number of revision */
	trk_revision_number_t revision;
	/* Server-side date of the commit. */
	const std::string date;
	/* Author of the commit. */
	const std::string author;
} trk_commitinfo_t;



/* Lock object for client and server to share. */
typedef struct trk_lock_t
{
	/* the path this lock applies to. */
	const std::string path;
	/* unique URI representing lock. */
	const std::string token;
	/* the username which owns the lock. */
	const std::string owner;
	/* when lock was made. */
	trk_datetime_t creation_date;
} trk_lock_t;



#ifdef __cplusplus
}
#endif

#endif /* TRK_TYPES_H */