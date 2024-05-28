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

bool TrkMacOSServer::Init(std::string& ErrorStr)
{
	return false;
}

bool TrkMacOSServer::Run(std::string& ErrorStr)
{
	return false;
}

bool TrkMacOSServer::Cleanup(std::string& ErrorStr)
{
	return false;
}


#endif /* __APPLE__ */