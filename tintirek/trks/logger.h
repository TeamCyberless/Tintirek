/*
 *	logger.h
 * 
 *	Logger helper functions
 */

#ifndef LOGGER_H
#define LOGGER_H


#include <iostream>

#include "trk_types.h"


/* Return a formatted timestamp */
static std::string GetTimestamp(std::string format)
{
	std::time_t now = std::time(nullptr);
	char timestamp[100];
	std::strftime(timestamp, sizeof(timestamp), format.c_str(), std::localtime(&now));
	return std::string(timestamp);
}


/*
 * Log Functions.
 *
 * We have functions similar to stderr and stdout available here.
 * We use these for our daemon system and they also help us convert
 * strings in a user-friendly manner.
 */
#define LOG_OUT(str) std::cout << GetTimestamp("[%Y.%m.%d-%H.%M.%S]") << " " << str << std::endl;
#define LOG_ERR(str) std::cerr << GetTimestamp("[%Y.%m.%d-%H.%M.%S]") << " ERROR: " << str << std::endl;



#endif /* LOGGER_H */