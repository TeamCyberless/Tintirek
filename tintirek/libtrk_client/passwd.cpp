/*
 *	passwd.cpp
 *
 *	Client's password functions
 */

#include "passwd.h"

#include <filesystem>
#include <fstream>

#include "trk_client.h"

namespace fs = std::filesystem;


bool TrkPasswdHelper::CheckSessionFileExists()
{
	TrkString HomeDir = GetCurrentUserDir();

	try
	{
		fs::path SessionFile = fs::canonical(fs::path((const char*)HomeDir) / ".trksession");

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

bool TrkPasswdHelper::SaveSessionTicket(TrkString Ticket, TrkString ServerUrl)
{
	TrkString HomeDir = GetCurrentUserDir();

	try
	{
		fs::path SessionFile = fs::canonical(fs::path((const char*)HomeDir) / ".trksession");

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

bool TrkPasswdHelper::DeleteSessionTicket(TrkString ServerUrl)
{
	TrkString HomeDir = GetCurrentUserDir();

	try
	{
		fs::path SessionFile = fs::canonical(fs::path((const char*)HomeDir) / ".trksession");

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
				const TrkString line(l.c_str());
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

bool TrkPasswdHelper::ChangeSessionTicket(TrkString Ticket, TrkString ServerUrl)
{
	TrkString HomeDir = GetCurrentUserDir();

	try
	{
		fs::path SessionFile = fs::canonical(fs::path((const char*)HomeDir) / ".trksession");

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
				const TrkString line(l.c_str());
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

TrkString TrkPasswdHelper::GetSessionTicketByServerURL(TrkString ServerUrl)
{
	TrkString HomeDir = GetCurrentUserDir();

	try
	{
		fs::path SessionFile = fs::canonical(fs::path((const char*)HomeDir) / ".trksession");

		if (fs::exists(SessionFile) && fs::is_regular_file(SessionFile))
		{
			std::ifstream SessionFileReader(SessionFile);
			if (SessionFileReader.is_open())
			{
				std::string l;
				while (std::getline(SessionFileReader, l))
				{
					const TrkString line(l.c_str());
					size_t delimiterPos = line.find("=");
					if (delimiterPos != std::string::npos)
					{
						const TrkString key = line.substr(0, delimiterPos);
						const TrkString value = line.substr(delimiterPos + 1);

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