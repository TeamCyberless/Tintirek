/*
 *	types.c
 * 
 *	Tintirek's data type implementations
 */

#include <stdio.h>

#include "trk_types.h"

trk_boolean_t trk_workspaceinfo_appendpath(trk_workspaceinfo_t* workspace_info, trk_pathsyncinfo_dictionary_t* element)
{
	if (workspace_info->rev_info_list == NULL)
	{
		workspace_info->rev_info_list = element;
		element->next_elem = NULL;
		return TRUE;
	}

	trk_pathsyncinfo_dictionary_t* current = workspace_info->rev_info_list;
	while (current->next_elem != NULL)
	{
		if (current->next_elem == element)
		{
			return FALSE;
		}

		current = current->next_elem;
	}

	current->next_elem = element;
	element->next_elem = NULL;
	return TRUE;
}