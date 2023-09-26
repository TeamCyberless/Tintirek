/*
 * cmdline.cpp
 *
 * Helpers for cmd-line programs.
 */


#include <vector>

#include "trk_cmdline.h" 



/* TrkCliCommand functions */

bool TrkCliCommand::CallCommand(const TrkCliOption* Options, TrkCliOptionResults* Results)
{
	return CallCommand_Implementation(Options, Results);
}

bool TrkCliCommand::CheckCommandFlags(const char Flag)
{
	return CheckCommandFlags_Implementation(Flag);
}

/* TrkCliOptionFlag functions */

TrkString TrkCliOptionFlag::Display(TrkCliOption* opt)
{
	TrkString str;

	for (const auto* flag : *opt->head_flag)
	{
		if (flag->required_parameter)
		{
			str << "[-" << TrkString(flag->character) << " <" << flag->parameter_name << ">] ";
		}
		else
		{
			str << "[-" << TrkString(flag->character) << "] ";
		}
	}

	return str;
}

TrkCliOptionFlag* TrkCliOptionFlag::CreateList(TrkCliOptionFlag* list, size_t size)
{
	if (size <= 0)
	{
		return nullptr;
	}

	TrkCliOptionFlag* head = new TrkCliOptionFlag(*(list));
	head->next_flag = nullptr;
	
	TrkCliOptionFlag* current = head;

	for (int i = 1; i < size; ++i) {
		TrkCliOptionFlag* newNode = new TrkCliOptionFlag(*(list + i));
		newNode->next_flag = nullptr;

		current->next_flag = newNode;
		current = newNode;
	}

	return head;
}

/* TrkCliOption functions */

TrkString TrkCliOption::GetFlagsForHelpText()
{
	return TrkCliOptionFlag::Display(this);
}