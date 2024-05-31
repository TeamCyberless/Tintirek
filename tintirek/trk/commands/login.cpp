/*
 *	login.cpp
 *
 *	Tintirek's login command source file
 */


#include "login.h"

#include <iostream>

#include "trk_version.h"
#include "connect.h"
#include "passwd.h"

bool TrkCliLoginCommand::CallCommand_Implementation(const TrkCliOption* Options, TrkCliOptionResults* Results)
{
	TrkCliClientOptionResults* ClientResults = static_cast<TrkCliClientOptionResults*>(Results);

	if (TrkPasswdHelper::CheckSessionFileExists())
	{
		std::string ticket = TrkPasswdHelper::GetSessionTicketByServerURL(ClientResults->server_url);
		if (ticket.size() > 0)
		{
			if (Results->showsessionticket)
			{
				std::cout << ticket << std::endl;
				return true;
			}
			
			std::cout << "Already logged in." << std::endl;
			return true;
		}
	}
	
	if (ClientResults->command_parameter != "")
	{
		ClientResults->username = ClientResults->command_parameter;
	}

	std::string returned, errmsg;
	if (!TrkConnectHelper::SendCommand(*ClientResults, "Close", errmsg, returned))
	{
		std::cerr << errmsg << std::endl;
		return true;
	}

	if (Results->showsessionticket)
	{
		std::string ticket = TrkPasswdHelper::GetSessionTicketByServerURL(ClientResults->server_url);
		std::cout << ticket << std::endl;
	}
	else
	{
		std::cout << "User \"" << ClientResults->username << "\" logged in." << std::endl;
	}
	
	return true;
}

bool TrkCliLoginCommand::CheckCommandFlags_Implementation(const char Flag)
{
	return Flag == 's';
}
