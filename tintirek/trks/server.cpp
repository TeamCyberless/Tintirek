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
#include <regex>

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
#include "database.h"
#include "logger.h"
#include "server.h"



void TrkServer::HandleConnection(TrkClientInfo* client_info)
{
	std::string error_str, message;
	client_info->mutex = std::make_unique<std::mutex>();

	if (!Authenticate(client_info, error_str))
	{
		LOG_OUT("Error in authentication (" << client_info->client_connection_info << "): " << error_str);
		Disconnect(client_info);
		return;
	}

	if (!ReceivePacket(client_info, message, error_str))
	{
		Disconnect(client_info);
		return;
	}

	if (message != "Close")
	{
		std::string returned;
		HandleCommand(client_info, message, returned);

		int errcode;
		if (!SendPacket(client_info, returned, error_str))
		{
			LOG_ERR("Error with " << client_info->client_connection_info << ": " << error_str);
		}
	}

	Disconnect(client_info);
}

void TrkServer::Disconnect(TrkClientInfo* client_info)
{
	std::chrono::milliseconds sleeptime(1000);
	std::this_thread::sleep_for(sleeptime);

	client_info->mutex->lock();
#if _WIN32
	closesocket(client_info->client_socket);
#else
	close(client_info->client_socket);
#endif
	LOG_OUT("Connection closed: " << client_info->client_connection_info);
	RemoveFromList(client_info);
}

bool TrkServer::Authenticate(TrkClientInfo* client_info, std::string& error_msg, bool retry)
{
	std::string error_str, message, username, passwd;
	bool ticketauth = false;
	if (!ReceivePacket(client_info, message, error_str))
	{
		return false;
	}

	while (message.size() > 0)
	{
		size_t pos = message.find(";");
		if (pos == std::string::npos)
		{
			pos = message.size();
		}

		const std::string token = message.substr(0, pos);
		size_t equalPos = token.find("=");
		if (equalPos != std::string::npos)
		{
			const std::string key = token.substr(0, equalPos);
			const std::string value = token.substr(equalPos + 1);

			if (key == "Username")
			{
				username = value;
			}
			else if (key == "Password")
			{
				passwd = value;
				ticketauth = false;
			}
			else if (key == "Ticket")
			{
				passwd = value;
				ticketauth = true;
			}
		}
		message.erase(0, pos + 1);
	}

	client_info->username = username;

	if (username != "" && (!ticketauth || passwd != ""))
	{
		if (!ticketauth)
		{
			int db_itr;
			std::string db_passwd, db_salt;
			if (GetUserPasswdFromDB(username, db_passwd, db_salt, db_itr))
			{
				std::string val = passwd;
				for (int i = 0; i < db_itr; i++)
				{
					val = TrkCryptoHelper::SHA256(val + "::" + db_salt);
				}

				if (val == db_passwd)
				{
					std::stringstream newTicketBuilder;
					newTicketBuilder << "Tintirek::";
					newTicketBuilder << TRK_VERSION << "::";
					newTicketBuilder << static_cast<int64_t>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())) << "::";
					newTicketBuilder << db_passwd << "::";
					newTicketBuilder << "EndOfTicket";
					std::string newTicket = newTicketBuilder.str();

					newTicket = TrkCryptoHelper::SHA256(newTicket, ":");
					if (UpdateUserTicketDB(username, newTicket))
					{
						std::stringstream str;
						str << "OK\n" << newTicket;
						if (!SendPacket(client_info, str.str(), error_msg))
						{
							return false;
						}
						return true;
					}

					std::stringstream errorBuilder;
					errorBuilder << "Something went wrong with updating ticket value. This should not have happened.";
					error_msg = errorBuilder.str();
					return false;
				}
			}
		}
		else
		{
			int64_t db_ticket_endtime;
			std::string db_ticket;
			if (GetUserTicketFromDB(username, db_ticket, db_ticket_endtime))
			{
				if (db_ticket == passwd)
				{
					int64_t unix_ticket_end = static_cast<int64_t>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
					if (unix_ticket_end < db_ticket_endtime)
					{
						std::string str = "OK\n";
						if (!SendPacket(client_info, str, error_msg))
						{
							return false;
						}
						return true;
					}
				}

				std::string str = "ERROR\nTicket Invalid";
				if (!SendPacket(client_info, str, error_msg))
				{
					return false;
				}
				if (retry)
				{
					error_msg = "Retry count exceeded.";
					return false;
				}
				return Authenticate(client_info, error_msg, true);
			}
		}

		std::stringstream errorBuilder;
		errorBuilder << "Username or password invalid. (Username: " << username << ")";
		error_msg = errorBuilder.str();
	}

	std::string str = "ERROR\n", empty;
	SendPacket(client_info, str, empty);
	return false;
}

bool TrkServer::HandleConnectionMultiple(TrkClientInfo* client_info, std::string& error_str)
{
	std::string message;
	client_info->mutex = std::make_unique<std::mutex>();

	if (!ReceivePacket(client_info, message, error_str))
	{
		return false;
	}

	std::string returned;
	if (!HandleCommand(client_info, message, returned))
	{
		error_str = returned;
		return false;
	}

	if (strcmp(returned.c_str(), "NONE\n") != 0)
	{
		if (!SendPacket(client_info, returned, error_str))
		{
			return false;
		}
	}

	return true;
}

