/*
 *	add.cpp
 *
 *	Tintirek's add command source file
 */


#include "add.h"

#include <iostream>
#include <filesystem>
#include <set>

namespace fs = std::filesystem;


bool TrkCliAddCommand::CallCommand_Implementation(const TrkCliOption* Options, TrkCliOptionResults* Results)
{
	try
	{
		fs::path targetPath = fs::canonical(fs::current_path() / Results->command_parameter);

		if (fs::exists(targetPath))
		{
			if (fs::is_directory(targetPath))
			{
				for (const auto& entry : fs::recursive_directory_iterator(targetPath))
				{
					if (fs::is_regular_file(entry))
					{
						std::cout << "File: " << entry.path() << std::endl;
					}
				}
			}
			else if (fs::is_regular_file(targetPath)) {
				std::cout << "File: " << targetPath << std::endl;
			}
		}
	}
	catch (const fs::filesystem_error& e) {
		std::cerr << "Error: " << e.code().message() << " (errno: " << e.code().value() << ")" << std::endl << std::endl;
		return false;
	}

	TrkCliClientOptionResults* ClientResults = dynamic_cast<TrkCliClientOptionResults*>(Results);
	if (ClientResults != nullptr)
		std::cout << "Username: " << ClientResults->username << std::endl << "Password: " << ClientResults->password << std::endl;

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