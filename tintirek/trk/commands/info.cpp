/*
 *	info.cpp
 *
 *	Tintirek's info command source file
 */


#include "info.h"

#include <iostream>
#include <filesystem>

#include "trk_version.h"
#include "connect.h"

namespace fs = std::filesystem;


bool TrkCliInfoCommand::CallCommand_Implementation(const TrkCliOption* Options, TrkCliOptionResults* Results)
{
	TrkCliClientOptionResults* ClientResults = static_cast<TrkCliClientOptionResults*>(Results);

	if (ClientResults)
	{
		std::string returned, errmsg;
		if (!TrkConnectHelper::SendCommand(*ClientResults, "GetInformation", errmsg, returned))
		{
			std::cerr << errmsg << std::endl;
			return true;
		}
		else
		{
			std::string data(returned);
			size_t pos = 0;
			size_t semicolonPos;

			if (data.back() != ';')
			{
				data += ';';
			}

			while ((semicolonPos = data.find(';', pos)) != std::string::npos)
			{
				std::string param = data.substr(pos, semicolonPos - pos);
				size_t equalSignPos = param.find('=');
				if (equalSignPos != std::string::npos)
				{
					std::string key = param.substr(0, equalSignPos);
					std::string value = param.substr(equalSignPos + 1);

					if (key == "servertime")
					{
						ClientResults->server_time.append(value);
					}
					else if (key == "serveruptime")
					{
						ClientResults->server_uptime.append(value);
					}
					else if (key == "serverversion")
					{
						ClientResults->server_version.append(value);
					}
				}
				pos = semicolonPos + 1;
			}
		}

		std::cout <<
			"User Name: " << ClientResults->username << std::endl <<
			"Workspace Directory: " << (ClientResults->workspace_path.empty() ? ClientResults->workspace_path : "unknown") << std::endl <<
			"Current Directory: " << fs::current_path().string() << std::endl <<
			"Client Version: " << TRK_VERSION << std::endl <<
			"Development Mode: " << (std::string(TRK_VER_TAG) == " (Development)" ? "Enabled" : "Disabled") << std::endl <<

			"Server URL: " << ClientResults->server_url << std::endl <<
			"Server Time: " << ClientResults->server_time << std::endl <<
			"Server Uptime: " << ClientResults->server_uptime << std::endl <<
			"Server Version: " << ClientResults->server_version << std::endl <<
			"Server Encryption: " << (ClientResults->trust ? "Enabled" : "Disabled") << std::endl;
	}

	return true;
}

bool TrkCliInfoCommand::CheckCommandFlags_Implementation(const char Flag)
{
	return false;
}
