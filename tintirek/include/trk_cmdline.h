/*
 *	trk_cmdline.h
 *
 *	Tintirek's command line functions
 */



#ifndef TRK_CMDLINE_H
#define TRK_CMDLINE_H

#include "trk_types.h"


/* Forward declaration for option class */
class TrkCliOption;



/* Determines if the command requires a paremeter or not */
enum class TrkCliRequiredOption : uint8_t
{
	/* Don't allow to parameter */
	NOT_ALLOWED = 0,
	/* Parameter allowed but not required */
	NO_REQUIRED,
	/* Parameter required */
	REQUIRED,
};



/* Holds results of some all commands */
class TrkCliOptionResults
{
public:
	/* Stores requested command */
	const TrkCliOption* requested_command = nullptr;
	/* If any, stores command parameter. Watch out! can be null. */
	const char* command_parameter = nullptr;


	/* If true, result will not effect anything, just for preview */
	bool preview = false;
	/* If true, process will be more forceful */
	bool force = false;
	/* If true, ignore file will be ignored */
	bool ignore = false;
	/* If true, files that haven't changed will be affected */
	bool not_changed = false;
	/* If true, processed files will be marked again after processing */
	bool remark = false;


	/* Type of file affected by the process */
	const char* filetype = nullptr;
	/* Process description */
	const char* description = nullptr;


	/* Determines how many files will be affected */
	int affect_max = -1;

protected:
	virtual void EmptyFunc() {}
};

/* Results of client-side */
class TrkCliClientOptionResults : public TrkCliOptionResults
{
public:
	/* Home directory path of our workspace */
	const char* workspace_path = nullptr;
	/* The connection addres of the server we want to connect to */
	const char* server_url = nullptr;
	/* The name of the user who will establish the connection */
	const char* username = nullptr;
	/* The user's password. Maybe will be encrypted */
	const char* password = nullptr;
	/* If true, trusts remote connection */
	bool trust = false;

	/* Found IP address from server_url */
	const char* ip_address = nullptr;
	/* Found port number from server_url */
	uint16_t port = 0;

	/* Server-side version info */
	const char* server_version = nullptr;
	/* Server-side uptime info */
	const char* server_uptime = nullptr;
	/* Server-side time info */
	const char* server_time = nullptr;
};

/* Results of server-side */
class TrkCliServerOptionResult : public TrkCliOptionResults
{
public:
	/* Starts as a daemon */
	bool daemon = false;

	/* Server start timestamp as epoch time */
	long long start_timestamp = 0;

	/* PID file path */
	const char* pid_file = nullptr;

	/* Server running root path */
	const char* running_root = nullptr;

	/* Log file destination */
	const char* log_path = nullptr;

	/* Server running port */
	uint16_t port_number = 5566;
};



/*
 * This class is used to create all commands in Tintirek
 * and process them accordingly.
 */
class TrkCliCommand
{
public:
	/* Execute the command with provided option results. */
	bool CallCommand(const TrkCliOption* Options, TrkCliOptionResults* Results);

	/* Check if a specific command flag is allowed. */
	bool CheckCommandFlags(const char Flag);

	/* Implementation of this command */
	virtual bool CallCommand_Implementation(const TrkCliOption* Options, TrkCliOptionResults* Result) = 0;

	/* Check if command flags are allowed */
	virtual bool CheckCommandFlags_Implementation(const char Flag) = 0;
};



/* Option flag for Tintirek's options */
class TrkCliOptionFlag
{
	/* Friendly class */
	friend class TrkCliOption;

public:
	TrkCliOptionFlag(const char Character, const char* Description, const char* ParameterName = nullptr)
		: character(Character)
		, description(Description)
		, required_parameter(ParameterName != nullptr)
		, parameter_name(ParameterName)
		, next_flag(nullptr)
	{ }

	/* Flag character */
	const char character;

	/* True if this flag requires a parameter, false otherwise */
	bool required_parameter;

	/* Flag parameter name if it's required */
	const char* parameter_name;

	/* The description of this flag */
	const char* description;

protected:
	/* Linked list's next element */
	TrkCliOptionFlag* next_flag;

	/*
	 *	Displays all flags in given option
	 *
	 *  Must be called from trk_cli_option
	 */
	static const char* Display(TrkCliOption* opt);

	/*
	 *	Creates a linked list from given array
	 *
	 *  Must be called from trk_cli_option
	 */
	static TrkCliOptionFlag* CreateList(TrkCliOptionFlag* list, size_t size);

public:
	/* Get next flag */
	TrkCliOptionFlag* GetNext() const { return next_flag; }



	/* Class for option flag iterator */
	class TrkCliOptionFlagIterator
	{
	private:
		/* Current element */
		TrkCliOptionFlag* current;

	public:
		TrkCliOptionFlagIterator(TrkCliOptionFlag* StartNode)
			: current(StartNode)
		{ }

		/* Checks given element with our current element */
		bool operator != (const TrkCliOptionFlagIterator& other) const {
			return current != other.current;
		}

		/* Returns current element when called as pointer */
		TrkCliOptionFlag* operator*() const {
			return current;
		}

		/* Increments our current element */
		TrkCliOptionFlagIterator& operator++() {
			current = current->GetNext();
			return *this;
		}
	};



	/* begin function override */
	TrkCliOptionFlagIterator begin() {
		return TrkCliOptionFlagIterator(this);
	}

	/* end function override */
	TrkCliOptionFlagIterator end() {
		return TrkCliOptionFlagIterator(nullptr);
	}
};



/* Tintirek's option list */
class TrkCliOption
{
public:
	TrkCliOption(const char* Command, TrkCliCommand* CommandUtilities, const char* Description, TrkCliRequiredOption RequiredParameter = TrkCliRequiredOption::NOT_ALLOWED, const char* ParameterName = "")
		: command(Command)
		, cmd_util(CommandUtilities)
		, description(Description)
		, head_flag(nullptr)
		, required_parameter(RequiredParameter)
		, parameter_name(ParameterName)
	{ }

	TrkCliOption(const char* Command, TrkCliCommand* CommandUtilities, const char* Description, TrkCliOptionFlag* Flags = {}, size_t FlagSize = 0, TrkCliRequiredOption RequiredParameter = TrkCliRequiredOption::NOT_ALLOWED, const char* ParameterName = "")
		: command(Command)
		, cmd_util(CommandUtilities)
		, description(Description)
		, head_flag(TrkCliOptionFlag::CreateList(Flags, FlagSize))
		, required_parameter(RequiredParameter)
		, parameter_name(ParameterName)
	{ }

	/* Command text */
	const char* command;

	/* Flags */
	TrkCliOptionFlag* head_flag;

	/* Command utilities */
	TrkCliCommand* cmd_util;

	/* Determines if this command requires a parameter or not */
	TrkCliRequiredOption required_parameter;

	/* Parameter name if it's required */
	const char* parameter_name;

	/* The description of this command */
	const char* description;

	/* Get all flags for help text */
	const char* GetFlagsForHelpText();
};

#endif /* TRK_CMDLINE_H */