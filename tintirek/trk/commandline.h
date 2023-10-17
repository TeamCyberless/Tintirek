/*
 *	commandline.h
 * 
 *	command line stuff about Tintirek programs.
 */

#ifndef TRK_COMMANDLINE_H
#define TRK_COMMANDLINE_H

#include "trk_types.h"
#include "cmdline.h"

/* Includes of all commands */
#include "commands/add.h"
#include "commands/edit.h"
#include "commands/info.h"
#include "commands/login.h"
#include "commands/logout.h"
#include "commands/trust.h"


/* All commands are generated here */
TrkCliCommand* trkAddCommand = new TrkCliAddCommand;
TrkCliCommand* trkEditCommand = new TrkCliEditCommand;
TrkCliCommand* trkInfoCommand = new TrkCliInfoCommand;
TrkCliCommand* trkLoginCommand = new TrkCliLoginCommand;
TrkCliCommand* trkLogoutCommand = new TrkCliLogoutCommand;
TrkCliCommand* trkTrustCommand = new TrkCliTrustCommand;



/*
 *	Predefined flags for command line
 *
 *	These constants define the predefined command line flags and their descriptions.
 */

 /* Ignores the ignore file */
#define TRK_CLI_FLAG_IGNORE TrkCliOptionFlag('i', "Informs the client that it should not perform any ignore checking.")

/* Previews operation */
#define TRK_CLI_FLAG_PREVIEW TrkCliOptionFlag('p', "Displays a preview of this command operation without changing any data.")

/* Displays status */
#define TRK_CLI_FLAG_STATUS TrkCliOptionFlag('s', "Displays the ticket information.")

/* Specifies filetype of added/edited file */
#define TRK_CLI_FLAG_TYPE TrkCliOptionFlag('t', "adds/edits the file as the specified filetype, overriding any settings in the typemap table.", "filetype")

/* Reverts only opened files */
#define TRK_CLI_FLAG_ADD TrkCliOptionFlag('a', "Reverts only those files that haven't changed.")

/* Gives description to submit */
#define TRK_CLI_FLAG_DESCRIPTION TrkCliOptionFlag('d', "Use the given description as the submit message.", "message")

/* Reopens submitted files */
#define TRK_CLI_FLAG_REOPEN TrkCliOptionFlag('r', "Reopens submitted files after submission")

/* Forces resynchronization */
#define TRK_CLI_FLAG_FORCE TrkCliOptionFlag('f', "Forces resynchronization even if the client already has the file.")

/* The amount of maximum file count */
#define TRK_CLI_FLAG_MAX TrkCliOptionFlag('m', "Limits the number of files to sync.", "count")



#endif /* TRK_COMMANDLINE_H */