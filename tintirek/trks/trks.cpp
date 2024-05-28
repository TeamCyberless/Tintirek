/*
 *	trk.cpp
 *
 *	Tintirek's main server program
 */

#include <string>
#include <iostream>

#ifndef _WIN32
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#else
#include "openssl/applink.c"
#endif

#include <filesystem>
#include <string.h>
#include <sstream>
#include <climits>
#include <signal.h>
#include <cmath>
#include <chrono>

#include "trk_types.h"
#include "trk_version.h"
#include "trk_core.h"
#include "trk_cpp.h"

#include "cmdline.h"
#include "config.h"
#include "database.h"
#include "logger.h"
#include "server.h"
#include "service.h"

namespace fs = std::filesystem;

/* Options and descriptions for the cmd-line. */
const TrkCliOptionFlag trk_cli_options[] =
{
	TrkCliOptionFlag('v', std::string("Displays version info")),
	TrkCliOptionFlag('h', std::string("Displays usage")),

#ifndef _WIN32
	TrkCliOptionFlag('d', std::string("Starts as daemon mode")),
	TrkCliOptionFlag('i', std::string("Assign a pid file to process")),
	TrkCliOptionFlag('l', std::string("Defines log file path")),
#endif

	TrkCliOptionFlag('p', std::string("Sets server running port")),
	TrkCliOptionFlag('r', std::string("Sets server root directory")),
	TrkCliOptionFlag('s', std::string("Sets SSL path containing the server SSL credential files"), std::string("Path"))
};


/* Prints version information to screen */
void print_version()
{
	TRK_VERSION_DEFINE(ver_info);
    std::cout << trk_get_full_version_info(&ver_info) << std::endl;
}


/* Prints help to screen */
void print_help(bool print_legal = false)
{
	if (print_legal)
	{
		std::cout << "Tintirek Version Control Software Server Program by TeamCyberless." << std::endl;
	}

	std::cout << std::endl << "Usage: trk [-d] [-i <pid_file>] [-p <port>] [-r <path>] [-s <ssl files path>]" << std::endl << std::endl;

	std::cout << "Flags:" << std::endl;
	for (const auto flag : trk_cli_options)
	{
		std::cout << "-" << flag.character << ": " << "\t" << flag.description << std::endl;
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
    { NULL, NULL },
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

    TrkCliServerOptionResults opt_result;
	opt_result.start_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

#ifndef _WIN32
	struct flock fileLock;
	int pidFileDescriptor = -1;
#endif

	/* Check commands */
	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-' && argv[i][1] != '\0')
		{
			char opt = argv[i][1];

			switch (opt)
			{
			case 'v':
				print_version();
				return EXIT_SUCCESS;
				break;

			case 'h':
				print_help(true);
				return EXIT_SUCCESS;
				break;
#ifndef _WIN32
			case 'd':
                opt_result.daemon = true;
				break;


            case 'i':
                if (!return_argument_or_null(opt_result.pid_file, 'i', i, argv, argc))
				{
					print_help();
					return EXIT_FAILURE;
				}
				break;

			case 'l':
                if (!return_argument_or_null(opt_result.log_path, 'l', i, argv, argc))
				{
					print_help();
					return EXIT_FAILURE;
				}
				break;
#endif
			case 'p':
            {
            	std::string optarg;
				if (!return_argument_or_null(optarg, 'p', i, argv, argc))
				{
					print_help();
					return EXIT_FAILURE;
				}
				else
				{
					char* endPtr;
					long result = std::strtol(optarg.c_str(), &endPtr, 10);

					if (std::strcmp(optarg.c_str(), endPtr))
					{
						std::cerr << "Parameter [-" << opt << "] requires an positive numeric argument." << std::endl;
						print_help();
						return EXIT_FAILURE;
					}

					if (result >= 0 && result <= UINT16_MAX)
					{
                        opt_result.port_number = static_cast<uint16_t>(result);
					}
					else
					{
						std::cerr << "The numeric value of parameter [-" << opt << "] argument was out of range." << std::endl;
						print_help();
						return EXIT_FAILURE;
					}
				}
			}
				break;

			case 'r':
                if (!return_argument_or_null(opt_result.running_root, 'r', i, argv, argc))
				{
					print_help();
					return EXIT_FAILURE;
				}
				break;

			case 's':
                if (!return_argument_or_null(opt_result.ssl_files_path, 'r', i, argv, argc))
				{
					print_help();
					return EXIT_FAILURE;
				}
				break;

			default:
				std::cerr << "Unknown parameter [-" << opt << "]" << std::endl;
				print_help();
				return EXIT_FAILURE;
			}
		}
	}

    if (opt_result.pid_file == "")
    {
        if (opt_result.log_path != "")
		{
			std::cout << "Log path parameter [-l] must be used with PID parameter." << std::endl;
			print_help();
			return EXIT_FAILURE;
        }

		std::stringstream ss;
		ss << opt_result.pid_file << (fs::temp_directory_path() / "trks.pid").string();
		opt_result.pid_file = ss.str();
	}

    if (opt_result.log_path == "")
	{
		std::stringstream ss;
		ss << opt_result.log_path << (fs::temp_directory_path() / "trks_log_").string() << GetTimestamp("%Y-%m-%d_%H-%M-%S");
		opt_result.log_path = ss.str();
	}

    if (opt_result.running_root == "")
	{
		std::stringstream ss;
		ss << opt_result.running_root << fs::current_path().string() << (char)fs::path::preferred_separator;
		opt_result.running_root = ss.str();
	}

	{
		LOG_OUT("Check and generate databases...");
		auto dbInitStart = std::chrono::high_resolution_clock::now();

        InitDatabases(opt_result.running_root);

		LOG_OUT("Database generation done! Took " << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - dbInitStart).count() << " seconds.");
	}

