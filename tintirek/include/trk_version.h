/*
 *	trk_version.h
 *
 *	Version information of Tintirek
 */



#ifndef TRK_VERSION_H
#define TRK_VERSION_H


#include <iostream>

#include "trk_types.h"


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
 *	The TrkVersion class provides a way to store and manage
 *	major, minor, and patch version numbers, along with an
 *	optional version tag that provides additional descriptive
 *	information.
 */
class TrkVersion
{
public:
	TrkVersion(int major, int minor, int patch, const TrkString tag, const TrkString label)
		: major_(major)
		, minor_(minor)
		, patch_(patch)
		, tag_(tag)
		, label_(label)
	{ }

    /* Get the major version number */
    int getMajor() const { return major_; }

    /* Get the minor version number */
    int getMinor() const { return minor_; }

    /* Get the patch version number */
    int getPatch() const { return patch_; }

    /* Get the version tag */
    TrkString getTag() const { return tag_; }

	/* Get the version owner label */
	TrkString getLabel() const { return label_; }

	/* Return full version info */
	TrkString getFullVersionInfo() const;

	/* Check library version compatibility */
	static bool TrkVerCompatible(const TrkVersion* my_version, const TrkVersion* lib_version);

	/* Check if versions are the same */
	static bool TrkVerEqual(const TrkVersion* my_version, const TrkVersion* other_version);


	/* Override for "<<" operator */
	friend std::ostream& operator << (std::ostream& os, const TrkVersion& obj);


private:
	/* Major version number */
	int major_;
	/* Minor version number */
	int minor_;
	/* Patch number */
	int patch_;
	/* The version tag, such as " (Development)" or " (Release)" */
	TrkString tag_;
	/* The version owner, such as "program" or "trk_core" */
	TrkString label_;
};


/* Helper class for check version */
class TrkVersionHelper
{
public:
	static bool CheckVersionList(const TrkVersion* MyVersion, const TrkVersion** CheckList, bool UseEqual, TrkString& ErrorString);
};


/* Define a static TrkVersion object */
#define TRK_VERSION_DEFINE_PROGRAM(name) TRK_VERSION_DEFINE(name, "program")
#define TRK_VERSION_DEFINE(name, libname) \
static const TrkVersion name = \
{ \
	TRK_VER_MAJOR, \
	TRK_VER_MINOR, \
	TRK_VER_PATCH, \
	TRK_VER_TAG, \
	libname, \
}\


#endif /* TRK_VERSION_H */