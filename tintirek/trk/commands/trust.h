/*
 *	trust.h
 *
 *	Tintirek's trust command header file
 */

#ifndef TRK_TRUST_COMMAND_H
#define TRK_TRUST_COMMAND_H

#include "cmdline.h"

class TrkCliTrustCommand : public TrkCliCommand
{
	virtual bool CallCommand_Implementation(const TrkCliOption* Options, TrkCliOptionResults* Result) override;

	virtual bool CheckCommandFlags_Implementation(const char Flag) override;
};

#endif /* TRK_TRUST_COMMAND_H */