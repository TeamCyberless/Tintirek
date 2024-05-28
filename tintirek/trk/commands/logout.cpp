/*
 *	logout.cpp
 *
 *	Tintirek's logout command source file
 */


#include "logout.h"

#include <iostream>

#include "trk_version.h"
#include "connect.h"
#include "passwd.h"


bool TrkCliLogoutCommand::CallCommand_Implementation(const TrkCliOption* Options, TrkCliOptionResults* Results)
{
	TrkCliClientOptionResults* ClientResults = static_cast<TrkCliClientOptionResults*>(Results);

	if (TrkPasswdHelper::CheckSessionFileExists())
	{
		std::string ticket = TrkPasswdHelper::GetSessionTicketByServerURL(ClientResults->server_url);
		if (ticket.size() == 0)
		{
			std::cout << "Already logged out." << std::endl;
			return true;
		}
		else
		{
			std::string returned, errmsg;
			if (!TrkConnectHelper::SendCommand(*ClientResults, "Logout", errmsg, returned))
			{
				std::cerr << errmsg << std::endl;
				return true;
			}
			else
			{
				TrkPasswdHelper::DeleteSessionTicket(ClientResults->server_url);
			}
		}
	}

	std::cout << "User \"" << ClientResults->username << "\" logged out." << std::endl;
	return true;
}

bool TrkCliLogoutCommand::CheckCommandFlags_Implementation(const char Flag)
{
	return false;
}
