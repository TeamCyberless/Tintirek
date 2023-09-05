/*
 *	info.h
 *
 *	Tintirek's info command header file
 */

#ifndef TRK_INFO_COMMAND_H
#define TRK_INFO_COMMAND_H

#include "trk_cmdline.h"

class TrkCliInfoCommand : public TrkCliCommand
{
	virtual bool CallCommand_Implementation(const TrkCliOption* Options, TrkCliOptionResults* Result) override;

	virtual bool CheckCommandFlags_Implementation(const char Flag) override;
};

#endif /* TRK_INFO_COMMAND_H */