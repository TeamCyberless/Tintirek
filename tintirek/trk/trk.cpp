/*
 *	trk.cpp
 *
 *	Tintirek cmd-line file.
 */


#include <string.h>
#include <assert.h>
#include <iostream>
#include <climits>
#include <string>
#include <thread>

#include "trk_types.h"
#include "trk_version.h"
#include "trk_client.h"
#include "trk_core.h"
#include "trk_cpp.h"

#include "cmdline.h"
#include "commandline.h"
#include "connect.h"
#include "config.h"



 /* Options and descriptions for the cmd-line. */
const TrkCliOption trk_cli_options[] =
{
	TrkCliOption("version", nullptr, "Displays version information about Tintirek.", TrkCliRequiredOption::NOT_ALLOWED),
	TrkCliOption("help", nullptr, "Displays help information about Tintirek.", TrkCliRequiredOption::NO_REQUIRED, "command"),
	TrkCliOption("info", trkInfoCommand, "Displays connection, status and other information about Tintirek", TrkCliRequiredOption::NOT_ALLOWED),

	TrkCliOption("trust", trkTrustCommand, "Establish trust with the server by verifying its identity and certificate", TrkCliRequiredOption::NOT_ALLOWED),
	TrkCliOption("login", trkLoginCommand, "Performs the login process with the server", new TrkCliOptionFlag[1] { TRK_CLI_FLAG_STATUS }, 1, TrkCliRequiredOption::NO_REQUIRED, "user"),
	TrkCliOption("logout", trkLogoutCommand, "Performs the login process with the server", TrkCliRequiredOption::NOT_ALLOWED),

	TrkCliOption("add", trkAddCommand, "Open a new file to add it to the repository.", new TrkCliOptionFlag[3] { TRK_CLI_FLAG_IGNORE, TRK_CLI_FLAG_PREVIEW, TRK_CLI_FLAG_TYPE }, 3, TrkCliRequiredOption::REQUIRED, "file/dir"),
	TrkCliOption("edit", trkEditCommand, "Open an existing file for edit.", new TrkCliOptionFlag[2] { TRK_CLI_FLAG_PREVIEW, TRK_CLI_FLAG_TYPE }, 2, TrkCliRequiredOption::REQUIRED, "file/dir"),
	TrkCliOption("delete", trkAddCommand, "Open an existing file for deletion from the repository.", new TrkCliOptionFlag[1] { TRK_CLI_FLAG_PREVIEW }, 1, TrkCliRequiredOption::REQUIRED, "file/dir"),
	TrkCliOption("revert", trkAddCommand, "Discard changed from an opened file.", new TrkCliOptionFlag[2] { TRK_CLI_FLAG_ADD, TRK_CLI_FLAG_PREVIEW }, 2, TrkCliRequiredOption::REQUIRED, "file/dir"),
	TrkCliOption("submit", trkAddCommand, "Submit open files to the remote.", new TrkCliOptionFlag[2] { TRK_CLI_FLAG_DESCRIPTION, TRK_CLI_FLAG_REOPEN }, 2, TrkCliRequiredOption::REQUIRED, "file/dir"),
	TrkCliOption("sync", trkAddCommand, "Synchronize the client with its view of the repository.", new TrkCliOptionFlag[3] { TRK_CLI_FLAG_FORCE, TRK_CLI_FLAG_PREVIEW, TRK_CLI_FLAG_MAX }, 3)

};



/* Prints version information to screen */
void print_version()
{
    TRK_VERSION_DEFINE(ver_info);
    std::cout << trk_get_full_version_info(&ver_info) << std::endl;
}


/* Prints help to screen */
void print_help(std::string command)
{
	if (command == "\0")
	{
		std::cout << "Tintirek Version Control Software by TeamCyberless." << std::endl << std::endl;
		std::cout << "Usage: trk <command> [-flags] (command parameter)" << std::endl;
		std::cout << "These are common Tintirek commands:" << std::endl << std::endl;
	}

	for (auto option : trk_cli_options)
	{
		if (command == "")
		{
			std::cout << option.command << "\t\t" << option.description << std::endl;
		}
		else if (command != "" && command == option.command)
		{
			std::cout << option.command << ": " << option.description << std::endl;
			std::cout << "Usage: trk " << option.command << " " << option.GetFlagsForHelpText();

			switch (option.required_parameter)
			{
			case TrkCliRequiredOption::NO_REQUIRED:
				std::cout << "[" << option.parameter_name << "]" << std::endl << std::endl;
				break;
			case TrkCliRequiredOption::REQUIRED:
				std::cout << "<" << option.parameter_name << ">" << std::endl << std::endl;
				break;
			case TrkCliRequiredOption::NOT_ALLOWED:
				std::cout << std::endl << std::endl;
				break;
			}

			if (option.head_flag != nullptr)
			{
				std::cout << "Flags:" << std::endl;
				for (const auto flag : *option.head_flag)
				{
					std::cout << "-" << flag->character << ": " << "\t" << flag->description << std::endl;
				}
			}
		}
	}
}


/* Takes the argument if given, otherwise null and returns false */
bool return_argument_or_null(std::string& argument, char opt, int& i, char** argv, int argc)
{
	if (argv[i][2] == '\0' && i + 1 < argc && argv[i + 1][0] != '-')
	{
		argument = argv[i + 1];
		++i;
		return true;
	}

	std::cerr << "Parameter [-" << opt << "] requires an argument." << std::endl;
	argument = "";
	return false;
}


