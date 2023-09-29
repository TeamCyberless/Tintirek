/*
 *	linuxserver.cpp
 *
 *	Utilities for the Tintirek Server based on Linux/Unix OS
 */


#ifdef __linux__


#include "../server.h"


TrkLinuxServer::TrkLinuxServer(int Port, TrkCliServerOptionResult* Options)
	: TrkServer(Port, Options)
{

}

bool TrkLinuxServer::Init(TrkString& ErrorStr)
{
	return false;
}

bool TrkLinuxServer::Run(TrkString& ErrorStr)
{
	return false;
}

bool TrkLinuxServer::Cleanup(TrkString& ErrorStr)
{
	return false;
}


#endif /* __linux__ */