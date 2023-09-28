/*
 *	connect.cpp
 *
 *	Tintirek's client-side connection helpers
 */


#include <iostream>
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
#include <netdb.h>
#define INVALID_SOCKET (~0)
#define SOCKET_ERROR (-1)
#endif

#include "connect.h"
#include "trk_cmdline.h"


bool TrkConnectHelper::SendCommand(TrkCliClientOptionResults& opt_result, const TrkString Command, TrkString& ErrorStr, TrkString& Returned)
{
	int client_socket;
	if (!Connect_Internal(opt_result, client_socket, ErrorStr))
	{
		return false;
	}

	int errcode;
	if (!SendPacket(client_socket, Command, errcode))
	{
		TrkString str;
		str << "Send Failed! (errno: " << errcode << ")";
#ifdef _WIN32
		WSACleanup();
#endif
		ErrorStr = str;
		return false;
	}

	TrkString message;
	if (!ReceivePacket(client_socket, message, ErrorStr))
	{
#if WIN32
		closesocket(client_socket);
#else
		close(client_socket);
#endif
		return false;
	}

	size_t firstNewlinePos = message.find("\n");
	TrkString firstLine = (firstNewlinePos != TrkString::npos) ? message.substr(0, firstNewlinePos) : message;

	if (firstLine == "OK")
	{
		if (!Disconnect_Internal(client_socket, ErrorStr))
		{
#if WIN32
			closesocket(client_socket);
#else
			close(client_socket);
#endif
			return false;
		}

		if (firstNewlinePos != TrkString::npos)
		{
			Returned = message.substr(firstNewlinePos + 1);
		}
		return true;
	}
	else if (firstLine == "ERROR")
	{
		if (firstNewlinePos != TrkString::npos)
		{
			ErrorStr = message.substr(firstNewlinePos + 1);
		}
		return false;
	}

	std::cout << message << std::endl;
	ErrorStr = "Something strange happened. Are you sure about you are not under any MITM attack?";
	return false;
}

bool TrkConnectHelper::SendCommandMultiple(class TrkCliClientOptionResults& opt_result, class TrkCommandQueue* Commands, TrkString& ErrorStr, TrkString& Returned)
{
	int client_socket;
	if (!Connect_Internal(opt_result, client_socket, ErrorStr))
	{
		return false;
	}

	while (!Commands->IsEmpty())
	{
		const char* command = Commands->Peek();

		int errcode;
		if (!SendPacket(client_socket, command, errcode))
		{
			TrkString str;
			str << "Send Failed! (errno: " << errcode << ")";
#ifdef _WIN32
			WSACleanup();
#endif
			ErrorStr = str;
			return false;
		}

		TrkString message;
		if (!ReceivePacket(client_socket, message, ErrorStr))
		{
#if WIN32
			closesocket(client_socket);
#else
			close(client_socket);
#endif
			return false;
		}

		size_t firstNewlinePos = message.find("\n");
		
		if (firstNewlinePos != TrkString::npos)
		{
			TrkString firstLine = message.substr(0, firstNewlinePos);

			if (firstLine == "OK")
			{
				std::cout << message.substr(firstNewlinePos + 1) << std::endl;
			}
			else if (firstLine == "ERROR")
			{
				ErrorStr = message.substr(firstNewlinePos + 1);
				return false;
			}
		}

		Commands->Dequeue();
	}

	if (!Disconnect_Internal(client_socket, ErrorStr))
	{
#if WIN32
		closesocket(client_socket);
#else
		close(client_socket);
#endif
		return false;
	}

	return true;
}

