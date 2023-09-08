/*
 *	trk_config.h
 * 
 *  Tintirek's configuration logics.
 */



#ifndef TRK_CONFIG_H
#define TRK_CONFIG_H


#include "trk_types.h"
#include "trk_cmdline.h"



/*
 *	Configuration Defines
 * 
 *	All Configuration defines are defined here
 */

#define TRK_CONFIG_CLIENT_PATH					"PATH"
#define TRK_CONFIG_CLIENT_SERVERURL				"SERVERURL"
#define TRK_CONFIG_CLIENT_USER					"USER"
#define TRK_CONFIG_CLIENT_PASSWORD				"PASSWORD"
#define TRK_CONFIG_CLIENT_TRUST					"TRUST"

#define TRK_CONFIG_SERVER_LOG					"LOG"
#define TRK_CONFIG_SERVER_PORT					"PORT"
#define TRK_CONFIG_SERVER_ROOT					"ROOT"
#define TRK_CONFIG_SERVER_NAME					"NAME"
#define TRK_CONFIG_SERVER_SSLKEY				"SSLKEY"


#define TRK_ENV_CLIENT_PATH						"TRK" TRK_CONFIG_CLIENT_PATH
#define TRK_ENV_CLIENT_SERVERURL				"TRK" TRK_CONFIG_CLIENT_SERVERURL
#define TRK_ENV_CLIENT_USER						"TRK" TRK_CONFIG_CLIENT_USER
#define TRK_ENV_CLIENT_PASSWORD					"TRK" TRK_CONFIG_CLIENT_PASSWORD
#define TRK_ENV_CLIENT_TRUST					"TRK" TRK_CONFIG_CLIENT_TRUST

#define TRK_ENV_SERVER_LOG						"TRK" TRK_CONFIG_SERVER_LOG
#define TRK_ENV_SERVER_PORT						"TRK" TRK_CONFIG_SERVER_PORT
#define TRK_ENV_SERVER_ROOT						"TRK" TRK_CONFIG_SERVER_ROOT
#define TRK_ENV_SERVER_NAME						"TRK" TRK_CONFIG_SERVER_NAME
#define TRK_ENV_SERVER_SSLKEY					"TRK" TRK_CONFIG_SERVER_SSLKEY


/* Configuration utilities */
class TrkConfig
{
public:
	/* Loads config file with validation */
	static bool LoadConfig(TrkCliOptionResults* Results);

	/* Writes to config file with validation */
	static bool WriteConfig(const TrkString FilePath, const TrkString key, const TrkString value);
};

#endif /* TRK_CONFIG */