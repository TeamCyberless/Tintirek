/*
 *	trk_version.h
 *
 *	Version information of Tintirek
 */


#ifndef TRK_VERSION_H
#define TRK_VERSION_H


#include "trk_types.h"
#include "trk_string.h"


#ifdef __cplusplus
extern "C" {
#endif


	/* Macro to convert an identifier into a string literal */
#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)

/* Major version number */
#define TRK_VER_MAJOR 1

/* Minor version number */
#define TRK_VER_MINOR 0

/* Patch number */
#define TRK_VER_PATCH 0

/*
 *	Version Branch Tag
 *
 *	A descriptive string indicating the source branch
 *	from which the software originated. The presence of
 *	this tag as " (Development)" within the repository
 *	ensures that we can readily identify that the software
 *	is compiled from the source code.
 */
#define TRK_VER_TAG " (Development)"

 /* Version number */
#define TRK_VER_NUM STRINGIFY(TRK_VER_MAJOR) \
					"." STRINGIFY(TRK_VER_MINOR) \
					"." STRINGIFY(TRK_VER_PATCH)

/* Complete version string */
#define TRK_VERSION TRK_VER_NUM TRK_VER_TAG



/*
 *	Version information
 *
 *	Represents version information for the Tintirek.
 *
 *	The trk_version_t class provides a way to store and manage
 *	major, minor, and patch version numbers, along with an
 *	optional version tag that provides additional descriptive
 *	information.
 */
struct trk_version_t
{
	/* Major version number */
	int major;
	/* Minor version number */
	int minor;
	/* Patch number */
	int patch;
	/* The version tag, such as " (Development)" or " (Release)" */
	struct trk_string_t tag;
};

/*
 *	An entry in the compatibility checklist
 */
typedef struct trk_version_checklist_t
{
	/* The version owner, such as "program" or "trk_core" */
    const char* label;
	/* Version query function for this entry */
	const trk_version_t *(*version_query)(void);
} trk_version_checklist_t;

/* Return full version info */
trk_string_t trk_get_full_version_info(const trk_version_t* version);

/* Check library version compatibility */
trk_boolean_t trk_version_compatible(const trk_version_t* my_version, const trk_version_t* lib_version);

/* Check if versions are the same */
trk_boolean_t trk_version_equal(const trk_version_t* my_version, const trk_version_t* other_version);

/* Helper function for check version */
trk_boolean_t trk_version_check_list(const trk_version_t* my_version, const trk_version_checklist_t* check_list, trk_boolean_t use_equal, trk_string_t* error_str);


/* Define a static trk_version_t object */
#define TRK_VERSION_DEFINE(name) 	\
static const trk_version_t name = 	\
{ 									\
	TRK_VER_MAJOR, 					\
	TRK_VER_MINOR, 					\
	TRK_VER_PATCH, 					\
	TRK_VER_TAG, 					\
}

/* Generate the implementation of a version query function */
#define TRK_VERSION_BODY 			\
static struct versioninfo_t			\
{									\
	const char* const str;			\
	const trk_version_t num;		\
} const versioninfo =				\
{									\
	"@(#)" TRK_VERSION,				\
	{								\
		TRK_VER_MAJOR,				\
		TRK_VER_MINOR,				\
		TRK_VER_PATCH				\
	}								\
};									\
return &versioninfo.num


#ifdef __cplusplus
}
#endif

#endif /* TRK_VERSION_H */
