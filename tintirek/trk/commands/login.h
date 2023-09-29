/*
 *	login.h
 *
 *	Tintirek's login command header file
 */

#ifndef TRK_LOGIN_COMMAND_H
#define TRK_LOGIN_COMMAND_H

#include "trk_cmdline.h"

class TrkCliLoginCommand : public TrkCliCommand
{
	virtual bool CallCommand_Implementation(const TrkCliOption* Options, TrkCliOptionResults* Result) override;

	virtual bool CheckCommandFlags_Implementation(const char Flag) override;
};

#endif /* TRK_LOGIN_COMMAND_H */