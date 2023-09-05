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


#endif /* CONNECT_H */