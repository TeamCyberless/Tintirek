/*
 *	macosserver.cpp
 *
 *	Utilities for the Tintirek Server based on MacOS OS
 */


#ifdef __APPLE__


#include "../server.h"


TrkMacOSServer::TrkMacOSServer(int Port, TrkCliServerOptionResults* Options)
	: TrkServer(Port, Options)
{

}

bool TrkMacOSServer::Init(TrkString& ErrorStr)
{
	return false;
}

bool TrkMacOSServer::Run(TrkString& ErrorStr)
{
	return false;
}

bool TrkMacOSServer::Cleanup(TrkString& ErrorStr)
{
	return false;
}


#endif /* __APPLE__ */