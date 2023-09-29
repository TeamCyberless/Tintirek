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

namespace fs = std::filesystem;


bool TrkCliLoginCommand::CallCommand_Implementation(const TrkCliOption* Options, TrkCliOptionResults* Results)
{
	TrkCliClientOptionResults* ClientResults = static_cast<TrkCliClientOptionResults*>(Results);

	if (Results->showsessionticket)
	{
		TrkString sessionInfoPath;

#ifdef WIN32
		{
			char userProfile[MAX_PATH];
			if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_PROFILE, nullptr, 0, userProfile)))
			{
				sessionInfoPath = userProfile;
			}
		}
#else
		// @TODO: Get User Directory in Linux/Unix Platform.
#endif
		sessionInfoPath << (char)fs::path::preferred_separator << ".trksession";

		std::ifstream sessionInfoFile(sessionInfoPath);
		if (sessionInfoFile.is_open())
		{
			std::string l;
			while (std::getline(sessionInfoFile, l))
			{
				TrkString line(l.c_str());
				size_t delimiterPos = line.find("::");
				if (delimiterPos != TrkString::npos)
				{
					const TrkString serverName = line.substr(0, delimiterPos);
					if (serverName == ClientResults->server_url)
					{
						const TrkString sessionInfo = line.substr(delimiterPos + 2);
						std::cout << sessionInfo;
						return true;
					}
				}
			}
		}
	}

	std::cout << "Enter Password: " << std::endl;

	char* passwd;
	std::cin >> passwd;
	//TrkString passwd(passwd);

	return false;
}

bool TrkCliLoginCommand::CheckCommandFlags_Implementation(const char Flag)
{
	return Flag == 's';
}
