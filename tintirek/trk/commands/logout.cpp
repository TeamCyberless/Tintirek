/*
 *	logout.cpp
 *
 *	Tintirek's logout command source file
 */


#include "logout.h"

#include <iostream>

#include "trk_version.h"


bool TrkCliLogoutCommand::CallCommand_Implementation(const TrkCliOption* Options, TrkCliOptionResults* Results)
{
	return false;
}

bool TrkCliLogoutCommand::CheckCommandFlags_Implementation(const char Flag)
{
	return Flag == 's';
}
