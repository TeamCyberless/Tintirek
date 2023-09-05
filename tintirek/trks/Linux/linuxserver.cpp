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

bool TrkLinuxServer::Init(const char* ErrorStr)
{
}

bool TrkLinuxServer::Run(const char* ErrorStr)
{
}

bool TrkLinuxServer::Cleanup(const char* ErrorStr)
{
}


#endif /* __linux */