/*
 *	connect.cpp
 *
 *	Tintirek's client-side connection helpers
 */


#include <iostream>
#include <chrono>
#include <thread>
#include <regex>

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
	TrkSSLCTX* ssl_context = nullptr;
	TrkSSL* ssl_connection = nullptr;
	if (!Connect_Internal(opt_result, ssl_context, ssl_connection, client_socket, ErrorStr))
	{
		return false;
	}

	int errcode;
	if (!SendPacket(ssl_connection, client_socket, Command, errcode))
	{
		Disconnect_Internal(ssl_context, ssl_connection, client_socket, ErrorStr);
		ErrorStr << "Send Failed! (errno: " << errcode << ")";
		return false;
	}

	TrkString message;
	if (!ReceivePacket(ssl_connection, client_socket, message, ErrorStr))
	{
		Disconnect_Internal(ssl_context, ssl_connection, client_socket, ErrorStr);
		return false;
	}

	size_t firstNewlinePos = message.find("\n");
	TrkString firstLine = (firstNewlinePos != TrkString::npos) ? message.substr(0, firstNewlinePos) : message;

	if (firstLine == "OK")
	{
		Disconnect_Internal(ssl_context, ssl_connection, client_socket, ErrorStr);

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
	TrkSSLCTX* ssl_context;
	TrkSSL* ssl_connection;
	if (!Connect_Internal(opt_result, ssl_context, ssl_connection, client_socket, ErrorStr))
	{
		return false;
	}

	while (!Commands->IsEmpty())
	{
		const char* command = Commands->Peek();

		int errcode;
		if (!SendPacket(ssl_connection, client_socket, command, errcode))
		{
			Disconnect_Internal(ssl_context, ssl_connection, client_socket, ErrorStr);
			ErrorStr << "Send Failed! (errno: " << errcode << ")";
			return false;
		}

		TrkString message;
		if (!ReceivePacket(ssl_connection, client_socket, message, ErrorStr))
		{
			Disconnect_Internal(ssl_context, ssl_connection, client_socket, ErrorStr);
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

	Disconnect_Internal(ssl_context, ssl_connection, client_socket, ErrorStr);

	return true;
}

bool TrkConnectHelper::SendPacket(class TrkSSL* ssl_connection, int client_socket, const TrkString message, int& error_code)
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
		char hexString[9];
		std::sprintf(hexString, "%X", sendSize);
		chunkHeader << hexString << "\r\n";
		if (Send(ssl_connection, client_socket, chunkHeader, chunkHeader.size()) <= 0)
		{
#ifdef _WIN32
			error_code = WSAGetLastError();
#endif
			return false;
		}

		TrkString sendData(message.begin() + totalSent, message.begin() + totalSent + sendSize);
		if (Send(ssl_connection, client_socket, sendData, sendData.size()) <= 0)
		{
#ifdef _WIN32
			error_code = WSAGetLastError();
#endif
			return false;
		}

		if (Send(ssl_connection, client_socket, "\r\n", 2) <= 0)
		{
#ifdef _WIN32
			error_code = WSAGetLastError();
#endif
			return false;
		}

		totalSent += sendSize;
	}

	if (Send(ssl_connection, client_socket, "0\r\n\r\n", 5) <= 0)
	{
#ifdef _WIN32
		error_code = WSAGetLastError();
#endif
		return false;
	}

	return true;
}

bool TrkConnectHelper::ReceivePacket(class TrkSSL* ssl_connection, int client_socket, TrkString& message, TrkString& error_msg)
{
	TrkString buffer, receivedData;
	bool readingChunkSize = true;
	int chunkSize = 0, lastIndex = 0;

	while (true)
	{
		std::chrono::milliseconds sleeptime(100);
		std::this_thread::sleep_for(sleeptime);
		int bytesRead = Recv(ssl_connection, client_socket, buffer, 1024);
		if (bytesRead < 0)
		{
			error_msg = "Receive failed unexceptedly.";
			message = "";
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
						TrkString chunkSizeStr(buffer.begin() + lastIndex, buffer.begin() + i);
						chunkSize = TrkString::stoi(chunkSizeStr, 16);
						if (chunkSize == 0)
						{
							TrkString WHITESPACE = "\n\r\t\f\v";
							size_t index = receivedData.find_first_not_of(WHITESPACE);
							receivedData = (index == TrkString::npos) ? "" : receivedData.substr(index);
							index = receivedData.find_last_not_of(WHITESPACE);
							receivedData = (index == TrkString::npos) ? "" : receivedData.substr(0, index + 1);

							message = receivedData;
							return true;
						}
						readingChunkSize = false;
						i += 2;
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
				int remainingBytes = bytesRead - lastIndex;

				if (remainingBytes >= chunkSize) {
					receivedData << TrkString(buffer.begin() + lastIndex, buffer.begin() + lastIndex + chunkSize);
				}
				else
				{
					int endOfChunk = chunkSize + lastIndex;
					receivedData << TrkString(buffer.begin() + lastIndex, buffer.begin() + endOfChunk);
				}

				i = lastIndex += chunkSize;
				chunkSize = 0;
				readingChunkSize = true;

				if (i + 2 < bytesRead)
				{
					lastIndex += 3;
					i += 3;
				}
			}
		}
	}

	return true;
}

