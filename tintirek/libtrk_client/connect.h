/*
 *	connect.h
 * 
 *	Tintirek's client-side connection helpers
 */


#ifndef CONNECT_H
#define CONNECT_H


 /* Connection Helper Class */
class TrkConnectHelper
{
public:
	/* Send command to server */
	static bool SendCommand(class TrkCliClientOptionResults& opt_result, const char* Command, const char*& ErrorStr, const char*& Returned);
	/* Send multiple commands to server */
	static bool SendCommandMultiple(class TrkCliClientOptionResults& opt_result, class TrkCommandQueue* Commands, const char*& ErrorStr, const char*& Returned);

protected:
	/* Sends packet to client as chunked data */
	static bool SendPacket(int client_socket, const char* message, int& error_code);
	/* Recovers packet from all chunk data from client */
	static bool ReceivePacket(int client_socket, const char*& message, const char*& error_msg);
	/* Internal code for connecting to the server */
	static bool Connect_Internal(class TrkCliClientOptionResults& opt_result, int& client_socket, const char*& ErrorStr);
	/* Internal code for disconnecting from the server */
	static bool Disconnect_Internal(int client_socket, const char*& ErrorStr);
};


/* Command Queue Helper Class */
class TrkCommandQueue
{
public:
	TrkCommandQueue(const char* Command)
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
	const char* Peek()
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
	const char* command;
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