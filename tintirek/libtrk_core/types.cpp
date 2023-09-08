/*
 *	types.cpp
 * 
 *	Tintirek's data type implementations
 */

#include "trk_types.h"

void TrkWorkspaceInfo::AppendToRevInfo(TrkPathSyncInfoDict* NewElement)
{
	if (rev_info_list == nullptr)
	{
		rev_info_list = NewElement;
		NewElement->SetNext(nullptr);
		return true;
	}

	TrkPathSyncInfoDict* current = rev_info_list;

	while (current->GetNext() != nullptr)
	{
		if (current->GetNext() == NewElement)
		{
			return false;
		}

		current = current->GetNext();
	}

	current->SetNext(NewElement);
	NewElement->SetNext(nullptr);
	return true;
}