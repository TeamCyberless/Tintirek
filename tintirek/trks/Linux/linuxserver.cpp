/*
 *	linuxserver.cpp
 *
 *	Utilities for the Tintirek Server based on Linux/Unix OS
 */


#ifdef __linux__


#include "../server.h"


TrkLinuxServer::TrkServer(int Port)
	: server_socket(INVALID_SOCKET)
	, port_number(Port)
{

}

bool TrkLinuxServer::Init(TrkString ErrorStr)
{
}

bool TrkLinuxServer::Run(TrkString ErrorStr)
{
}

bool TrkLinuxServer::Cleanup(TrkString ErrorStr)
{
}


#endif /* __linux */