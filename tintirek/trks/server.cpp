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
#elif
#endif


#include "trk_version.h"
#include "logger.h"
#include "server.h"



bool TrkServer::HandleCommand(const char* Message, const char*& Returned)
{
	std::string command, message = std::string(Message);
	std::vector<std::string> parameters;
	size_t pos = message.find('?');
	if (pos != std::string::npos)
	{
		command = message.substr(0, pos);

		size_t current = pos + 1;
		while ((pos = message.find('?', current)) != std::string::npos)
		{
			parameters.push_back(message.substr(current, pos));
			current = pos + 1;
		}

		parameters.push_back(message.substr(current));
	}
	else
	{
		command = message;
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

		std::stringstream os;
		os << "OK\n"
			<< "serverversion=" << verinfo.getFullVersionInfo() << ";"
			<< "serveruptime="
			<< std::setfill('0') << std::setw(2) << hours << ":"
			<< std::setw(2) << minutes << ":"
			<< std::setw(2) << seconds
			<< ";"
			<< "servertime=" << GetTimestamp("%Y/%m/%d %H:%M:%S %z");

		Returned = strdup(os.str().c_str());
		return true;
	}

	Returned = strdup("Command not found");
	return false;
}

bool TrkServer::SendPacket(int client_socket, const char* message, int& error_code)
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

bool TrkServer::ReceivePacket(int client_socket, const char*& message, const char*& error_msg)
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