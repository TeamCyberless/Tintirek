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
	TrkString error_str, message;
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
		TrkString returned;
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

bool TrkServer::Authenticate(TrkClientInfo* client_info, TrkString& error_msg, bool retry)
{
	TrkString error_str, message, username, passwd;
	bool ticketauth = false;
	if (!ReceivePacket(client_info, message, error_str))
	{
		return false;
	}

	while (message.size() > 0)
	{
		size_t pos = message.find(";");
		if (pos == TrkString::npos)
		{
			pos = message.size();
		}

		const TrkString token = message.substr(0, pos);
		size_t equalPos = token.find("=");
		if (equalPos != TrkString::npos)
		{
			const TrkString key = token.substr(0, equalPos);
			const TrkString value = token.substr(equalPos + 1);

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
			TrkString db_passwd, db_salt;
			if (GetUserPasswdFromDB(username, db_passwd, db_salt, db_itr))
			{
				TrkString val = passwd;
				for (int i = 0; i < db_itr; i++)
				{
					val = TrkCryptoHelper::SHA256(val + "::" + db_salt);
				}

				if (val == db_passwd)
				{
					TrkString newTicket = "Tintirek::";
					newTicket << TRK_VERSION << "::";
					newTicket << static_cast<int64_t>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())) << "::";
					newTicket << db_passwd << "::";
					newTicket << "EndOfTicket";

					newTicket = TrkCryptoHelper::SHA256(newTicket, ":");
					if (UpdateUserTicketDB(username, newTicket))
					{
						TrkString str = "OK\n";
						str << newTicket;
						if (!SendPacket(client_info, str, error_msg))
						{
							return false;
						}
						return true;
					}

					error_msg << "Something went wrong with updating ticket value. This should not have happened.";
					return false;
				}
			}
		}
		else
		{
			int64_t db_ticket_endtime;
			TrkString db_ticket;
			if (GetUserTicketFromDB(username, db_ticket, db_ticket_endtime))
			{
				if (db_ticket == passwd)
				{
					int64_t unix_ticket_end = static_cast<int64_t>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
					if (unix_ticket_end < db_ticket_endtime)
					{
						TrkString str = "OK\n";
						if (!SendPacket(client_info, str, error_msg))
						{
							return false;
						}
						return true;
					}
				}

				TrkString str = "ERROR\nTicket Invalid";
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

		error_msg << "Username or password invalid. (Username: " << username << ")";
	}

	TrkString str = "ERROR\n", empty;
	SendPacket(client_info, str, empty);
	return false;
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
		if (!SendPacket(client_info, returned, error_str))
		{
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
		TRK_VERSION_DEFINE(verinfo);

		time_t currentTime;
		time(&currentTime);

        time_t timeDiff = currentTime - (opt_result->start_timestamp / 1000);
		int hours = timeDiff / 3600;
		int minutes = (timeDiff % 3600) / 60;
		int seconds = timeDiff % 60;

		TrkString ss;
        ss << "OK\n"
           << "serverversion=" << trk_get_full_version_info(&verinfo) << ";"
			<< "serveruptime="
			<< std::setfill('0') << std::setw(2) << hours << ":"
			<< std::setw(2) << minutes << ":"
			<< std::setw(2) << seconds
			<< ";"
			<< "servertime=" << GetTimestamp("%Y/%m/%d %H:%M:%S %z");

		Returned = ss;
		return true;
	}
	else if (command == "Logout")
	{
		ResetUserTicketDB(client_info->username);
		Returned << "OK\n";
		return true;
	}
	else if (command == "MultipleCommands")
	{
		if (!SendPacket(client_info, "OK\n", Returned))
		{
			TrkString ss;
			ss << "ERROR\n" << Returned;
			Returned = ss;
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
		Returned << "OK\n" << parameters[0] << " -- " << "opened for client";
		return true;
	}
	else if (command == "Edit")
	{
		Returned << "OK\n" << parameters[0] << " -- " << "already opened for client";
		return true;
	}

	Returned = "ERROR\nCommand not found";
	return false;
}

bool TrkServer::SendPacket(TrkClientInfo* client_info, const TrkString message, TrkString& error_str)
{
	int totalSent = 0;
	int chunkSize = 1024;

	while (totalSent < message.size())
	{
		std::chrono::milliseconds sleeptime(1);
		std::this_thread::sleep_for(sleeptime);
		int remaining = message.size() - totalSent;
		int sendSize = std::min<int>(chunkSize, remaining);

		TrkString chunkHeader;
		char hexString[4];
		std::sprintf(hexString, "%03X", sendSize);
		chunkHeader << hexString << "\r\n";
		if (Send(client_info, chunkHeader, chunkHeader.size(), client_info->client_ssl_socket != nullptr) <= 0)
		{
			error_str << "Send Failed! (errno: "
#ifdef _WIN32
				<< WSAGetLastError()
#else
				<< errno
#endif
				<< ")";
			return false;
		}

		TrkString chunkBody(message.begin() + totalSent, message.begin() + totalSent + sendSize);
		if (Send(client_info, chunkBody, chunkBody.size(), client_info->client_ssl_socket != nullptr) <= 0)
		{
			error_str << "Send Failed! (errno: "
#ifdef _WIN32
				<< WSAGetLastError()
#else
				<< errno
#endif
				<< ")";
			return false;
		}

		totalSent += sendSize;
	}

	if (Send(client_info, "000\r\n", 5, client_info->client_ssl_socket != nullptr) <= 0)
	{
		error_str << "Send Failed! (errno: "
#ifdef _WIN32
			<< WSAGetLastError()
#else
			<< errno
#endif
			<< ")";
		return false;
	}

	return true;
}

bool TrkServer::ReceivePacket(TrkClientInfo* client_info, TrkString& message, TrkString& error_str)
{
	TrkString buffer, receivedData = "";
	bool readingChunkHeader = true;
	int chunkSize = 5, bytesRead;

	while (true)
	{
		bytesRead = 0;
		buffer = "";
		while (chunkSize > bytesRead)
		{
			std::chrono::milliseconds sleeptime(1);
			std::this_thread::sleep_for(sleeptime);
			TrkString newBuffer;
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
			TrkString chunkSizeStr(buffer.begin(), buffer.begin() + buffer.size() - 2);
			chunkSize = TrkString::stoi(chunkSizeStr, 16);
			if (chunkSize == 0)
			{
				message = receivedData;
				return true;
			}
			readingChunkHeader = false;
		}
		else
		{
			receivedData << TrkString(buffer.begin(), buffer.begin() + chunkSize);
			chunkSize = 5;
			readingChunkHeader = true;
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
