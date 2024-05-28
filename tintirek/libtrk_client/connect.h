/*
 *	connect.h
 * 
 *	Tintirek's client-side connection helpers
 */


#ifndef CONNECT_H
#define CONNECT_H


#include "crypto.h"


 /* Connection Helper Class */
class TrkConnectHelper
{
public:
	/* Send command to server */
	static bool SendCommand(class TrkCliClientOptionResults& opt_result, const std::string Command, std::string& ErrorStr, std::string& Returned);
	/* Send multiple commands to server */
	static bool SendCommandMultiple(class TrkCliClientOptionResults& opt_result, class TrkCommandQueue* Commands, std::string& ErrorStr, std::string& Returned);

protected:
	/* Sends packet to client as chunked data */
	static bool SendPacket(TrkSSL* ssl_connection, int client_socket, const std::string message, std::string& error_msg);
	/* Recovers packet from all chunk data from client */
	static bool ReceivePacket(TrkSSL* ssl_connection, int client_socket, std::string& message, std::string& error_msg);
	/* Internal code for connecting to the server */
	static bool Connect_Internal(class TrkCliClientOptionResults& opt_result, TrkSSLCTX*& ssl_context, TrkSSL*& ssl_connection, int& client_socket, std::string& ErrorStr);
	/* Internal code for disconnecting from the server */
	static bool Disconnect_Internal(TrkSSLCTX* ssl_context, TrkSSL* ssl_connection, int client_socket, std::string& error_msg);
	/* Internal code for authentication */
	static bool Authenticate_Internal(class TrkCliClientOptionResults* opt_result, TrkSSL* ssl_connection, int client_socket, std::string& error_msg, bool retry = false);
	
	/* Send implementation with SSL and non-SSL */
	static int Send(TrkSSL* ssl_connection, int client_socket, std::string buf, int len);
	/* Recv implementation with SSL and non-SSL */
	static int Recv(TrkSSL* ssl_connection, int client_socket, std::string& buf, int len);
};


/* Command Queue Helper Class */
class TrkCommandQueue
{
public:
	TrkCommandQueue(std::string Command)
		: command(Command)
		, next_queue(nullptr)
		, queue_front(nullptr)
		, queue_rear(nullptr)
		, size(0)
	{ }

	~TrkCommandQueue()
	{
		while (!IsEmpty())
		{
			Dequeue();
		}
	}

	/* Enqueue new element */
	void Enqueue(TrkCommandQueue* NewCommand)
	{
		if (IsEmpty())
		{
			queue_front = queue_rear = NewCommand;
		}
		else
		{
			queue_rear->next_queue = NewCommand;
			queue_rear = NewCommand;
		}
		size++;
	}
	/* Dequeue new element */
	void Dequeue()
	{
		if (IsEmpty())
		{
			return;
		}

		TrkCommandQueue* temp = queue_front;
		queue_front = queue_front->next_queue;
		delete temp;
		size--;
	}
	/* Peek the next element */
	std::string Peek()
	{
		if (IsEmpty())
		{
			return "";
		}

		return queue_front->command;
	}
	/* Check if this list empty */
	bool IsEmpty() const { return size == 0; }
	/* Returns the size of this list */
	int GetSize() const { return size; }


	/* Command of this queue element */
	std::string command;
private:
	/* Next queue of this element */
	TrkCommandQueue* next_queue;
	/* First element of list */
	TrkCommandQueue* queue_front;
	/* Last element of list */
	TrkCommandQueue* queue_rear;
	/* Size of list */
	int size;
};


#endif /* CONNECT_H */