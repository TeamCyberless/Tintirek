/*
 *	version.c
 * 
 *	Library version number and utilities
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "trk_version.h"
#include "trk_types.h"


trk_string_t trk_get_full_version_info(const trk_version_t* version)
{
	char major[5], minor[5], patch[5];
	sprintf(major, "%d", version->major);
	sprintf(minor, "%d", version->minor);
	sprintf(patch, "%d", version->patch);

	trk_string_t ss;
	trk_string_append_cstr(&ss, trk_string_create(major));
	trk_string_append_cstr(&ss, trk_string_create("."));
	trk_string_append_cstr(&ss, trk_string_create(minor));
	trk_string_append_cstr(&ss, trk_string_create("."));
	trk_string_append_cstr(&ss, trk_string_create(patch));
	trk_string_append_cstr(&ss, version->tag);

	return ss;
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
		&& 0 == strcmp(my_version->tag.data, other_version->tag.data);
}

trk_boolean_t trk_version_check_list(const trk_version_t* my_version, const trk_version_checklist_t* check_list, trk_boolean_t use_equal, trk_string_t* error_str)
{
    for (int i = 0; check_list[i].label != NULL; i++)
	{
		const trk_version_t* current_check = check_list[i].version_query();
		if (use_equal ? 
			!trk_version_equal(my_version, current_check) :
			!trk_version_compatible(my_version, current_check))
		{
			trk_string_destroy(error_str);
			error_str = (trk_string_t*)malloc(sizeof(trk_string_t));
			if (error_str != NULL)
			{
				error_str->data = "";
				error_str->length = 0;
			}
			
            trk_string_append_cstr(error_str, trk_string_create("Version mismatch in "));
            trk_string_append_cstr(error_str, trk_string_create(check_list[i].label));
			trk_string_append_cstr(error_str, trk_string_create(" \""));
			trk_string_append_cstr(error_str, trk_get_full_version_info(current_check));
			trk_string_append_cstr(error_str, trk_string_create("\", must be"));
			if (use_equal) trk_string_append_cstr(error_str, trk_string_create(" or greater than"));
			trk_string_append_cstr(error_str, trk_string_create(" \""));
			trk_string_append_cstr(error_str, trk_get_full_version_info(my_version));
			trk_string_append_cstr(error_str, trk_string_create("\""));

			return FALSE;
		}
	}

	return TRUE;
}
