/*
 *	connect.cpp
 *
 *	Tintirek's client-side connection helpers
 */


#include <iostream>
#include <chrono>
#include <thread>
#include <regex>
#include <sstream>
#include <string>

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

#include "cmdline.h"
#include "connect.h"
#include "passwd.h"


bool TrkConnectHelper::SendCommand(TrkCliClientOptionResults& opt_result, const std::string Command, std::string& ErrorStr, std::string& Returned)
{
	int client_socket;
	TrkSSLCTX* ssl_context = nullptr;
	TrkSSL* ssl_connection = nullptr;
	if (!Connect_Internal(opt_result, ssl_context, ssl_connection, client_socket, ErrorStr))
	{
		return false;
	}

	if (!Authenticate_Internal(&opt_result, ssl_connection, client_socket, ErrorStr))
	{
		Disconnect_Internal(ssl_context, ssl_connection, client_socket, ErrorStr);
		return false;
	}

	if (!SendPacket(ssl_connection, client_socket, Command, ErrorStr))
	{
		Disconnect_Internal(ssl_context, ssl_connection, client_socket, ErrorStr);
		return false;
	}

	if (Command != "Close")
	{
		std::string message;
		if (!ReceivePacket(ssl_connection, client_socket, message, ErrorStr))
		{
			Disconnect_Internal(ssl_context, ssl_connection, client_socket, ErrorStr);
			return false;
		}

		size_t firstNewlinePos = message.find("\n");
		std::string firstLine = (firstNewlinePos != std::string::npos) ? message.substr(0, firstNewlinePos) : message;

		if (firstLine == "OK")
		{
			Disconnect_Internal(ssl_context, ssl_connection, client_socket, ErrorStr);

			if (firstNewlinePos != std::string::npos)
			{
				Returned = message.substr(firstNewlinePos + 1);
			}
			return true;
		}
		else if (firstLine == "ERROR")
		{
			if (firstNewlinePos != std::string::npos)
			{
				ErrorStr = message.substr(firstNewlinePos + 1);
			}
			return false;
		}

		std::cout << message << std::endl;
		ErrorStr = "Something strange happened. Are you sure about you are not under any MITM attack?";
		return false;
	}
	else
	{
		Disconnect_Internal(ssl_context, ssl_connection, client_socket, ErrorStr);
		return true;
	}
}

