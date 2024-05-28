/*
 *	server.h
 *
 *	Declarations for the Tintirek Server
 */

#ifndef SERVER_H
#define SERVER_H


#include <iostream>
#include <memory>
#include <mutex>

#include "config.h"
#include "crypto.h"


class TrkClientInfo
{
public:
	TrkClientInfo(struct sockaddr_in* ClientInfo = nullptr, int Socket = -1, TrkSSL* SSLSocket = nullptr, std::string ip_port = "")
		: client_info(ClientInfo)
		, client_socket(Socket)
		, client_ssl_socket(SSLSocket)
		, client_connection_info(ip_port)
	{ }

	~TrkClientInfo()
	{
		if (next_client != nullptr)
		{
			delete next_client;
		}

		if (client_ssl_socket != nullptr)
		{
			delete client_ssl_socket;
		}
	}

	/*	Mutex object for async processes */
	std::unique_ptr<std::mutex> mutex;
	/*	Client info */
	struct sockaddr_in* client_info;
	/*	Client socket number */
	const int client_socket = -1;
	/*	Client SSL socket information */
	TrkSSL* client_ssl_socket;
	/*	Client connection info (IP:PORT) */
	std::string client_connection_info = "";
	/*	Client username */
	std::string username = "";

protected:
	/* Linked list's next element */
	TrkClientInfo* next_client = nullptr;

public:
	/* Get next client */
	TrkClientInfo* GetNext() const { return next_client; }
	/* Set next client*/
	void SetNext(TrkClientInfo* NewClient) { next_client = NewClient; }


	/* Class for option flag iterator */
	class TrkClientInfoIterator
	{
	private:
		/* Current element */
		TrkClientInfo* current;

	public:
		TrkClientInfoIterator(TrkClientInfo* StartNode)
			: current(StartNode)
		{ }

		/* Checks given element with our current element */
		bool operator != (const TrkClientInfoIterator& other) const {
			return current != other.current;
		}

		/* Returns current element when called as pointer */
		TrkClientInfo* operator*() const {
			return current;
		}

		/* Increments our current element */
		TrkClientInfoIterator& operator++() {
			current = current->GetNext();
			return *this;
		}
	};



	/* begin function override */
	TrkClientInfoIterator begin() {
		return TrkClientInfoIterator(this);
	}

	/* end function override */
	TrkClientInfoIterator end() {
		return TrkClientInfoIterator(nullptr);
	}
};



/*
 *	Server subsystem for TRKS (Tintirek's Server)
 * 
 *	It is the program loop OF Tintirek's Server software.
 */
class TrkServer
{
public:
    TrkServer(int Port, class TrkCliServerOptionResults* Options) { }

	/*	Starts the server and checks the listening status */
	virtual bool Init(std::string& ErrorStr) = 0;
	/*	Updates the server's status, edits connections and audits users */
	virtual bool Run(std::string& ErrorStr) = 0;
	/*	Before the program closes, it performs server-related cleaning and
		resets the connections of connected users */
	virtual bool Cleanup(std::string& ErrorStr) = 0;

	/*  Handle clients */
	virtual void HandleConnection(TrkClientInfo* client_info);
	/*	Disconnects client */
	virtual void Disconnect(TrkClientInfo* client_info);
	/*	Process authentication */
	virtual bool Authenticate(TrkClientInfo* client_info, std::string& error_msg, bool retry = false);
	/*  Handles clients by iteration */
	virtual bool HandleConnectionMultiple(TrkClientInfo* client_info, std::string& error_str);
	/*	Handle commands */
	virtual bool HandleCommand(TrkClientInfo* client_info, const std::string Message, std::string& Returned);

	/*	Sends packet to client as chunked data */
	virtual bool SendPacket(TrkClientInfo* client_info, const std::string message, std::string& error_str);
	/*	Recovers packet from all chunk data from client */
	virtual bool ReceivePacket(TrkClientInfo* client_info, std::string& message, std::string& error_str);

	/* Send implementation with SSL and non-SSL */
	virtual int Send(TrkClientInfo* clientInfo, std::string buf, int len, bool use_ssl);
	/* Recv implementation with SSL and non-SSL */
	virtual int Recv(TrkClientInfo* clientInfo, std::string& buf, int len, bool use_ssl);

protected:
	/*	Server's port number */
	int port_number;
	/*	Queue length held during connection waiting */
	static constexpr int backlog = 5;
	/*	The size of the data buffer to be used for Read/Write operations */
	static constexpr int buffer_size = 2048;

	/*	Server socket identifier */
	int server_socket = -1;
	/*	Maximum amount of sockets to monitor */
	int max_socket = -1;
	/*	Main socket set */
#if _WIN32
	struct fd_set* master;
#endif

	/*  List of clients */
	class TrkClientInfo* list = nullptr;

	/*	Data of server program */
	TrkCliServerOptionResults* opt_result = nullptr;

	/* SSL object */
	TrkSSLCTX* ssl_ctx;

public:
	/*	If element is unique, addd new element to the end */
	bool AppendToListUnique(TrkClientInfo* NewElement)
	{
		if (list == nullptr)
		{
			list = NewElement;
			NewElement->SetNext(nullptr);
			return true;
		}

		TrkClientInfo* current = list;

		while (current->GetNext() != nullptr)
		{
			if (current->GetNext() == NewElement)
			{
				return false;
			}

			current = current->GetNext();
		}

		current->SetNext(NewElement);
		NewElement->SetNext(nullptr);
		return true;
	}

	/*	Remove element from the list */
	bool RemoveFromList(TrkClientInfo* RemoveItem)
	{
		TrkClientInfo* current = list;
		TrkClientInfo* previous = nullptr;

		if (current == RemoveItem)
		{
			list = current->GetNext();
			current->SetNext(nullptr);
			current->mutex->unlock();
			delete current;
			return true;
		}

		while (current)
		{
			if (current == RemoveItem)
			{
				previous->SetNext(current->GetNext());
				current->SetNext(nullptr);
				current->mutex->unlock();
				delete current;
				return true;
			}

			previous = current;
			current = current->GetNext();
		}

		return false;
	}
};


#ifdef _WIN32

class TrkWindowsServer : public TrkServer
{
public:
    TrkWindowsServer(int Port, TrkCliServerOptionResults* Options);

	virtual bool Init(std::string& ErrorStr) override;
	virtual bool Run(std::string& ErrorStr) override;
	virtual bool Cleanup(std::string& ErrorStr) override;
};

#elif __APPLE__

class TrkMacOSServer : public TrkServer
{
public:
    TrkMacOSServer(int Port, TrkCliServerOptionResults* Options);

	virtual bool Init(std::string& ErrorStr) override;
	virtual bool Run(std::string& ErrorStr) override;
	virtual bool Cleanup(std::string& ErrorStr) override;
};

#elif __linux__

class TrkLinuxServer : public TrkServer
{
public:
    TrkLinuxServer(int Port, TrkCliServerOptionResults* Options);

	virtual bool Init(std::string& ErrorStr) override;
	virtual bool Run(std::string& ErrorStr) override;
	virtual bool Cleanup(std::string& ErrorStr) override;
};

#endif



#endif /* SERVER_H */
