/*
 *	service.h
 * 
 *	Defintions for platform-based service support
 * 
 *	This file features a cross-platform compatible structure, enabling 
 *	our project to operate seamlessly on various operating systems,
 *	including Windows with service support. The cross-platform design
 *	not only ensures compatibility with different environments but also
 *	facilitates easy utilization across a wide range of operating systems.
 */


#ifndef SERVICE_H
#define SERVICE_H


#include "config.h"

typedef int sig_atomic_t;

/*
 *	Service Interface Class
 * 
 *	This class allows to start our program as a service/daemon in the
 *	operating system.
 */
class TrkServiceInterface
{
public:
	virtual void Initialization() = 0;
	virtual bool ServiceStart() = 0;
	virtual void ServiceRunning() = 0;
	virtual void ServiceNotifyStop() = 0;

	virtual bool DoesServiceStopping() const { return g_terminate > 0; }
	virtual void ServiceMustStop() { g_terminate = 1; }

protected:
	volatile sig_atomic_t g_terminate = 0;
};


#ifdef _WIN32

class TrkWindowsService : public TrkServiceInterface
{
public:
	virtual void Initialization() override;
	virtual bool ServiceStart() override;
	virtual void ServiceRunning() override;
	virtual void ServiceNotifyStop() override;
};

#elif __APPLE__

class TrkMacOSService : public TrkServiceInterface
{
public:
	virtual void Initialization() override;
	virtual bool ServiceStart() override;
	virtual void ServiceRunning() override;
	virtual void ServiceNotifyStop() override;
};

#elif __linux__

class TrkLinuxService : public TrkServiceInterface
{
public:
	TrkLinuxService(std::string PIDPath, std::string LogPath)
		: pid_path(PIDPath)
		, log_path(LogPath)
	{ }

	virtual void Initialization() override;
	virtual bool ServiceStart() override;
	virtual void ServiceRunning() override;
	virtual void ServiceNotifyStop() override;

private:
	std::string pid_path;
	std::string log_path;
	int pidFile = -1;
};

#endif


#endif /* SERVICE_H */