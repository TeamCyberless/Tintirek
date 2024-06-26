/*
 *	add.cpp
 *
 *	Tintirek's add command source file
 */


#include <iostream>
#include <filesystem>
#include <sstream>

#include "add.h"
#include "connect.h"

namespace fs = std::filesystem;


bool TrkCliAddCommand::CallCommand_Implementation(const TrkCliOption* Options, TrkCliOptionResults* Results)
{
	TrkCliClientOptionResults* ClientResults = static_cast<TrkCliClientOptionResults*>(Results);
	TrkCommandQueue addQueue("");

	try
	{
		fs::path targetPath = fs::canonical(fs::current_path() / (const char*)Results->command_parameter);

		if (fs::exists(targetPath))
		{
			if (fs::is_directory(targetPath))
			{
                TrkCommandQueue* firstElem = new TrkCommandQueue("");
				addQueue.Enqueue(firstElem);
				int count = 0;
				for (const auto& entry : fs::recursive_directory_iterator(targetPath))
				{
					if (fs::is_regular_file(entry))
					{
						++count;
						TrkString ss;
						ss << "Add?" << entry.path().string();
						addQueue.Enqueue(new TrkCommandQueue(ss));
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
				ss << "Add?" << targetPath.string();
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
	if (!TrkConnectHelper::SendCommandMultiple(*ClientResults, &addQueue, errmsg, returned))
	{
		std::cerr << errmsg << std::endl << std::endl;
		return false;
	}

	return true;
}

bool TrkCliAddCommand::CheckCommandFlags_Implementation(const char Flag)
{
	return	Flag == 'i' ||
			Flag == 'p' ||
			Flag == 't';
}