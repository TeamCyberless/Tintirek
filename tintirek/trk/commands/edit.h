/*
 *	edit.h
 *
 *	Tintirek's edit command header file
 */

#ifndef TRK_EDIT_COMMAND_H
#define TRK_EDIT_COMMAND_H

#include "trk_cmdline.h"

class TrkCliEditCommand : public TrkCliCommand
{
	virtual bool CallCommand_Implementation(const TrkCliOption* Options, TrkCliOptionResults* Result) override;

	virtual bool CheckCommandFlags_Implementation(const char Flag) override;
};

#endif /* TRK_EDIT_COMMAND_H */

