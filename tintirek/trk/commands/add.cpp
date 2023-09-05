/*
 *	add.cpp
 *
 *	Tintirek's add command source file
 */


#include <iostream>
#include <filesystem>
#include <set>
#include <vector>
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
		fs::path targetPath = fs::canonical(fs::current_path() / Results->command_parameter);

		if (fs::exists(targetPath))
		{
			if (fs::is_directory(targetPath))
			{
				TrkCommandQueue* firstElem = new TrkCommandQueue(nullptr);
				addQueue.Enqueue(firstElem);
				int count = 0;
				for (const auto& entry : fs::recursive_directory_iterator(targetPath))
				{
					if (fs::is_regular_file(entry))
					{
						++count;
						std::stringstream os;
						os << "Add?" << entry.path().string();
						addQueue.Enqueue(new TrkCommandQueue(strdup(os.str().c_str())));
					}
				}
				std::stringstream os;
				os << "MultipleCommands?" << count;
				firstElem->command = strdup(os.str().c_str());
			}
			else if (fs::is_regular_file(targetPath)) {
				const char* errmsg;
				const char* returned;
				std::stringstream os;
				os << "Add?" << targetPath.string() << std::endl;
				if (!TrkConnectHelper::SendCommand(*ClientResults, strdup(os.str().c_str()), errmsg, returned))
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

	const char* errmsg;
	const char* returned;
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

bool isAllowedPath(const fs::path& path)
{
	static const std::set<std::string> disallowedNames = { ".", ".." };

	for (const auto& part : path) {
		if (disallowedNames.find(part.string()) != disallowedNames.end()) {
			return false;
		}
	}
	return true;
}