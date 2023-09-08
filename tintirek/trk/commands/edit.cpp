/*
 *	edit.cpp
 *
 *	Tintirek's edit command source file
 */


#include <iostream>
#include <filesystem>
#include <sstream>

#include "edit.h"
#include "connect.h"

namespace fs = std::filesystem;


bool TrkCliEditCommand::CallCommand_Implementation(const TrkCliOption* Options, TrkCliOptionResults* Results)
{
	TrkCliClientOptionResults* ClientResults = static_cast<TrkCliClientOptionResults*>(Results);
	TrkCommandQueue editQueue("");

	try
	{
		fs::path targetPath = fs::canonical(fs::current_path() / (const char*)Results->command_parameter);

		if (fs::exists(targetPath))
		{
			if (fs::is_directory(targetPath))
			{
				TrkCommandQueue* firstElem = new TrkCommandQueue(nullptr);
				editQueue.Enqueue(firstElem);
				int count = 0;
				for (const auto& entry : fs::recursive_directory_iterator(targetPath))
				{
					if (fs::is_regular_file(entry))
					{
						++count;
						TrkString ss;
						ss << "Edit?" << entry.path().string();
						editQueue.Enqueue(new TrkCommandQueue(ss));
					}
				}
				TrkString ss;
				ss << "MultipleCommands?" << count;
				firstElem->command = ss;
			}
			else if (fs::is_regular_file(targetPath)) {
				TrkString errmsg;
				TrkString returned;
				TrkString ss;
				ss << "Edit?" << targetPath.string();
				if (!TrkConnectHelper::SendCommand(*ClientResults, ss, errmsg, returned))
				{
					std::cerr << errmsg << std::endl << std::endl;
					return false;
				}

				return true;
			}
		}
	}
	catch (const fs::filesystem_error& e) {
		std::cerr << "Error: " << e.code().message() << " (errno: " << e.code().value() << ")" << std::endl << std::endl;
		return false;
	}

	TrkString errmsg;
	TrkString returned;
	if (!TrkConnectHelper::SendCommandMultiple(*ClientResults, &editQueue, errmsg, returned))
	{
		std::cerr << errmsg << std::endl << std::endl;
		return true;
	}

	return true;
}

bool TrkCliEditCommand::CheckCommandFlags_Implementation(const char Flag)
{
	return	Flag == 'p' ||
			Flag == 't';
}
