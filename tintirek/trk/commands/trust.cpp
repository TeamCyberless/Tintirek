/*
 *	trust.cpp
 *
 *	Tintirek's trust command source file
 */


#include "trust.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#if _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

#include "trk_client.h"
#include "trk_version.h"
#include "connect.h"

namespace fs = std::filesystem;


bool TrkCliTrustCommand::CallCommand_Implementation(const TrkCliOption* Options, TrkCliOptionResults* Results)
{
	TrkCliClientOptionResults* ClientResults = static_cast<TrkCliClientOptionResults*>(Results);

	if (ClientResults)
	{
		if (!ClientResults->trust)
		{
			std::cout << "No trust command needed for this server ('"<< ClientResults->server_url <<"').\nIf you think it's necessary, try again with 'tls:' prefix.";
			return true;
		}

		TrkString returned, errmsg;
		if (!TrkConnectHelper::SendCommand(*ClientResults, "GetInformation", errmsg, returned))
		{
			std::cout << errmsg;

			if (ClientResults->last_certificate_fingerprint == "")
			{
				return true;
			}

			char choice;
			std::cin >> choice;

			if (choice != 'y' && choice != 'Y')
			{
				return true;
			}

			TrkString HomeDir = GetCurrentUserDir();

			try
			{
				namespace fs = std::filesystem;
				fs::path TrustFile = fs::path(fs::path((const char*)HomeDir) / ".trktrust");

				if (fs::exists(TrustFile) && fs::is_regular_file(TrustFile))
				{
					std::ofstream TrustFileStream(TrustFile, std::ios::app);
					TrustFileStream << ClientResults->last_certificate_fingerprint << std::endl;
					TrustFileStream.close();
				}
				else if (!fs::exists(TrustFile))
				{
					std::ofstream TrustFileStream(TrustFile);
					TrustFileStream << ClientResults->last_certificate_fingerprint << std::endl;
					TrustFileStream.close();
				}

				std::cout << "Trust has been established for this server. ('" << ClientResults->server_url << "')";
				return true;
			}
			catch (std::exception ex)
			{
				std::cerr << "Something went wrong. Check '.trktrust' file is valid and accessible.";
				return true;
			}
		}
		else
		{
			std::cerr << "Something went wrong. Check '.trktrust' file is valid and accessible.";
			return true;
		}
	}

	return true;
}

bool TrkCliTrustCommand::CheckCommandFlags_Implementation(const char Flag)
{
	return false;
}
