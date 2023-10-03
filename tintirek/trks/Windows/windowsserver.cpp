/*
 *	windowsserver.cpp
 *
 *	Utilities for the Tintirek Server based on Windows OS
 */

#ifdef WIN32

#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
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

bool TrkWindowsServer::Init(TrkString& ErrorStr)
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

	if (opt_result->ssl_files_path != "")
	{
		try
		{
			TrkSSLHelper::InitSSL();
			ssl_ctx = TrkSSLHelper::CreateServerMethod();

			if (!TrkSSLHelper::LoadSSLFiles(ssl_ctx, opt_result->ssl_files_path))
			{
				ErrorStr << "Certificate files didn't load: " << opt_result->ssl_files_path;
				delete master;
				closesocket(server_socket);
				WSACleanup();
				delete ssl_ctx;
				return false;
			}
		}
		catch (std::exception ex)
		{
			ErrorStr << "Error in SSL initialization: " << ex.what();
			delete master;
			closesocket(server_socket);
			WSACleanup();
			delete ssl_ctx;
			return false;
		}
	}

	return true;
}

bool TrkWindowsServer::Run(TrkString& ErrorStr)
{
	bool ssl_active = opt_result->ssl_files_path != "";
	fd_set readSet = *master;
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	int activity = select(0, &readSet, nullptr, nullptr, &timeout);
	if (activity == SOCKET_ERROR)
	{
		int error_code = WSAGetLastError();
		ErrorStr << "There's something went wrong. (err: SERVER01-" << std::to_string(error_code) << ")";
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

					char ip[INET_ADDRSTRLEN];
					inet_ntop(AF_INET, &clientAddr.sin_addr, ip, INET_ADDRSTRLEN);
					TrkString ss;
					ss << ip << ":" << htons(clientAddr.sin_port);

					if (clientSocket == INVALID_SOCKET)
					{
						ErrorStr << "Client accept error: " << ss;
						continue;
					}

					unsigned char clientResponse[5];
					int bytesRead = recv(clientSocket, reinterpret_cast<char*>(clientResponse), sizeof(clientResponse), 0);

					if (bytesRead == sizeof(clientResponse) &&
						clientResponse[0] == 0xEA &&			// Special character 1 for Tintirek's TLS detection
						clientResponse[1] == 0xEB &&			// Special character 2 for Tintirek's TLS detection
						clientResponse[2] == 0x00 &&			// Empty character
						clientResponse[3] == 0xCC				// CD: Server, CC: Client
						)
					{
						if (ssl_active != (clientResponse[4] == 0x01)) // 1: tls mode active, 0: tls mode deactive
						{
							ErrorStr << "TLS mode mismatch, terminating connection: " << ss;

							unsigned char response[5] = { 0xEA, 0xEB, 0x00, 0xCD, (ssl_active ? 0x01 : 0x00) };
							send(clientSocket, reinterpret_cast<char*>(response), sizeof(response), 0);
							closesocket(clientSocket);
							continue;
						}
						else
						{
							LOG_OUT("Client-Server TLS check-up completed: " << ss);

							unsigned char response[5] = { 0xEA, 0xEB, 0x00, 0xCD, (ssl_active ? 0x01 : 0x00) };
							send(clientSocket, reinterpret_cast<char*>(response), sizeof(response), 0);
						}
					}
					else
					{
						ErrorStr << "Invalid custom packet received, terminating connection: " << ss;
						closesocket(clientSocket);
						continue;
					}

					TrkSSL* ssl = nullptr;
					if (ssl_active)
					{
						ssl = TrkSSLHelper::CreateClient(ssl_ctx, clientSocket);
						if (TrkSSLHelper::AcceptClient(ssl) <= 0)
						{
							TrkSSLHelper::RefreshErrors();
							if (TrkSSLHelper::GetError(ssl) == 6)
							{
								ErrorStr << "Client disconnected during SSL (errno: 6)";
							}
							else
							{
								TrkSSLHelper::PrintErrors();
								ErrorStr << "SSL error (errno: " << TrkSSLHelper::GetError(ssl) << ")";
							}
							
							delete ssl;
							closesocket(clientSocket);
							continue;
						}
					}

					TrkClientInfo* client = new TrkClientInfo(&clientAddr, clientSocket, ssl, ss);
					LOG_OUT("Connection established: " << ss);
					AppendToListUnique(client);
					std::thread(&TrkServer::HandleConnection, this, client).detach();
				}
			}
		}
	}

	return true;
}

bool TrkWindowsServer::Cleanup(TrkString& ErrorStr)
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
	delete ssl_ctx;

	return true;
}

#endif /* WIN32 */