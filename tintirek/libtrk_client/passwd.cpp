/*
 *	passwd.cpp
 *
 *	Client's password functions
 */

#include "passwd.h"
#include "trk_client.h"

#include <iostream>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;


bool TrkPasswdHelper::CheckSessionFileExists()
{
	std::string HomeDir = GetCurrentUserDir();

	try
	{
		fs::path SessionFile = fs::canonical(fs::path(HomeDir) / ".trksession");

		if (fs::exists(SessionFile) && fs::is_regular_file(SessionFile))
		{
			return true;
		}

		return false;
	}
	catch (std::filesystem::filesystem_error& ex)
	{
		if (!fs::exists(ex.path1()))
		{
			std::fstream SessionFileCreater(ex.path1(), std::ios::out);
			if (!SessionFileCreater)
			{
				std::cout << "Error for creating session file" << std::endl;
				return false;
			}

			SessionFileCreater.close();
			return true;
		}
	}
	catch (std::exception ex)
	{
		// No need to anything here
	}

	return false;
}

bool TrkPasswdHelper::SaveSessionTicket(std::string Ticket, std::string ServerUrl)
{
	std::string HomeDir = GetCurrentUserDir();

	try
	{
		fs::path SessionFile = fs::canonical(fs::path(HomeDir) / ".trksession");

		if (fs::exists(SessionFile) && fs::is_regular_file(SessionFile))
		{
			std::ofstream SessionFileWriter(SessionFile, std::ios::app);
			if (SessionFileWriter.is_open())
			{
				SessionFileWriter << ServerUrl << "=" << Ticket << std::endl;
				SessionFileWriter.close();
				return true;
			}
			else
			{
				throw std::exception();
			}
		}
	}
	catch (std::filesystem::filesystem_error& ex)
	{
		if (!fs::exists(ex.path1()))
		{
			std::fstream SessionFileCreater(ex.path1(), std::ios::out);
			if (!SessionFileCreater)
			{
				std::cout << "Error for creating session file" << std::endl;
				return false;
			}

			SessionFileCreater.close();
			return SaveSessionTicket(Ticket, ServerUrl);
		}
	}
	catch (std::exception&)
	{
		// No need to anything here
	}

	return false;
}

bool TrkPasswdHelper::DeleteSessionTicket(std::string ServerUrl)
{
	std::string HomeDir = GetCurrentUserDir();

	try
	{
		fs::path SessionFile = fs::canonical(fs::path(HomeDir) / ".trksession");

		if (fs::exists(SessionFile) && fs::is_regular_file(SessionFile))
		{
			std::ifstream input(SessionFile);
			std::ofstream output(fs::temp_directory_path() / "trktempfile.txt");

			if (!input || !output)
			{
				throw std::exception();
			}

			std::string l;
			while (std::getline(input, l))
			{
				const std::string line(l.c_str());
				if (line.find(ServerUrl + "=") != 0)
				{
					output << line << std::endl;
				}
			}

			input.close();
			output.close();
			fs::rename(fs::temp_directory_path() / "trktempfile.txt", SessionFile);

			return true;
		}
	}
	catch (std::filesystem::filesystem_error& ex)
	{
		if (!fs::exists(ex.path1()))
		{
			std::fstream SessionFileCreater(ex.path1(), std::ios::out);
			if (!SessionFileCreater)
			{
				std::cout << "Error for creating session file" << std::endl;
				return false;
			}

			SessionFileCreater.close();
			return true; // File created. no need to change anything.
		}
	}
	catch (std::exception& ex)
	{
		// No need to anything here
	}

	return false;
}

bool TrkPasswdHelper::ChangeSessionTicket(std::string Ticket, std::string ServerUrl)
{
	std::string HomeDir = GetCurrentUserDir();

	try
	{
		fs::path SessionFile = fs::canonical(fs::path(HomeDir) / ".trksession");

		if (fs::exists(SessionFile) && fs::is_regular_file(SessionFile))
		{
			std::ifstream input(SessionFile);
			std::ofstream output(fs::temp_directory_path() / "trktempfile.txt");

			if (!input || !output)
			{
				throw std::exception();
			}

			std::string l;
			while (std::getline(input, l))
			{
				const std::string line(l.c_str());
				if (line.find(ServerUrl + "=") == 0)
				{
					output << ServerUrl << "=" << Ticket << std::endl;
				}
				else
				{
					output << line << std::endl;
				}
			}

			input.close();
			output.close();
			fs::rename(fs::temp_directory_path() / "trktempfile.txt", SessionFile);

			return true;
		}
	}
	catch (std::filesystem::filesystem_error& ex)
	{
		if (!fs::exists(ex.path1()))
		{
			std::fstream SessionFileCreater(ex.path1(), std::ios::out);
			if (!SessionFileCreater)
			{
				std::cout << "Error for creating session file" << std::endl;
				return false;
			}

			SessionFileCreater.close();
			return true; // File created. no need to change anything.
		}
	}
	catch (std::exception& ex)
	{
		// No need to anything here
	}

	return false;
}

std::string TrkPasswdHelper::GetSessionTicketByServerURL(std::string ServerUrl)
{
	std::string HomeDir = GetCurrentUserDir();

	try
	{
		fs::path SessionFile = fs::canonical(fs::path(HomeDir) / ".trksession");

		if (fs::exists(SessionFile) && fs::is_regular_file(SessionFile))
		{
			std::ifstream SessionFileReader(SessionFile);
			if (SessionFileReader.is_open())
			{
				std::string l;
				while (std::getline(SessionFileReader, l))
				{
					const std::string line(l.c_str());
					size_t delimiterPos = line.find("=");
					if (delimiterPos != std::string::npos)
					{
						const std::string key = line.substr(0, delimiterPos);
						const std::string value = line.substr(delimiterPos + 1);

						if (key == ServerUrl)
						{
							SessionFileReader.close();
							return value;
						}
					}
				}

				SessionFileReader.close();
			}
		}
	}
	catch (std::exception ex)
	{
		// No need to anything here
	}

	return "";
}