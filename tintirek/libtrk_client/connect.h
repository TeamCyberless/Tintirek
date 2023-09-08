/*
 *	connect.h
 * 
 *	Tintirek's client-side connection helpers
 */


#ifndef CONNECT_H
#define CONNECT_H


#include "trk_string.h"


 /* Connection Helper Class */
class TrkConnectHelper
{
public:
	/* Send command to server */
	static bool SendCommand(class TrkCliClientOptionResults& opt_result, const TrkString Command, TrkString& ErrorStr, TrkString& Returned);
	/* Send multiple commands to server */
	static bool SendCommandMultiple(class TrkCliClientOptionResults& opt_result, class TrkCommandQueue* Commands, TrkString& ErrorStr, TrkString& Returned);

protected:
	/* Sends packet to client as chunked data */
	static bool SendPacket(int client_socket, const TrkString message, int& error_code);
	/* Recovers packet from all chunk data from client */
	static bool ReceivePacket(int client_socket, TrkString& message, TrkString& error_msg);
	/* Internal code for connecting to the server */
	static bool Connect_Internal(class TrkCliClientOptionResults& opt_result, int& client_socket, TrkString& ErrorStr);
	/* Internal code for disconnecting from the server */
	static bool Disconnect_Internal(int client_socket, TrkString& ErrorStr);
};


/* Command Queue Helper Class */
class TrkCommandQueue
{
public:
	TrkCommandQueue(TrkString Command)
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
	TrkString Peek()
	{
		if (IsEmpty())
		{
			return nullptr;
		}

		return queue_front->command;
	}
	/* Check if this list empty */
	bool IsEmpty() const { return size == 0; }
	/* Returns the size of this list */
	int GetSize() const { return size; }


	/* Command of this queue element */
	TrkString command;
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