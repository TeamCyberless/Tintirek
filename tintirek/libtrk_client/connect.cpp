/*
 *	connect.cpp
 *
 *	Tintirek's client-side connection helpers
 */


#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif


#include "connect.h"
#include "trk_cmdline.h"


bool TrkConnectHelper::SendCommand(TrkCliClientOptionResults& opt_result, const char* Command, const char*& ErrorStr, const char*& Returned)
{
	int client_socket;
	if (!Connect_Internal(opt_result, client_socket, ErrorStr))
	{
		return false;
	}

	int errcode;
	if (!SendPacket(client_socket, Command, errcode))
	{
		std::stringstream os;
		os << "Send Failed! (errno: " << errcode << ")";
#ifdef _WIN32
		WSACleanup();
#endif
		ErrorStr = strdup(os.str().c_str());
		return false;
	}

	const char* message;
	if (!ReceivePacket(client_socket, message, ErrorStr))
	{
		closesocket(client_socket);
		return false;
	}

	std::string msg(message);
	size_t firstNewlinePos = msg.find('\n');
	if (firstNewlinePos != std::string::npos)
	{
		std::string firstLine = msg.substr(0, firstNewlinePos);

		if (firstLine == "OK")
		{
			if (!Disconnect_Internal(client_socket, ErrorStr))
			{
				closesocket(client_socket);
				return false;
			}

			Returned = strdup(msg.substr(firstNewlinePos + 1).c_str());
			return true;
		}
		else if (firstLine == "ERROR")
		{
			ErrorStr = strdup(msg.substr(firstNewlinePos + 1).c_str());
			return false;
		}
	}

	ErrorStr = "Something strange happened. Are you sure about you are not under any MITM attack?";
	return false;
}

bool TrkConnectHelper::SendPacket(int client_socket, const char* message, int& error_code)
{
	std::string data = message;
	int totalSent = 0;
	int chunkSize = 1024;

	while (totalSent < data.size())
	{
		std::chrono::milliseconds sleeptime(100);
		std::this_thread::sleep_for(sleeptime);
		int remaining = data.size() - totalSent;
		int sendSize = std::min<int>(chunkSize, remaining);

		std::string chunkHeader = std::to_string(sendSize) + "\r\n";
		if (send(client_socket, chunkHeader.c_str(), chunkHeader.size(), 0) <= 0)
		{
#ifdef _WIN32
			error_code = WSAGetLastError();
#endif
			return false;
		}

		std::vector<char> sendData(data.begin() + totalSent, data.begin() + totalSent + sendSize);
		if (send(client_socket, sendData.data(), sendData.size(), 0) <= 0)
		{
#ifdef _WIN32
			error_code = WSAGetLastError();
#endif
			return false;
		}

		if (send(client_socket, "\r\n", 2, 0) <= 0)
		{
#ifdef _WIN32
			error_code = WSAGetLastError();
#endif
			return false;
		}

		totalSent += sendSize;
	}

	if (send(client_socket, "0\r\n\r\n", 5, 0) <= 0)
	{
		return false;
	}

	return true;
}

