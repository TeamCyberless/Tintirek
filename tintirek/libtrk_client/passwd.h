/*
 *	passwd.h
 * 
 *	Client's password functions
 */


#ifndef TRK_PASSWD_H
#define TRK_PASSWD_H

#include <string>

/* Helper for password authentication */
class TrkPasswdHelper
{
public:
	static bool CheckSessionFileExists();
	static bool SaveSessionTicket(std::string Ticket, std::string ServerUrl);
	static bool DeleteSessionTicket(std::string ServerUrl);
	static bool ChangeSessionTicket(std::string Ticket, std::string ServerUrl);
	static std::string GetSessionTicketByServerURL(std::string ServerUrl);
};

#endif /* TRK_PASSWD_H */