/* Library version check */
const trk_version_checklist_t libVersionList[] =
{
    { "trk_core", trk_core_version },
    { "trk_cpp", trk_cpp_version },
    { "trk_client", trk_client_version },
	nullptr
};



/* Main function of our program */
int main(int argc, char** argv)
{
	/* Check library versions */
	{
		TRK_VERSION_DEFINE(MyVer);
		std::string ErrorStr;
        if (!trk_version_check_list(&MyVer, libVersionList, false, ErrorStr))
		{
			std::cerr << ErrorStr << std::endl;
			return EXIT_FAILURE;
		}
	}

	/* When not passed any arguments, show usage. */
	if (argc < 2) {
		print_help("");
		return EXIT_FAILURE;
	}

	/* Quick escape for some commands */
    if (strcmp(argv[1], "version") == 0)
	{
		print_version();
		return EXIT_SUCCESS;
	}
    else if (strcmp(argv[1], "help") == 0)
	{
		print_help(argv[2] ? argv[2] : "");
		return EXIT_SUCCESS;
	}

	TrkCliClientOptionResults opt_result;
	TrkConfig::LoadConfig(&opt_result);

	/* Get command requested by client */
	for (const auto& cmd : trk_cli_options)
	{
        if (strcmp(cmd.command.c_str(), argv[1]) == 0)
		{
			opt_result.requested_command = &cmd;
			break;
		}
	}

	/* Check this command is valid. If not, show usage. */
	if (opt_result.requested_command == nullptr)
	{
		std::cerr << "Command \"" << argv[1] << "\" is not valid command." << std::endl << std::endl;
		print_help("");
		return EXIT_FAILURE;
	}

	/* Check flags */
	for (int i = 2; i < argc; i++)
	{
		if (argc == i + 1 && argv[i][0] != '-')
		{
			opt_result.command_parameter = argv[i];
			i++;
			continue;
		}

		if (argv[i][0] == '-' && argv[i][1] != '\0')
		{
			char opt = argv[i][1];

			if (!opt_result.requested_command->cmd_util->CheckCommandFlags(opt))
			{
				std::cerr << "Not allowed flag [-" << opt << "]" << std::endl << std::endl;
				print_help(argv[1]);
				return EXIT_FAILURE;
			}

			switch (opt)
			{
			case 'i':
				opt_result.ignore = true;
				break;
			
			case 'p':
				opt_result.preview = true;
				break;

			case 's':
				opt_result.showsessionticket = true;
				break;

			case 't':
				if (!return_argument_or_null(opt_result.description, 't', i, argv, argc))
				{
					return EXIT_FAILURE;
				}
				break;

			case 'a':
				opt_result.not_changed = true;
				break;

			case 'd':
				if (!return_argument_or_null(opt_result.description, 'd', i, argv, argc))
				{
					return EXIT_FAILURE;
				}
				break;

			case 'r':
				opt_result.remark = true;
				break;

			case 'f':
				opt_result.force = true;
				break;

			case 'm':
			{
				std::string optarg;
				if (!return_argument_or_null(optarg, 'm', i, argv, argc))
				{
					return EXIT_FAILURE;
				}
				else
				{
					char* endPtr;
					long result = std::strtoul(optarg.c_str(), &endPtr, 10);

					if (std::strcmp(optarg.c_str(), endPtr))
					{
						std::cerr << "Parameter [-" << opt << "] requires an positive numeric argument." << std::endl << std::endl;
						return EXIT_FAILURE;
					}

					if (result >= INT_MIN && result <= INT_MAX)
					{
						opt_result.affect_max = static_cast<int>(result);
					}
					else
					{
						std::cerr << "The numeric value of parameter [-" << opt << "] argument was out of range." << std::endl << std::endl;
						return EXIT_FAILURE;
					}
				}
			}
				break;

			default:
				std::cerr << "Not allowed flag [-" << opt << "]" << std::endl << std::endl;
				print_help(argv[1]);
				return EXIT_FAILURE;
			}
		}
	}

	/* Check if any command passed and ensure we allow it */
	switch (opt_result.requested_command->required_parameter)
	{
	case TrkCliRequiredOption::NOT_ALLOWED:

		if (opt_result.command_parameter != "")
		{
			std::cerr << "Command parameter not allowed. Don't use this command with the command parameter." << std::endl << std::endl;
			print_help(argv[1]);
			return EXIT_FAILURE;
		}

		break;
	case TrkCliRequiredOption::REQUIRED:

		if (opt_result.command_parameter == "")
		{
			std::cerr << "The command parameter is required and not passed. You must use this command with the command parameter." << std::endl << std::endl;
			print_help(argv[1]);
			return EXIT_FAILURE;
		}

		break;

	default: break;
	}

	if (opt_result.ip_address == "" || opt_result.port == 0)
	{
		std::string realurl = opt_result.server_url;
		if (realurl.rfind("tls:", 0) != std::string::npos)
		{
			opt_result.trust = true;
			realurl = realurl.substr(4);
		}

		size_t found = realurl.find(":");
		if (found != std::string::npos)
		{
			opt_result.ip_address = realurl.substr(0, found);
			opt_result.port = atoi(realurl.substr(found + 1).c_str());
		}
		else
		{
			opt_result.ip_address = realurl;
			opt_result.port = 5566;
		}
	}

	std::chrono::milliseconds sleeptime(100);
	std::this_thread::sleep_for(sleeptime);
	if (!opt_result.requested_command->cmd_util->CallCommand(opt_result.requested_command, &opt_result))
	{
		print_help(argv[1]);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}