/*
 *	server.cpp
 *
 *	Declarations for the Tintirek Server
 */


#include <sstream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif


#include "trk_version.h"
#include "logger.h"
#include "server.h"



void TrkServer::HandleConnection(TrkClientInfo* client_info)
{
	TrkString error_str;
	TrkString message;
	client_info->mutex = std::make_unique<std::mutex>();

	if (!ReceivePacket(client_info, message, error_str))
	{
		client_info->mutex->lock();
		LOG_ERR("Error with " << client_info->client_connection_info << ": " << error_str);
#if WIN32
		closesocket(client_info->client_socket);
#else
		close(client_info->client_socket);
#endif
		RemoveFromList(client_info);
		return;
	}

	TrkString returned;
	HandleCommand(client_info, message, returned);

	int errcode;
	if (!SendPacket(client_info, returned, errcode))
	{
		std::stringstream os;
		os << "Send error! (errno: " << errcode << ")";
		LOG_ERR("Error with " << client_info->client_connection_info << ": " << os.str());
	}

	client_info->mutex->lock();
#if WIN32
		closesocket(client_info->client_socket);
#else
		close(client_info->client_socket);
#endif
	LOG_OUT("Connection closed: " << client_info->client_connection_info);
	RemoveFromList(client_info);
}

bool TrkServer::HandleConnectionMultiple(TrkClientInfo* client_info, TrkString& error_str)
{
	TrkString message;
	client_info->mutex = std::make_unique<std::mutex>();

	if (!ReceivePacket(client_info, message, error_str))
	{
		return false;
	}

	TrkString returned;
	if (!HandleCommand(client_info, message, returned))
	{
		error_str = returned;
		return false;
	}

	if (strcmp(returned, "NONE\n") != 0)
	{
		int errcode;
		if (!SendPacket(client_info, returned, errcode))
		{
			TrkString ss;
			ss << "Send error! (errno: " << errcode << ")";
			error_str = ss;
			return false;
		}
	}

	return true;
}

bool TrkServer::HandleCommand(TrkClientInfo* client_info, const TrkString Message, TrkString& Returned)
{
	TrkString command;
	std::vector<TrkString> parameters;
	size_t pos = Message.find("?");
	if (pos != std::string::npos)
	{
		command = Message.substr(0, pos);

		size_t current = pos + 1;
		while ((pos = Message.find("?", current)) != std::string::npos)
		{
			parameters.push_back(Message.substr(current, pos));
			current = pos + 1;
		}

		parameters.push_back(Message.substr(current));
	}
	else
	{
		command = Message;
	}

	if (command == "GetInformation")
	{
		TRK_VERSION_DEFINE_PROGRAM(verinfo);

		time_t currentTime;
		time(&currentTime);

		time_t timeDiff = currentTime - (opt_result->start_timestamp / 1000);
		int hours = timeDiff / 3600;
		int minutes = (timeDiff % 3600) / 60;
		int seconds = timeDiff % 60;

		TrkString ss;
		ss << "OK\n"
			<< "serverversion=" << verinfo.getFullVersionInfo() << ";"
			<< "serveruptime="
			<< std::setfill('0') << std::setw(2) << hours << ":"
			<< std::setw(2) << minutes << ":"
			<< std::setw(2) << seconds
			<< ";"
			<< "servertime=" << GetTimestamp("%Y/%m/%d %H:%M:%S %z");

		Returned = ss;
		return true;
	}
	else if (command == "MultipleCommands")
	{
		int errcode;
		if (!SendPacket(client_info, "OK\n", errcode))
		{
			Returned = "ERROR\n";
			return false;
		}

		for (int i = 0; i < TrkString::stoi(parameters[0]); ++i)
		{
			TrkString error_str;
			if (!HandleConnectionMultiple(client_info, error_str))
			{
				TrkString ss;
				ss << "ERROR\n" << error_str;
				Returned = ss;
				return false;
			}
		}

		Returned = "NONE\n";
		return true;
	}
	else if (command == "Add")
	{
		TrkString ss;
		ss << "OK\n" << parameters[0] << " -- " << "opened for client";
		Returned = ss;
		return true;
	}
	else if (command == "Edit")
	{
		TrkString ss;
		ss << "OK\n" << parameters[0] << " -- " << "already opened for client";
		Returned = ss;
		return true;
	}

	Returned = "ERROR\nCommand not found";
	return false;
}

bool TrkServer::SendPacket(TrkClientInfo* client_info, const TrkString message, int& error_code)
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
		if (Send(client_info, chunkHeader, chunkHeader.size(), 0) <= 0)
		{
#ifdef _WIN32
			error_code = WSAGetLastError();
#endif
			return false;
		}

		TrkString sendData(message.begin() + totalSent, message.begin() + totalSent + sendSize);
		if (Send(client_info, sendData, sendData.size(), client_info->client_ssl_socket != nullptr) <= 0)
		{
#ifdef _WIN32
			error_code = WSAGetLastError();
#endif
			return false;
		}

		if (Send(client_info, "\r\n", 2, client_info->client_ssl_socket != nullptr) <= 0)
		{
#ifdef _WIN32
			error_code = WSAGetLastError();
#endif
			return false;
		}

		totalSent += sendSize;
	}

	if (Send(client_info, "0\r\n\r\n", 5, client_info->client_ssl_socket != nullptr) <= 0)
	{
#ifdef _WIN32
		error_code = WSAGetLastError();
#endif
		return false;
	}

	return true;
}

bool TrkServer::ReceivePacket(TrkClientInfo* client_info, TrkString& message, TrkString& error_msg)
{
	TrkString buffer, receivedData;
	bool readingChunkSize = true;
	int chunkSize = 0, lastIndex = 0;

	while (true)
	{
		std::chrono::milliseconds sleeptime(100);
		std::this_thread::sleep_for(sleeptime);
		int bytesRead = Recv(client_info, buffer, 1024, client_info->client_ssl_socket != nullptr);
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
				int remainingBytes = bytesRead - lastIndex;
				
				if (remainingBytes >= chunkSize) {
					receivedData << TrkString(buffer.begin() + lastIndex, buffer.begin() + lastIndex + chunkSize);
					lastIndex += chunkSize;
					chunkSize = 0;
					readingChunkSize = true;
				}
				else
				{
					receivedData << TrkString(buffer.begin() + lastIndex, buffer.end());
					chunkSize -= remainingBytes;
					lastIndex = bytesRead;
				}
			}
		}
	}

	return true;
}

int TrkServer::Send(TrkClientInfo* clientInfo, TrkString buf, int len, bool use_ssl)
{
	if (use_ssl)
	{
		return TrkSSLHelper::Write(clientInfo->client_ssl_socket, buf, len);
	}
	else
	{
		return send(clientInfo->client_socket, buf, len, 0);
	}
}

int TrkServer::Recv(TrkClientInfo* clientInfo, TrkString& buf, int len, bool use_ssl)
{
	if (use_ssl)
	{
		return TrkSSLHelper::Read(clientInfo->client_ssl_socket, buf, len);
	}
	else
	{
		char internal_strings[1024];
		int bytes_read = recv(clientInfo->client_socket, internal_strings, len, 0);
		buf = TrkString(internal_strings, internal_strings + bytes_read);
		return bytes_read;
	}
}
