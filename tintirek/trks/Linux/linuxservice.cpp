/*
 *	linuxservice.cpp
 *
 *	Definitions for Linux/Unix service support
 */

#include "../service.h"


#ifdef __linux__

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>


/* static reference to our service object */
static TrkLinuxService* obj;

 /* SIGTERM handler. */
void sigTermHandler(int signum) {
	obj->ServiceMustStop();
}

/* SIGINT handler. */
void sigIntHandler(int signum) {
	obj->ServiceMustStop();
}



void TrkLinuxService::Initialization()
{
	obj = this;

	struct sigaction actionTerm;
	actionTerm.sa_handler = sigTermHandler;
	sigemptyset(&actionTerm.sa_mask);
	actionTerm.sa_flags = 0;
	sigaction(SIGTERM, &actionTerm, NULL);

	struct sigaction actionInt;
	actionInt.sa_handler = sigIntHandler;
	sigemptyset(&actionInt.sa_mask);
	actionInt.sa_flags = 0;
	sigaction(SIGINT, &actionInt, NULL);
}

bool TrkLinuxService::ServiceStart()
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
	pid_t pid = fork();
	if (pid < 0)
	{
		exit(EXIT_FAILURE);
	}

	if (pid > 0)
	{
		exit(EXIT_SUCCESS);
	}

	if (setsid() < 0)
	{
		exit(EXIT_FAILURE);
	}

	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	close(STDIN_FILENO);
	freopen(log_path, "a", stdout);
	freopen(log_path, "a", stderr);

	pid = fork();

	if (pid < 0)
	{
		exit(EXIT_FAILURE);
	}

	if (pid > 0)
	{
		exit(EXIT_SUCCESS);
	}

	umask(0);

	chdir("/");

	pidFile = open(pid_path, O_WRONLY | O_CREAT, 0600);
	if (pidFile < 0)
	{
		return false;
	}

	if (lockf(pidFile, F_TLOCK, 0) < 0)
	{
		return false;
	}

	char str[256];
	sprintf(str, "%d\n", getpid());
	write(pidFile, str, strlen(str));

	return true;
#pragma GCC diagnostic pop
}

void TrkLinuxService::ServiceRunning()
{
	sleep(1);
}

void TrkLinuxService::ServiceNotifyStop()
{
	close(pidFile);
	unlink(pid_path);
}


#endif /* __linux__ */