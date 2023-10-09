/*
 *	passwd.h
 * 
 *	Client's password functions
 */


#ifndef TRK_PASSWD_H
#define TRK_PASSWD_H

#include "trk_string.h"


/* Helper for password authentication */
class TrkPasswdHelper
{
public:
	static bool CheckSessionFileExists();
	static bool SaveSessionTicket(TrkString Ticket, TrkString ServerUrl);
	static bool DeleteSessionTicket(TrkString ServerUrl);
	static bool ChangeSessionTicket(TrkString Ticket, TrkString ServerUrl);
	static TrkString GetSessionTicketByServerURL(TrkString ServerUrl);
};

#endif /* TRK_PASSWD_H */