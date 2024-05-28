/*
 *	version.c
 * 
 *	Library version number and utilities
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sstream>

#include "trk_version.h"
#include "trk_types.h"


std::string trk_get_full_version_info(const trk_version_t* version)
{
	char major[5], minor[5], patch[5];
	sprintf(major, "%d", version->major);
	sprintf(minor, "%d", version->minor);
	sprintf(patch, "%d", version->patch);

	std::stringstream ss;
	ss << major << "." << minor << "." << patch << version->tag;

	return ss.str();
}

trk_boolean_t trk_version_compatible(const trk_version_t* my_version, const trk_version_t* lib_version)
{
	// @TODO: Maybe we can do full version match checking system?

	return my_version->major == lib_version->major
		&& my_version->minor <= lib_version->minor;
}

trk_boolean_t trk_version_equal(const trk_version_t* my_version, const trk_version_t* other_version)
{
	return my_version->major == other_version->major
		&& my_version->minor == other_version->minor
		&& my_version->patch == other_version->patch
		&& my_version->tag == other_version->tag;
}

trk_boolean_t trk_version_check_list(const trk_version_t* my_version, const trk_version_checklist_t* check_list, trk_boolean_t use_equal, std::string& error_str)
{
    for (int i = 0; check_list[i].label != NULL; i++)
	{
		const trk_version_t* current_check = check_list[i].version_query();
		if (use_equal ? 
			!trk_version_equal(my_version, current_check) :
			!trk_version_compatible(my_version, current_check))
		{
			std::stringstream ss;
			ss << "Version mismatch in " << check_list[i].label
				<< " \"" << trk_get_full_version_info(current_check)
				<< " \", must be" << (use_equal ? " or greater than" : "")
				<< " \"" << trk_get_full_version_info(my_version)
				<< " \"";
			error_str = ss.str();

			return FALSE;
		}
	}

	return TRUE;
}