bool TrkConnectHelper::ReceivePacket(int client_socket, const char*& message, const char*& error_msg)
{
	std::vector<char> buffer(1024);
	std::string receivedData;
	bool readingChunkSize = true;
	int chunkSize = 0, lastIndex = 0;

	while (true)
	{
		std::chrono::milliseconds sleeptime(100);
		std::this_thread::sleep_for(sleeptime);
		int bytesRead = recv(client_socket, buffer.data(), buffer.size(), 0);
		if (bytesRead < 0)
		{
			error_msg = "Receive failed unexceptedly.";
			return false;
		}
		else if (bytesRead == 0)
		{
			continue;
		}

		lastIndex = 0;
		for (int i = 0; i < bytesRead; i++) {
			if (readingChunkSize)
			{
				if (buffer[i] == '\r')
				{
					if (i + 1 < bytesRead && buffer[i + 1] == '\n')
					{
						std::string chunkSizeStr(buffer.begin() + lastIndex, buffer.begin() + i);
						chunkSize = std::stoi(chunkSizeStr, nullptr, 16);
						if (chunkSize == 0)
						{
							const std::string WHITESPACE = "\n\r\t\f\v";
							size_t index = receivedData.find_first_not_of(WHITESPACE);
							receivedData = (index == std::string::npos) ? "" : receivedData.substr(index);
							index = receivedData.find_last_not_of(WHITESPACE);
							receivedData = (index == std::string::npos) ? "" : receivedData.substr(0, index + 1);

							message = strdup(receivedData.c_str());
							return true;
						}
						readingChunkSize = false;
						i++;
						lastIndex = i;
					}
					else
					{
						error_msg = "Receive failed unexceptedly.";
						return false;
					}
				}
			}
			else {
				if (buffer[i] == '\r')
				{
					if (i + 1 < bytesRead && buffer[i + 1] == '\n')
					{
						receivedData.append(buffer.begin() + lastIndex, buffer.begin() + i);
						readingChunkSize = true;
						lastIndex = i;
					}
				}
			}
		}
	}

	return true;
}

bool TrkConnectHelper::Connect_Internal(TrkCliClientOptionResults& opt_result, int& client_socket, const char*& ErrorStr )	
{
	if (opt_result.ip_address == nullptr || opt_result.port == 0)
	{
		const char* colon = strchr(opt_result.server_url, ':');
		if (colon != nullptr)
		{
			size_t ipLength = colon - opt_result.server_url;

			char ipBuffer[INET_ADDRSTRLEN];
			strncpy(ipBuffer, opt_result.server_url, ipLength);
			ipBuffer[ipLength] = '\0';
			opt_result.ip_address = ipBuffer;

			const char* portStr = colon + 1;
			opt_result.port = atoi(portStr);
		}
		else
		{
			opt_result.ip_address = opt_result.server_url;
			opt_result.port = 5566;
		}
	}

#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		ErrorStr = "Winsock startup failed.";
		return false;
	}
#endif

	struct addrinfo* result = nullptr, * ptr = nullptr, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (getaddrinfo(opt_result.ip_address, std::to_string(opt_result.port).c_str(), &hints, &result) != 0)
	{
		ErrorStr = "getaddrinfo failed.";
#ifdef _WIN32
		WSACleanup();
#endif
		return false;
	}

#ifdef _WIN32
	
#endif

	int errorCode;
	for (ptr = result; ptr != nullptr; ptr = ptr->ai_next)
	{
		client_socket = static_cast<int>(socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol));
		if (client_socket == INVALID_SOCKET)
		{
			ErrorStr = "Connection socket failed.";
#ifdef _WIN32
			WSACleanup();
#endif
			return false;
		}

		if (connect(client_socket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR)
		{
#ifdef _WIN32
			errorCode = WSAGetLastError();
			closesocket(client_socket);
#endif
			client_socket = INVALID_SOCKET;
		}
	}

	freeaddrinfo(result);

	if (client_socket == INVALID_SOCKET) {
		std::stringstream os;
		os << "Unable to connect to server. (errno: " << errorCode << ")" << std::endl;
#ifdef _WIN32
		LPTSTR errorText = NULL;
		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
			NULL,
			errorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&errorText,
			0,
			NULL
		);
		if (errorText != NULL)
		{
			os << opt_result.ip_address << ":" << opt_result.port << " : " << errorText << std::endl;
		}
		closesocket(client_socket);
		WSACleanup();
#elif
		os << gai_strerror(errno);
		close(client_socket);
#endif
		ErrorStr = strdup(os.str().c_str());
		return false;
	}

	freeaddrinfo(result);
	return true;
}

bool TrkConnectHelper::Disconnect_Internal(int client_socket, const char*& ErrorStr)
{
#ifdef _WIN32
	closesocket(client_socket);
	WSACleanup();
#else
	close(client_socket);
#endif

	return true;
}