bool TrkServer::HandleCommand(TrkClientInfo* client_info, const std::string Message, std::string& Returned)
{
	std::string command;
	std::vector<std::string> parameters;
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
		TRK_VERSION_DEFINE(verinfo);

		time_t currentTime;
		time(&currentTime);

        time_t timeDiff = currentTime - (opt_result->start_timestamp / 1000);
		int hours = timeDiff / 3600;
		int minutes = (timeDiff % 3600) / 60;
		int seconds = timeDiff % 60;

		std::stringstream ss;
        ss << "OK\n"
           << "serverversion=" << trk_get_full_version_info(&verinfo) << ";"
			<< "serveruptime="
			<< std::setfill('0') << std::setw(2) << hours << ":"
			<< std::setw(2) << minutes << ":"
			<< std::setw(2) << seconds
			<< ";"
			<< "servertime=" << GetTimestamp("%Y/%m/%d %H:%M:%S %z");

		Returned = ss.str();
		return true;
	}
	else if (command == "Logout")
	{
		ResetUserTicketDB(client_info->username);
		std::stringstream ss;
		ss << "OK\n";
		Returned = ss.str();
		return true;
	}
	else if (command == "MultipleCommands")
	{
		if (!SendPacket(client_info, "OK\n", Returned))
		{
			std::stringstream ss;
			ss << "ERROR\n" << Returned;
			Returned = ss.str();
			return false;
		}

		for (int i = 0; i < std::strtol(parameters[0].c_str(), nullptr, 10); ++i)
		{
			std::string error_str;
			if (!HandleConnectionMultiple(client_info, error_str))
			{
				std::stringstream ss;
				ss << "ERROR\n" << error_str;
				Returned = ss.str();
				return false;
			}
		}

		Returned = "NONE\n";
		return true;
	}
	else if (command == "Add")
	{
		std::stringstream ss;
		ss << "OK\n" << parameters[0] << " -- " << "opened for client";
		Returned = ss.str();
		return true;
	}
	else if (command == "Edit")
	{
		std::stringstream ss;
		ss << "OK\n" << parameters[0] << " -- " << "already opened for client";
		Returned = ss.str();
		return true;
	}

	Returned = "ERROR\nCommand not found";
	return false;
}

bool TrkServer::SendPacket(TrkClientInfo* client_info, const std::string message, std::string& error_str)
{
	int totalSent = 0;
	int chunkSize = 1024;

	while (totalSent < message.size())
	{
		std::chrono::milliseconds sleeptime(1);
		std::this_thread::sleep_for(sleeptime);
		int remaining = message.size() - totalSent;
		int sendSize = std::min<int>(chunkSize, remaining);

		std::stringstream chunkHeaderBuilder;
		char hexString[4];
		std::sprintf(hexString, "%03X", sendSize);
		chunkHeaderBuilder << hexString << "\r\n";
		std::string chunkHeader = chunkHeaderBuilder.str();
		if (Send(client_info, chunkHeader, chunkHeader.size(), client_info->client_ssl_socket != nullptr) <= 0)
		{
			std::stringstream ss;
			ss << "Send Failed! (errno: "
#ifdef _WIN32
				<< WSAGetLastError()
#else
				<< errno
#endif
				<< ")";
			error_str = ss.str();
			return false;
		}

		std::string chunkBody(message.begin() + totalSent, message.begin() + totalSent + sendSize);
		if (Send(client_info, chunkBody, chunkBody.size(), client_info->client_ssl_socket != nullptr) <= 0)
		{
			std::stringstream ss;
			ss << "Send Failed! (errno: "
#ifdef _WIN32
				<< WSAGetLastError()
#else
				<< errno
#endif
				<< ")";
			error_str = ss.str();
			return false;
		}

		totalSent += sendSize;
	}

	if (Send(client_info, "000\r\n", 5, client_info->client_ssl_socket != nullptr) <= 0)
	{
		std::stringstream ss;
		ss << "Send Failed! (errno: "
#ifdef _WIN32
			<< WSAGetLastError()
#else
			<< errno
#endif
			<< ")";
		error_str = ss.str();
		return false;
	}

	return true;
}

bool TrkServer::ReceivePacket(TrkClientInfo* client_info, std::string& message, std::string& error_str)
{
	std::stringstream buffer, receivedData;
	bool readingChunkHeader = true;
	int chunkSize = 5, bytesRead;

	while (true)
	{
		bytesRead = 0;
		buffer.clear();
		while (chunkSize > bytesRead)
		{
			std::chrono::milliseconds sleeptime(1);
			std::this_thread::sleep_for(sleeptime);
			std::string newBuffer;
			int newBytesRead = Recv(client_info, newBuffer, chunkSize - bytesRead, client_info->client_ssl_socket != nullptr);
			if (newBytesRead < 0)
			{
				error_str = "Receive failed unexceptedly.";
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

int TrkServer::Send(TrkClientInfo* clientInfo, std::string buf, int len, bool use_ssl)
{
	if (use_ssl)
	{
		return TrkSSLHelper::Write(clientInfo->client_ssl_socket, buf, len);
	}
	else
	{
		return send(clientInfo->client_socket, buf.c_str(), len, 0);
	}
}

int TrkServer::Recv(TrkClientInfo* clientInfo, std::string& buf, int len, bool use_ssl)
{
	if (use_ssl)
	{
		return TrkSSLHelper::Read(clientInfo->client_ssl_socket, buf, len);
	}
	else
	{
		char internal_strings[1024];
		int bytes_read = recv(clientInfo->client_socket, internal_strings, len, 0);

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