bool TrkConnectHelper::SendCommandMultiple(class TrkCliClientOptionResults& opt_result, class TrkCommandQueue* Commands, std::string& ErrorStr, std::string& Returned)
{
	int client_socket;
	TrkSSLCTX* ssl_context = nullptr;
	TrkSSL* ssl_connection = nullptr;
	if (!Connect_Internal(opt_result, ssl_context, ssl_connection, client_socket, ErrorStr))
	{
		return false;
	}

	if (!Authenticate_Internal(&opt_result, ssl_connection, client_socket, ErrorStr))
	{
		Disconnect_Internal(ssl_context, ssl_connection, client_socket, ErrorStr);
		return false;
	}

	while (!Commands->IsEmpty())
	{
		std::string command = Commands->Peek();

		int errcode;
		if (!SendPacket(ssl_connection, client_socket, command, ErrorStr))
		{
			Disconnect_Internal(ssl_context, ssl_connection, client_socket, ErrorStr);
			return false;
		}

		std::string message;
		if (!ReceivePacket(ssl_connection, client_socket, message, ErrorStr))
		{
			Disconnect_Internal(ssl_context, ssl_connection, client_socket, ErrorStr);
			return false;
		}

		size_t firstNewlinePos = message.find("\n");
		
		if (firstNewlinePos != std::string::npos)
		{
			std::string firstLine = message.substr(0, firstNewlinePos);

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

bool TrkConnectHelper::SendPacket(class TrkSSL* ssl_connection, int client_socket, const std::string message, std::string& error_msg)
{
	int totalSent = 0;
	int chunkSize = 1024;
	std::stringstream errorBuilder;

	while (totalSent < message.size())
	{
		std::chrono::milliseconds sleeptime(1);
		std::this_thread::sleep_for(sleeptime);
		int remaining = (size_t)message.size() - totalSent;
		int sendSize = std::min<int>(chunkSize, remaining);

		std::stringstream chunkHeaderBuilder;
		char hexString[4];
		std::sprintf(hexString, "%03X", sendSize);
		chunkHeaderBuilder << hexString << "\r\n";
		std::string chunkHeader = chunkHeaderBuilder.str();
		if (Send(ssl_connection, client_socket, chunkHeader, (size_t)chunkHeader.size()) <= 0)
		{
			errorBuilder << "Send Failed! (errno: "
#ifdef _WIN32
				<< WSAGetLastError()
#else
				<< errno
#endif
				<< ")";
			error_msg = errorBuilder.str();
			return false;
		}

		std::string chunkBody(message.begin() + totalSent, message.begin() + totalSent + sendSize);
		if (Send(ssl_connection, client_socket, chunkBody, (size_t)chunkBody.size()) <= 0)
		{
			errorBuilder << "Send Failed! (errno: "
#ifdef _WIN32
				<< WSAGetLastError()
#else
				<< errno
#endif
				<< ")";
			error_msg = errorBuilder.str();
			return false;
		}

		totalSent += sendSize;
	}

	if (Send(ssl_connection, client_socket, "000\r\n", 5) <= 0)
	{
		errorBuilder << "Send Failed! (errno: "
#ifdef _WIN32
			<< WSAGetLastError()
#else
			<< errno
#endif
			<< ")";
		error_msg = errorBuilder.str();
		return false;
	}

	return true;
}

bool TrkConnectHelper::ReceivePacket(class TrkSSL* ssl_connection, int client_socket, std::string& message, std::string& error_msg)
{
	std::stringstream buffer, receivedData;
	bool readingChunkHeader = true;
	int chunkSize = 5, bytesRead = 0;

	while (true)
	{
		bytesRead = 0;
		buffer.clear();
		while (chunkSize > bytesRead)
		{
			std::chrono::milliseconds sleeptime(1);
			std::this_thread::sleep_for(sleeptime);
			std::string newBuffer;
			int newBytesRead = Recv(ssl_connection, client_socket, newBuffer, chunkSize - bytesRead);
			if (newBytesRead < 0)
			{
				error_msg = "Receive failed unexceptedly.";
				message = "";
				return false;
			}
			else if (newBytesRead == 0)
			{
				continue;
			}
			bytesRead += newBytesRead;
			buffer << newBuffer;
		}

		if (readingChunkHeader)
		{
			std::string bufferStr = buffer.str();
			std::string chunkSizeStr(bufferStr.begin(), bufferStr.begin() + bufferStr.size() - 2);
			chunkSize = std::strtol(chunkSizeStr.c_str(), nullptr, 16);
			if (chunkSize == 0)
			{
				message = receivedData.str();
				return true;
			}
			readingChunkHeader = false;
		}
		else
		{
			std::string bufferStr = buffer.str();
			receivedData << std::string(bufferStr.begin(), bufferStr.begin() + chunkSize);
			chunkSize = 5;
			readingChunkHeader = true;
		}
	}

	return true;
}

bool TrkConnectHelper::Connect_Internal(TrkCliClientOptionResults& opt_result, TrkSSLCTX*& ssl_context, TrkSSL*& ssl_connection, int& client_socket, std::string& ErrorStr )
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

	char port_str[6];
	snprintf(port_str, sizeof(port_str), "%hu", opt_result.port);
	if (getaddrinfo(opt_result.ip_address.c_str(), port_str, &hints, &result) != 0)
	{
		std::stringstream ss;
#ifdef _WIN32
		ss << "getaddrinfo failed. (errno: " << WSAGetLastError() << ")";
		ErrorStr = ss.str();
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
			client_socket = static_cast<int>(INVALID_SOCKET);
		}
	}

	if (client_socket == INVALID_SOCKET) {
		freeaddrinfo(result);
		std::stringstream ss;
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
		ErrorStr = ss.str();
		return false;
	}

	freeaddrinfo(result);

	unsigned char response[5];
	response[0] = 0xEA;
	response[1] = 0xEB;
	response[2] = 0x00;
	response[3] = 0xCC;
	if (opt_result.trust)
	{
		response[4] = 0x01;
	}
	else
	{
		response[4] = 0x00;
	}

	send(client_socket, reinterpret_cast<char*>(response), sizeof(response), 0);

	std::stringstream errorBuilder;
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
			errorBuilder << "The server is not using TLS mode. No need for trust mode.";
		}
		else
		{
			errorBuilder << "The server is using TLS mode. Please enable trust mode and try again.";
		}

#ifdef _WIN32
		closesocket(client_socket);
		WSACleanup();
#else
		close(client_socket);
#endif
		ErrorStr = errorBuilder.str();
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
			if (opt_result.requested_command->command == "trust")
			{
				if (opt_result.last_certificate_fingerprint != "")
				{
					errorBuilder << "The server at '" << opt_result.server_url << "' requires your trust."
						<< "\nFingerprint: " << opt_result.last_certificate_fingerprint
						<< "\nDo you want to establish trust with this server? (y/n): ";
				}
				else
				{
					errorBuilder << "Trust has already been established for this server. ('" << opt_result.server_url << "')";
					delete ssl_context;
					delete ssl_connection;
					return false;
				}
			}
			else
			{
				if (opt_result.last_certificate_fingerprint != "")
				{
					errorBuilder << "Connection to '" << opt_result.server_url << "' could not be established."
						<< "\nThis may be your first attempt to connect to this server."
						<< "\nTo establish trust for this server, please run 'trk trust' command."
						<< "\n\nFingerprint: " << opt_result.last_certificate_fingerprint;
				}
				else
				{
					TrkSSLHelper::PrintErrors();
					errorBuilder << "SSL Error. Error code: " << TrkSSLHelper::GetError(ssl_connection);
				}
			}

			delete ssl_context;
			delete ssl_connection;

			ErrorStr = errorBuilder.str();
			return false;
		}
	}
	else
	{
		if (ssl_context != nullptr)
		{
			delete ssl_context;
		}
		if (ssl_connection = nullptr)
		{
			delete ssl_connection;
		}
	}

	return true;
}

