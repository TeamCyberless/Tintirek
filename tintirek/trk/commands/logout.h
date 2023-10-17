/*
 *	logout.h
 *
 *	Tintirek's logout command header file
 */

#ifndef TRK_LOGOUT_COMMAND_H
#define TRK_LOGOUT_COMMAND_H

#include "cmdline.h"

class TrkCliLogoutCommand : public TrkCliCommand
{
	virtual bool CallCommand_Implementation(const TrkCliOption* Options, TrkCliOptionResults* Result) override;

	virtual bool CheckCommandFlags_Implementation(const char Flag) override;
};

#endif /* TRK_LOGOUT_COMMAND_H */