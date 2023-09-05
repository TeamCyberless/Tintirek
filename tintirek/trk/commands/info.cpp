/*
 *	info.cpp
 *
 *	Tintirek's info command source file
 */


#include "info.h"

#include <iostream>
#include <filesystem>

#include "trk_version.h"

namespace fs = std::filesystem;


bool TrkCliInfoCommand::CallCommand_Implementation(const TrkCliOption* Options, TrkCliOptionResults* Results)
{
	TrkCliClientOptionResults* ClientResults = dynamic_cast<TrkCliClientOptionResults*>(Results);
	if (ClientResults)
	{
		std::cout <<
			"User Name: " << ClientResults->username << std::endl <<
			"Workspace Directory: " << (ClientResults->workspace_path ? ClientResults->workspace_path : "unknown") << std::endl <<
			"Current Directory: " << fs::current_path().string() << std::endl <<
			"Client Version: " << TRK_VERSION << std::endl <<
			"Development Mode: " << (std::string(TRK_VER_TAG) == " (Development)" ? "Enabled" : "Disabled") << std::endl <<

			"Server URL: " << ClientResults->server_url << std::endl <<
			"Server Time: " << ClientResults->server_time << std::endl <<
			"Server Uptime: " << ClientResults->server_uptime << std::endl <<
			"Server Version: " << ClientResults->server_version << std::endl <<
			"Server Encryption: " << (ClientResults->trust ? "Enabled" : "Disabled") << std::endl;

		return true;
	}

	return false;
}

bool TrkCliInfoCommand::CheckCommandFlags_Implementation(const char Flag)
{
	return false;
}