bool TrkConnectHelper::Disconnect_Internal(TrkSSLCTX* ssl_context, TrkSSL* ssl_connection, int client_socket, std::string& error_msg)
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

bool TrkConnectHelper::Authenticate_Internal(class TrkCliClientOptionResults* opt_result, TrkSSL* ssl_connection, int client_socket, std::string& error_msg, bool retry)
{
	std::string ticket, errmsg, result;
	std::stringstream authBuilder, errorBuilder;
	authBuilder << "Username="
		<< opt_result->username
		<< ";";

	if (TrkPasswdHelper::CheckSessionFileExists())
	{
		ticket = TrkPasswdHelper::GetSessionTicketByServerURL(opt_result->server_url);
		if (ticket.size() > 0 && !retry)
		{
			authBuilder << "Ticket="
				<< ticket;
		}
		else
		{
			std::cout << "Enter Password: ";
			std::string input;
			std::getline(std::cin, input);
			std::string passwd(input.c_str());

			authBuilder << "Password="
				<< TrkCryptoHelper::SHA256(passwd);
		}
	}

	if (!SendPacket(ssl_connection, client_socket, authBuilder.str(), errmsg))
	{
		errorBuilder << "Authentication failed: " << errmsg;
		error_msg = errorBuilder.str();
		return false;
	}

	if (!ReceivePacket(ssl_connection, client_socket, result, errmsg))
	{
		errorBuilder << "Authentication failed: " << errmsg;
		error_msg = errorBuilder.str();
		return false;
	}

	size_t firstNewlinePos = result.find("\n");
	std::string firstLine = (firstNewlinePos != std::string::npos) ? result.substr(0, firstNewlinePos) : result;

	if (firstLine == "OK")
	{
		if (result != "OK\n")
		{
			std::string newticket = result.substr(firstNewlinePos + 1);
			if (ticket != newticket && ticket != "")
			{
				TrkPasswdHelper::ChangeSessionTicket(newticket, opt_result->server_url);
			}
			else
			{
				TrkPasswdHelper::SaveSessionTicket(newticket, opt_result->server_url);
			}
		}
		return true;
	}
	else if (firstLine == "ERROR" && ticket != "" && !retry)
	{
		if (result.substr(firstNewlinePos + 1) == "Ticket Invalid")
		{
			return Authenticate_Internal(opt_result, ssl_connection, client_socket, error_msg, true);
		}
	}

	error_msg = "Authentication failed: Login failed.";
	return false;
}

int TrkConnectHelper::Send(TrkSSL* ssl_connection, int client_socket, std::string buf, int len)
{
	if (ssl_connection != nullptr)
	{
		return TrkSSLHelper::Write(ssl_connection, buf, len);
	}
	else
	{
		return send(client_socket, buf.c_str(), len, 0);
	}
}

int TrkConnectHelper::Recv(TrkSSL* ssl_connection, int client_socket, std::string& buf, int len)
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
			buf = std::string(internal_strings, internal_strings + bytes_read);
		}
		else
		{
			buf = std::string("");
		}

		return bytes_read;
	}
}
