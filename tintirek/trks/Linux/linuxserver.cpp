/*
 *	linuxserver.cpp
 *
 *	Utilities for the Tintirek Server based on Linux/Unix OS
 */


#ifdef __linux__


#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <thread>

#include "../server.h"
#include "../logger.h"


static fd_set* master;

TrkLinuxServer::TrkLinuxServer(int Port, TrkCliServerOptionResult* Options)
	: TrkServer(Port, Options)
{
	server_socket = -1;
	port_number = Port;
	opt_result = Options;
	master = new fd_set;
	FD_ZERO(master);
}

bool TrkLinuxServer::Init(TrkString& ErrorStr)
{
	if (opt_result->ssl_files_path != "")
	{
		try
		{
			TrkSSLHelper::InitSSL();
			ssl_ctx = TrkSSLHelper::CreateServerMethod();

			if (!TrkSSLHelper::LoadSSLFiles(ssl_ctx, opt_result->ssl_files_path))
			{
				ErrorStr << "Certificate files didn't load: " << opt_result->ssl_files_path;
				close(server_socket);
				delete master;
				delete ssl_ctx;
				return false;
			}
		}
		catch (std::exception ex)
		{
			ErrorStr << "Error in SSL initialization: " << ex.what();
			close(server_socket);
			delete master;
			delete ssl_ctx;
			return false;
		}
	}

	struct sockaddr_in addr;

	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1)
	{
		ErrorStr = "Socket failed to create.";
		delete master;
		return false;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_number);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(server_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		ErrorStr = "Unable to bind";
		close(server_socket);
		delete master;
		return false;
	}

	if (listen(server_socket, SOMAXCONN) < 0) {
		ErrorStr = "Unable to listen";
		close(server_socket);
		delete master;
		return false;
	}

	FD_SET(server_socket, master);
	max_socket = server_socket;

	return true;
}

bool TrkLinuxServer::Run(TrkString& ErrorStr)
{
	bool ssl_active = opt_result->ssl_files_path != "";
	fd_set readSet = *master;
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	int activity = select(max_socket + 1, &readSet, nullptr, nullptr, &timeout);
	if (activity == -1)
	{
		int error_code = errno;
		ErrorStr << "There's something went wrong. (err: SERVER01-" << std::to_string(error_code) << ")";
		return false;
	}
	else if (activity > 0)
	{
		for (int i = 0; i <= max_socket; i++)
		{
			if (FD_ISSET(i, &readSet))
			{
				if (i == server_socket)
				{
					sockaddr_in clientAddr;
					socklen_t clientLen = sizeof(clientAddr);
					int clientSocket = accept(server_socket, (struct sockaddr*)&clientAddr, &clientLen);

					char ip[INET_ADDRSTRLEN];
					inet_ntop(AF_INET, &clientAddr.sin_addr, ip, INET_ADDRSTRLEN);
					TrkString ss;
					ss << ip << ":" << htons(clientAddr.sin_port);

					if (clientSocket == -1)
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
							TrkSSLHelper::PrintErrors();
							ErrorStr << "SSL error (errno: " << TrkSSLHelper::GetError(ssl) << ")";
							delete ssl;
							close(clientSocket);
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

bool TrkLinuxServer::Cleanup(TrkString& ErrorStr)
{
	for (int i = 0; i <= max_socket; ++i)
	{
		if (FD_ISSET(i, master))
		{
			close(i);
		}
	}

	delete master;
	delete list;
	delete ssl_ctx;

	return true;
}


#endif /* __linux__ */