bool TrkConnectHelper::Connect_Internal(TrkCliClientOptionResults& opt_result, TrkSSLCTX*& ssl_context, TrkSSL*& ssl_connection, int& client_socket, TrkString& ErrorStr )
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

	unsigned char response[5] = {0xEA, 0xEB, 0x00, 0xCC, (opt_result.trust ? 0x01 : 0x00)};
	send(client_socket, reinterpret_cast<char*>(response), sizeof(response), 0);

	unsigned char serverResponse[5];
	int bytesRead = recv(client_socket, reinterpret_cast<char*>(serverResponse), sizeof(serverResponse), 0);

	if (bytesRead == sizeof(serverResponse) &&
		serverResponse[0] == 0xEA &&							// Special character 1 for Tintirek's TLS detection
		serverResponse[1] == 0xEB &&							// Special character 2 for Tintirek's TLS detection
		serverResponse[2] == 0x00 &&							// Empty character
		serverResponse[3] == 0xCD &&							// CD: Server, CC: Client
		serverResponse[4] != (opt_result.trust ? 0x01 : 0x00)	// Check server is using TLS mode
		)
	{
		if (opt_result.trust)
		{
			ErrorStr << "The server is not using TLS mode. No need for trust mode.";
		}
		else
		{
			ErrorStr << "The server is using TLS mode. Please enable trust mode and try again.";
		}

#ifdef _WIN32
		closesocket(client_socket);
		WSACleanup();
#else
		close(client_socket);
#endif
		return false;
	}

	if (opt_result.trust)
	{
		TrkSSLHelper::InitSSL();
		ssl_context = TrkSSLHelper::CreateClientMethod();
		ssl_connection = TrkSSLHelper::CreateClient(ssl_context, client_socket);
		TrkSSLHelper::SetupProgramOptionsToContext(ssl_context, &opt_result);

		if (TrkSSLHelper::ConnectServer(ssl_connection) <= 0)
		{
			TrkSSLHelper::RefreshErrors();
			if (opt_result.last_certificate_fingerprint != "")
			{
				if (opt_result.requested_command->command == "trust")
				{
					ErrorStr << "The server at '" << opt_result.server_url << "' requires your trust."
						<< "\nFingerprint:" << opt_result.last_certificate_fingerprint
						<< "\nDo you want to establish trust with this server? (y/n): ";
				}
				else
				{
					ErrorStr << "Connection to '" << opt_result.server_url << "' could not be established."
						<< "\nThis may be your first attempt to connect to this server."
						<< "\nTo establish trust for this server, please run 'trk trust' command."
						<< "\n\nFingerprint: " << opt_result.last_certificate_fingerprint;
				}
			}
			else
			{
				TrkSSLHelper::PrintErrors();
				ErrorStr << "SSL Error. Error code: " << TrkSSLHelper::GetError(ssl_connection);
			}

			delete ssl_context;
			delete ssl_connection;

			return false;
		}
	}

	return true;
}

bool TrkConnectHelper::Disconnect_Internal(TrkSSLCTX* ssl_context, TrkSSL* ssl_connection, int client_socket, TrkString& ErrorStr)
{
#ifdef _WIN32
	WSACleanup();
	closesocket(client_socket);
#else
	close(client_socket);
#endif

	delete ssl_connection;
	delete ssl_context;

	return true;
}

int TrkConnectHelper::Send(TrkSSL* ssl_connection, int client_socket, TrkString buf, int len)
{
	if (ssl_connection != nullptr)
	{
		return TrkSSLHelper::Write(ssl_connection, buf, len);
	}
	else
	{
		return send(client_socket, buf, len, 0);
	}
}

int TrkConnectHelper::Recv(TrkSSL* ssl_connection, int client_socket, TrkString& buf, int len)
{
	if (ssl_connection != nullptr)
	{
		return TrkSSLHelper::Read(ssl_connection, buf, len);
	}
	else
	{
		char internal_strings[1024];
		int bytes_read = recv(client_socket, internal_strings, len, 0);
		
		if (bytes_read > 0)
		{
			buf = TrkString(internal_strings, internal_strings + bytes_read);
		}
		else
		{
			buf = TrkString("");
		}

		return bytes_read;
	}
}
