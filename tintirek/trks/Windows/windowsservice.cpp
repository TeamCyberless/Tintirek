/*
 *	windowsservice.cpp
 * 
 *	Definitions for Windows service support
 */

#include "../service.h"

#ifdef _WIN32

#include <iostream>
#include <windows.h>



/* static reference for our windows system */
static TrkWindowsService* static_service;


/* Ctrl + C */
BOOL CtrlHandler(DWORD fdwCtrlType) {
	if (fdwCtrlType == CTRL_C_EVENT || fdwCtrlType == CTRL_BREAK_EVENT) {
		static_service->ServiceMustStop();
		return TRUE;
	}
	return FALSE;
}



void TrkWindowsService::Initialization()
{
	static_service = this;

	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE)) {
		std::cout << "Error setting CtrlHandler!";
		exit(EXIT_FAILURE);
	}
}

bool TrkWindowsService::ServiceStart()
{	
	return true;
}

void TrkWindowsService::ServiceRunning()
{
	
}

void TrkWindowsService::ServiceNotifyStop()
{
	
}

#endif /* _WIN32 */
