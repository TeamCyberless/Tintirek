/*
 *	add.h
 * 
 *	Tintirek's add command header file
 */

#ifndef TRK_ADD_COMMAND_H
#define TRK_ADD_COMMAND_H

#include "trk_cmdline.h"

class TrkCliAddCommand : public TrkCliCommand
{
	virtual bool CallCommand_Implementation(const TrkCliOption* Options, TrkCliOptionResults* Result) override;

	virtual bool CheckCommandFlags_Implementation(const char Flag) override;
};

#endif /* TRK_ADD_COMMAND_H */

