/*
 * cmdline.cpp
 *
 * Helpers for cmd-line programs.
 */


#include "cmdline.h"
#include <sstream>


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

std::string TrkCliOptionFlag::Display(TrkCliOption* opt)
{
    std::stringstream str;

    for (const auto* flag : *opt->head_flag)
    {
        if (flag->required_parameter)
        {
            str << "[-" << flag->character << " <" << flag->parameter_name << ">] ";
        }
        else
        {
            str << "[-" << flag->character << "] ";
        }
    }

    return str.str();
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

std::string TrkCliOption::GetFlagsForHelpText()
{
    return TrkCliOptionFlag::Display(this);
}
