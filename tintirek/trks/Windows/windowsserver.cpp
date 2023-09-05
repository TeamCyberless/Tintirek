/*
 *	windowsserver.cpp
 *
 *	Utilities for the Tintirek Server based on Windows OS
 */


#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <string>
#include <sstream>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

#include "trk_version.h"
#include "../server.h"
#include "../logger.h"


TrkWindowsServer::TrkWindowsServer(int Port, TrkCliServerOptionResult* Options)
	: TrkServer(Port, Options)
{
	server_socket = static_cast<int>(INVALID_SOCKET);
	port_number = Port;
	opt_result = Options;
	master = new fd_set;
	FD_ZERO(master);
}

bool TrkWindowsServer::Init(const char*& ErrorStr)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorStr = "WinSocket failed to create.";
		delete master;
		return false;
	}

	struct addrinfo* result = nullptr;
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(nullptr, std::to_string(port_number).c_str(), &hints, &result))
	{
		ErrorStr = "Get Addres Info failed.";
		delete master;
		WSACleanup();
		return false;
	}

	server_socket = static_cast<int>(socket(result->ai_family, result->ai_socktype, result->ai_protocol));
	if (server_socket == INVALID_SOCKET)
	{
		ErrorStr = "Socket failed to create.";
		delete master;
		freeaddrinfo(result);
		WSACleanup();
		return false;
	}

	if (bind(server_socket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR)
	{
		ErrorStr = "Socket failed to create.";
		delete master;
		freeaddrinfo(result);
		closesocket(server_socket);
		WSACleanup();
		return false;
	}

	freeaddrinfo(result);

	if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR)
	{
		ErrorStr = "Listen failed.";
		delete master;
		closesocket(server_socket);
		WSACleanup();
		return false;
	}

	FD_SET(server_socket, master);
	max_socket = server_socket;
	return true;
}

bool TrkWindowsServer::Run(const char*& ErrorStr)
{
	fd_set readSet = *master;
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	int activity = select(0, &readSet, nullptr, nullptr, &timeout);
	if (activity == SOCKET_ERROR)
	{
		int error_code = WSAGetLastError();
		ErrorStr = strdup(("There's something went wrong. (err: SERVER01-" + std::to_string(error_code) + ")").c_str());
		return false;
	}
	else
	{
		for (int i = 0; i <= max_socket; ++i)
		{
			if (FD_ISSET(i, &readSet))
			{
				if (i == server_socket)
				{
					sockaddr_in clientAddr;
					socklen_t clientLen = sizeof(clientAddr);
					SOCKET clientSocket = accept(server_socket, (struct sockaddr*)&clientAddr, &clientLen);
					if (clientSocket == INVALID_SOCKET)
					{
						ErrorStr = "Client accept error.";
						continue;
					}

					char ip[INET_ADDRSTRLEN];
					inet_ntop(AF_INET, &clientAddr.sin_addr, ip, INET_ADDRSTRLEN);
					std::stringstream os;
					os << ip << ":" << htons(clientAddr.sin_port);
					TrkClientInfo* client = new TrkClientInfo(&clientAddr, clientSocket, strdup(os.str().c_str()));
					LOG_OUT("Connection established: " << os.str());
					AppendToListUnique(client);
					std::thread(&TrkServer::HandleConnection, this, client).detach();
				}
			}
		}
	}

	return true;
}

bool TrkWindowsServer::Cleanup(const char*& ErrorStr)
{
	for (int i = 0; i <= max_socket; ++i)
	{
		if (FD_ISSET(i, &master))
		{
			closesocket(i);
		}
	}

	delete master;
	delete list;
	WSACleanup();

	return true;
}
