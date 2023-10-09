/*
 *	login.cpp
 *
 *	Tintirek's login command source file
 */


#include "login.h"

#include <iostream>
#include <filesystem>
#include <fstream>

#ifdef WIN32
#include <Shlobj.h>
#else
// @TODO: Linux Includes
#endif

#include "trk_version.h"
#include "connect.h"
#include "passwd.h"

namespace fs = std::filesystem;


bool TrkCliLoginCommand::CallCommand_Implementation(const TrkCliOption* Options, TrkCliOptionResults* Results)
{
	TrkCliClientOptionResults* ClientResults = static_cast<TrkCliClientOptionResults*>(Results);

	if (TrkPasswdHelper::CheckSessionFileExists())
	{
		TrkString ticket = TrkPasswdHelper::GetSessionTicketByServerURL(ClientResults->server_url);
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

	TrkString returned, errmsg;
	if (!TrkConnectHelper::SendCommand(*ClientResults, "Close", errmsg, returned))
	{
		std::cerr << errmsg << std::endl;
		return true;
	}

	std::cout << "Login successful." << std::endl;
	return true;
}

bool TrkCliLoginCommand::CheckCommandFlags_Implementation(const char Flag)
{
	return Flag == 's';
}