#ifdef _WIN32
	TrkWindowsService service;
    TrkWindowsServer server(opt_result.port_number, &opt_result);
#elif __APPLE__
	TrkMacOSService service;
	TrkMacOSServer server(opt_result.port_number, &opt_result);
#elif __linux__
	TrkLinuxService service(opt_result.pid_file, opt_result.log_path);
	TrkLinuxServer server(opt_result.port_number, &opt_result);
#endif

	std::string ErrorStr = "";
	service.Initialization();
    if ((!opt_result.daemon || service.ServiceStart()) && server.Init(ErrorStr))
	{
        LOG_OUT("Listening server at " << opt_result.port_number << " port")
		LOG_OUT("")
		LOG_OUT("========== INFO ==========")
		LOG_OUT("Start Time: " << GetTimestamp("%Y/%m/%d %H:%M:%S %z"))
#ifndef _WIN32
        LOG_OUT("Daemon: " << (opt_result.daemon ? "Enabled" : "Disabled"))
        LOG_OUT("PID File: " << opt_result.pid_file)
        LOG_OUT("Log File: " << opt_result.log_path)
#endif
        LOG_OUT("Port: " << opt_result.port_number)
        LOG_OUT("Root: " << opt_result.running_root)

        if (opt_result.ssl_files_path != "")
		{
            LOG_OUT("SSL Files Path: " << opt_result.ssl_files_path)
		}

		LOG_OUT("==========================")

		while (!service.DoesServiceStopping())
		{
			service.ServiceRunning();
			
			if (!server.Run(ErrorStr))
			{
				service.ServiceMustStop();
				break;
			}
			else
			{
				if (ErrorStr != "")
				{
					LOG_OUT(ErrorStr);
					ErrorStr = "";
				}
			}
		}

		LOG_OUT("Stopping Server...")
		service.ServiceNotifyStop();
	}
	else
	{
		LOG_ERR("Failed to start daemon: " << ErrorStr)
		return EXIT_FAILURE;
	}

	if (ErrorStr != "")
	{
		LOG_ERR(ErrorStr);
	}
	else
	{
		LOG_OUT("Goodbye!");
	}
	return (ErrorStr != "") ? EXIT_FAILURE : EXIT_SUCCESS;
}
