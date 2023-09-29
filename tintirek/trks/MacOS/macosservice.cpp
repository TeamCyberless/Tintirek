/*
 *	macosservice.cpp
 *
 *	Definitions for MacOS service support
 */

#include "../service.h"

#ifdef __APPLE__


void TrkMacOSService::Initialization()
{

}

bool TrkMacOSService::ServiceStart()
{
	return true;
}

void TrkMacOSService::ServiceRunning()
{

}

void TrkMacOSService::ServiceNotifyStop()
{

}

#endif /* __APPLE__ */