bool TrkConnectHelper::SendPacket(int client_socket, const TrkString message, int& error_code)
{
	int totalSent = 0;
	int chunkSize = 1024;

	while (totalSent < message.size())
	{
		std::chrono::milliseconds sleeptime(100);
		std::this_thread::sleep_for(sleeptime);
		int remaining = message.size() - totalSent;
		int sendSize = std::min<int>(chunkSize, remaining);

		TrkString chunkHeader;
		chunkHeader << sendSize << "\r\n";
		if (send(client_socket, chunkHeader, chunkHeader.size(), 0) <= 0)
		{
#ifdef _WIN32
			error_code = WSAGetLastError();
#endif
			return false;
		}

		TrkString sendData(message.begin() + totalSent, message.begin() + totalSent + sendSize);
		int i = send(client_socket, sendData, sendData.size(), 0);
		if (i <= 0)
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
#ifdef _WIN32
		error_code = WSAGetLastError();
#endif
		return false;
	}

	return true;
}

bool TrkConnectHelper::ReceivePacket(int client_socket, TrkString& message, TrkString& error_msg)
{
	TrkString buffer, receivedData;
	bool readingChunkSize = true;
	int chunkSize = 0, lastIndex = 0;

	while (true)
	{
		std::chrono::milliseconds sleeptime(100);
		std::this_thread::sleep_for(sleeptime);
		char internal_string[1024];
		int bytesRead = recv(client_socket, internal_string, 1024, 0);
		if (bytesRead < 0)
		{
			error_msg = "Receive failed unexceptedly.";
			return false;
		}
		else if (bytesRead == 0)
		{
			continue;
		}

		buffer = TrkString(internal_string, internal_string + bytesRead);

		lastIndex = 0;
		for (int i = 0; i < bytesRead; i++) {
			if (readingChunkSize)
			{
				if (buffer[i] == '\r')
				{
					if (i + 1 < bytesRead && buffer[i + 1] == '\n')
					{
						TrkString chunkSizeStr(buffer.begin() + lastIndex, buffer.begin() + i);
						chunkSize = TrkString::stoi(chunkSizeStr, 16);
						if (chunkSize == 0)
						{
							const TrkString WHITESPACE = "\n\r\t\f\v";
							size_t index = receivedData.find_first_not_of(WHITESPACE);
							receivedData = (index == TrkString::npos) ? "" : receivedData.substr(index);
							index = receivedData.find_last_not_of(WHITESPACE);
							receivedData = (index == TrkString::npos) ? "" : receivedData.substr(0, index + 1);

							message = receivedData;
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
						receivedData << TrkString(buffer.begin() + lastIndex, buffer.begin() + i);
						readingChunkSize = true;
						lastIndex = i;
					}
				}
			}
		}
	}

	return true;
}

bool TrkConnectHelper::Connect_Internal(TrkCliClientOptionResults& opt_result, int& client_socket, TrkString& ErrorStr )
{
	struct addrinfo *result = nullptr, *ptr = nullptr, hints;

#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		ErrorStr = "Winsock startup failed.";
		return false;
	}

	ZeroMemory(&hints, sizeof(hints));
#else
	memset(&hints, 0, sizeof(hints));
#endif
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	TrkString port;
	port << opt_result.port;
	if (getaddrinfo(opt_result.ip_address, port, &hints, &result) != 0)
	{
		TrkString ss;
#ifdef _WIN32
		ss << "getaddrinfo failed. (errno: " << WSAGetLastError() << ")";
		ErrorStr = ss;
		WSACleanup();
#else
		ss << "getaddrinfo failed. (errno: " << errno << ")";
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

	if (client_socket == INVALID_SOCKET) {
		freeaddrinfo(result);
		TrkString ss;
		ss << "Unable to connect to server. (errno: " << errorCode << ")\n";
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
			ss << opt_result.ip_address << ":" << opt_result.port << " : " << errorText ;
		}
		closesocket(client_socket);
		WSACleanup();
#else
		ss << gai_strerror(errno);
		close(client_socket);
#endif
		ErrorStr = ss;
		return false;
	}

	freeaddrinfo(result);
	return true;
}

bool TrkConnectHelper::Disconnect_Internal(int client_socket, TrkString& ErrorStr)
{
#ifdef _WIN32
	closesocket(client_socket);
	WSACleanup();
#else
	close(client_socket);
#endif

	return true;
}
