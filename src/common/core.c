/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2016  Hercules Dev Team
 * Copyright (C)  Athena Dev Teams
 *
 * Hercules is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#define HERCULES_CORE

#include "config/core.h"
#include "core.h"

#include "common/cbasetypes.h"
#include "common/console.h"
#include "common/db.h"
#include "common/des.h"
#include "common/grfio.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/strlib.h"
#include "common/sysinfo.h"
#include "common/timer.h"
#include "common/utils.h"

#ifndef MINICORE
#	include "common/HPM.h"
#	include "common/conf.h"
#	include "common/ers.h"
#	include "common/md5calc.h"
#	include "common/mutex.h"
#	include "common/random.h"
#	include "common/socket.h"
#	include "common/sql.h"
#	include "common/thread.h"
#endif

#ifndef _WIN32
#	include <unistd.h>
#else
#	include "common/winapi.h" // Console close event handling
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Uncomment the line below if you want to silence the root warning on startup
 * (not recommended, as it opens the machine to security risks. You should
 * never ever run software as root unless it requires the extra privileges
 * (which Hercules does not.)
 * More info:
 * http://www.tldp.org/HOWTO/Security-HOWTO/local-security.html
 * http://www.gentoo.org/doc/en/security/security-handbook.xml?style=printable&part=1&chap=1#doc_chap4
 * http://wiki.centos.org/TipsAndTricks/BecomingRoot
 * http://fedoraproject.org/wiki/Configuring_Sudo
 * https://help.ubuntu.com/community/RootSudo
 * http://www.freebsdwiki.net/index.php/Root
 *
 * If your service provider forces (or encourages) you to run server software
 * as root, please complain to them before and after uncommenting this line,
 * since it is a very bad idea.
 * Please note that NO SUPPORT will be given if you uncomment the following line.
 */
//#define I_AM_AWARE_OF_THE_RISK_AND_STILL_WANT_TO_RUN_HERCULES_AS_ROOT
// And don't complain to us if the XYZ plugin you installed wiped your hard disk, or worse.
// Note: This feature is deprecated, and should not be used.

/// Called when a terminate signal is received.
void (*shutdown_callback)(void) = NULL;

struct core_interface core_s;
struct core_interface *core = &core_s;

#ifndef MINICORE // minimalist Core
// Added by Gabuzomeu
//
// This is an implementation of signal() using sigaction() for portability.
// (sigaction() is POSIX; signal() is not.)  Taken from Stevens' _Advanced
// Programming in the UNIX Environment_.
//
#ifdef WIN32 // windows don't have SIGPIPE
#define SIGPIPE SIGINT
#endif

#ifndef POSIX
#define compat_signal(signo, func) signal((signo), (func))
#else
sigfunc *compat_signal(int signo, sigfunc *func)
{
	struct sigaction sact, oact;

	sact.sa_handler = func;
	sigemptyset(&sact.sa_mask);
	sact.sa_flags = 0;
#ifdef SA_INTERRUPT
	sact.sa_flags |= SA_INTERRUPT; /* SunOS */
#endif

	if (sigaction(signo, &sact, &oact) < 0)
		return (SIG_ERR);

	return (oact.sa_handler);
}
#endif

/*======================================
 * CORE : Console events for Windows
 *--------------------------------------*/
#ifdef _WIN32
static BOOL WINAPI console_handler(DWORD c_event)
{
	switch(c_event) {
		case CTRL_CLOSE_EVENT:
		case CTRL_LOGOFF_EVENT:
		case CTRL_SHUTDOWN_EVENT:
			if( shutdown_callback != NULL )
				shutdown_callback();
			else
				core->runflag = CORE_ST_STOP;// auto-shutdown
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

static void cevents_init(void)
{
	if (SetConsoleCtrlHandler(console_handler,TRUE)==FALSE)
		ShowWarning ("Unable to install the console handler!\n");
}
#endif

/*======================================
 * CORE : Signal Sub Function
 *--------------------------------------*/
static void sig_proc(int sn)
{
	static int is_called = 0;

	switch (sn) {
		case SIGINT:
		case SIGTERM:
			if (++is_called > 3)
				exit(EXIT_SUCCESS);
			if( shutdown_callback != NULL )
				shutdown_callback();
			else
				core->runflag = CORE_ST_STOP;// auto-shutdown
			break;
		case SIGSEGV:
		case SIGFPE:
			do_abort();
			// Pass the signal to the system's default handler
			compat_signal(sn, SIG_DFL);
			raise(sn);
			break;
	#ifndef _WIN32
		case SIGXFSZ:
			// ignore and allow it to set errno to EFBIG
			ShowWarning ("Max file size reached!\n");
			//run_flag = 0; // should we quit?
			break;
		case SIGPIPE:
			//ShowInfo ("Broken pipe found... closing socket\n"); // set to eof in socket.c
			break; // does nothing here
	#endif
	}
}

void signals_init (void)
{
	compat_signal(SIGTERM, sig_proc);
	compat_signal(SIGINT, sig_proc);
#ifndef _DEBUG // need unhandled exceptions to debug on Windows
	compat_signal(SIGSEGV, sig_proc);
	compat_signal(SIGFPE, sig_proc);
#endif
#ifndef _WIN32
	compat_signal(SIGILL, SIG_DFL);
	compat_signal(SIGXFSZ, sig_proc);
	compat_signal(SIGPIPE, sig_proc);
	compat_signal(SIGBUS, SIG_DFL);
	compat_signal(SIGTRAP, SIG_DFL);
#endif
}
#endif

/**
 * Warns the user if executed as superuser (root)
 *
 * @retval false if the check didn't pass and the program should be terminated.
 */
bool usercheck(void)
{
#ifndef _WIN32
	if (sysinfo->is_superuser()) {
		if (!isatty(fileno(stdin))) {
#ifdef BUILDBOT
			return true;
#else  // BUILDBOT
			ShowFatalError("You are running Hercules with root privileges, it is not necessary, nor recommended. "
					"Aborting.\n");
			return false; // Don't allow noninteractive execution regardless.
#endif  // BUILDBOT
		}
		ShowError("You are running Hercules with root privileges, it is not necessary, nor recommended.\n");
#ifdef I_AM_AWARE_OF_THE_RISK_AND_STILL_WANT_TO_RUN_HERCULES_AS_ROOT
#ifndef BUILDBOT
#warning This Hercules build is not eligible to obtain support by the developers.
#warning The setting I_AM_AWARE_OF_THE_RISK_AND_STILL_WANT_TO_RUN_HERCULES_AS_ROOT is deprecated and should not be used.
#endif  // BUILDBOT
#else // not I_AM_AWARE_OF_THE_RISK_AND_STILL_WANT_TO_RUN_HERCULES_AS_ROOT
		ShowNotice("Execution will be paused for 60 seconds. Press Ctrl-C if you wish to quit.\n");
		ShowNotice("If you want to get rid of this message, please open %s and uncomment, near the top, the line saying:\n"
				"\t\"//#define I_AM_AWARE_OF_THE_RISK_AND_STILL_WANT_TO_RUN_HERCULES_AS_ROOT\"\n", __FILE__);
		ShowNotice("Note: In a near future, this courtesy notice will go away. "
				"Please update your infrastructure not to require root privileges before then.\n");
		ShowWarning("It's recommended that you " CL_WHITE "press CTRL-C now!" CL_RESET "\n");
		{
			int i;
			for (i = 0; i < 60; i++) {
				ShowMessage("\a *");
				HSleep(1);
			}
		}
		ShowMessage("\n");
		ShowNotice("Resuming operations with root privileges. "
				CL_RED "If anything breaks, you get to keep the pieces, "
				"and the Hercules developers won't be able to help you."
				CL_RESET "\n");
#endif // I_AM_AWARE_OF_THE_RISK_AND_STILL_WANT_TO_RUN_HERCULES_AS_ROOT
	}
#endif // not _WIN32
	return true;
}

void core_defaults(void)
{
	nullpo_defaults();
#ifndef MINICORE
	hpm_defaults();
	HCache_defaults();
#endif
	sysinfo_defaults();
	console_defaults();
	strlib_defaults();
	malloc_defaults();
	showmsg_defaults();
	cmdline_defaults();
	des_defaults();
	grfio_defaults(); // Note: grfio is lazily loaded. grfio->init() and grfio->final() are not automatically called.
#ifndef MINICORE
	mutex_defaults();
	libconfig_defaults();
	sql_defaults();
	timer_defaults();
	db_defaults();
	socket_defaults();
	rnd_defaults();
	md5_defaults();
	thread_defaults();
#endif
}

/**
 * Returns the source (core or plugin name) for the given command-line argument
 */
const char *cmdline_arg_source(struct CmdlineArgData *arg)
{
#ifdef MINICORE
	return "core";
#else // !MINICORE
	nullpo_retr(NULL, arg);
	return HPM->pid2name(arg->pluginID);
#endif // MINICORE
}

/**
 * Defines a command line argument.
 *
 * @param pluginID  the source plugin ID (use HPM_PID_CORE if loading from the core).
 * @param name      the command line argument name, including the leading '--'.
 * @param shortname an optional short form (single-character, it will be prefixed with '-'). Use '\0' to skip.
 * @param func      the triggered function.
 * @param help      the help string to be displayed by '--help', if any.
 * @param options   options associated to the command-line argument. @see enum cmdline_options.
 * @return the success status.
 */
bool cmdline_arg_add(unsigned int pluginID, const char *name, char shortname, CmdlineExecFunc func, const char *help, unsigned int options)
{
	struct CmdlineArgData *data = NULL;

	nullpo_retr(false, name);
	VECTOR_ENSURE(cmdline->args_data, 1, 1);
	VECTOR_PUSHZEROED(cmdline->args_data);
	data = &VECTOR_LAST(cmdline->args_data);
	data->pluginID = pluginID;
	data->name = aStrdup(name);
	data->shortname = shortname;
	data->func = func;
	if (help)
		data->help = aStrdup(help);
	else
		data->help = NULL;
	data->options = options;

	return true;
}

/**
 * Help screen to be displayed by '--help'.
 */
static CMDLINEARG(help)
{
	int i;
	ShowInfo("Usage: %s [options]\n", SERVER_NAME);
	ShowInfo("\n");
	ShowInfo("Options:\n");

	for (i = 0; i < VECTOR_LENGTH(cmdline->args_data); i++) {
		struct CmdlineArgData *data = &VECTOR_INDEX(cmdline->args_data, i);
		char altname[16], paramnames[256];
		if (data->shortname) {
			snprintf(altname, sizeof(altname), " [-%c]", data->shortname);
		} else {
			*altname = '\0';
		}
		snprintf(paramnames, sizeof(paramnames), "%s%s%s", data->name, altname, (data->options&CMDLINE_OPT_PARAM) ? " <name>" : "");
		ShowInfo("  %-30s %s [%s]\n", paramnames, data->help ? data->help : "<no description provided>", cmdline->arg_source(data));
	}
	return false;
}

/**
 * Info screen to be displayed by '--version'.
 */
static CMDLINEARG(version)
{
	ShowInfo(CL_GREEN"Website/Forum:"CL_RESET"\thttp://herc.ws/\n");
	ShowInfo(CL_GREEN"IRC Channel:"CL_RESET"\tirc://irc.rizon.net/#Hercules\n");
	ShowInfo("Open "CL_WHITE"readme.txt"CL_RESET" for more information.\n");
	return false;
}

/**
 * Checks if there is a value available for the current argument
 *
 * @param name        the current argument's name.
 * @param current_arg the current argument's position.
 * @param argc        the program's argc.
 * @return true if a value for the current argument is available on the command line.
 */
bool cmdline_arg_next_value(const char *name, int current_arg, int argc)
{
	if (current_arg >= argc-1) {
		ShowError("Missing value for option '%s'.\n", name);
		return false;
	}

	return true;
}

/**
 * Executes the command line arguments handlers.
 *
 * @param argc    the program's argc
 * @param argv    the program's argv
 * @param options Execution options. Allowed values:
 * - CMDLINE_OPT_SILENT: Scans the argv for a command line argument that
 *   requires the server's silent mode, and triggers it. Invalid command line
 *   arguments don't cause it to abort. No command handlers are executed.
 * - CMDLINE_OPT_PREINIT: Scans the argv for command line arguments with the
 *   CMDLINE_OPT_PREINIT flag set and executes their handlers. Invalid command
 *   line arguments don't cause it to abort. Handler's failure causes the
 *   program to abort.
 * - CMDLINE_OPT_NORMAL: Scans the argv for normal command line arguments,
 *   skipping the pre-init ones, and executes their handlers. Invalid command
 *   line arguments or handler's failure cause the program to abort.
 * @return the amount of command line handlers successfully executed.
 */
int cmdline_exec(int argc, char **argv, unsigned int options)
{
	int count = 0, i;

	nullpo_ret(argv);
	for (i = 1; i < argc; i++) {
		int j;
		struct CmdlineArgData *data = NULL;
		const char *arg = argv[i];
		if (arg[0] != '-') { // All arguments must begin with '-'
			if ((options&(CMDLINE_OPT_SILENT|CMDLINE_OPT_PREINIT)) != 0)
				continue;
			ShowError("Invalid option '%s'.\n", argv[i]);
			exit(EXIT_FAILURE);
		}
		if (arg[1] != '-' && strlen(arg) == 2) {
			ARR_FIND(0, VECTOR_LENGTH(cmdline->args_data), j, VECTOR_INDEX(cmdline->args_data, j).shortname == arg[1]);
		} else {
			ARR_FIND(0, VECTOR_LENGTH(cmdline->args_data), j, strcmpi(VECTOR_INDEX(cmdline->args_data, j).name, arg) == 0);
		}
		if (j == VECTOR_LENGTH(cmdline->args_data)) {
			if (options&(CMDLINE_OPT_SILENT|CMDLINE_OPT_PREINIT))
				continue;
			ShowError("Unknown option '%s'.\n", arg);
			exit(EXIT_FAILURE);
		}
		data = &VECTOR_INDEX(cmdline->args_data, j);
		if (data->options&CMDLINE_OPT_PARAM) {
			if (!cmdline->arg_next_value(arg, i, argc))
				exit(EXIT_FAILURE);
			i++;
		}
		if (options&CMDLINE_OPT_SILENT) {
			if (data->options&CMDLINE_OPT_SILENT) {
				showmsg->silent = 0x7; // silence information and status messages
				break;
			}
		} else if ((data->options&CMDLINE_OPT_PREINIT) == (options&CMDLINE_OPT_PREINIT)) {
			const char *param = NULL;
			if (data->options&CMDLINE_OPT_PARAM) {
				param = argv[i]; // Already incremented above
			}
			if (!data->func(arg, param))
				exit(EXIT_SUCCESS);
			count++;
		}
	}
	return count;
}

/**
 * Defines the global command-line arguments.
 */
void cmdline_init(void)
{
#ifdef MINICORE
	// Minicore has no HPM. This value isn't used, but the arg_add function requires it, so we're (re)defining it here
#define HPM_PID_CORE ((unsigned int)-1)
#endif
	CMDLINEARG_DEF(help, 'h', "Displays this help screen", CMDLINE_OPT_NORMAL);
	CMDLINEARG_DEF(version, 'v', "Displays the server's version.", CMDLINE_OPT_NORMAL);
#ifndef MINICORE
	CMDLINEARG_DEF2(load-plugin, loadplugin, "Loads an additional plugin (can be repeated).", CMDLINE_OPT_PARAM|CMDLINE_OPT_PREINIT);
#endif // !MINICORE
	cmdline_args_init_local();
}

void cmdline_final(void)
{
	while (VECTOR_LENGTH(cmdline->args_data) > 0) {
		struct CmdlineArgData *data = &VECTOR_POP(cmdline->args_data);
		aFree(data->name);
		aFree(data->help);
	}
	VECTOR_CLEAR(cmdline->args_data);
}

struct cmdline_interface cmdline_s;
struct cmdline_interface *cmdline;

void cmdline_defaults(void)
{
	cmdline = &cmdline_s;

	VECTOR_INIT(cmdline->args_data);

	cmdline->init = cmdline_init;
	cmdline->final = cmdline_final;
	cmdline->arg_add = cmdline_arg_add;
	cmdline->exec = cmdline_exec;
	cmdline->arg_next_value = cmdline_arg_next_value;
	cmdline->arg_source = cmdline_arg_source;
}

/*======================================
 * CORE : MAINROUTINE
 *--------------------------------------*/
int main (int argc, char **argv)
{
	int retval = EXIT_SUCCESS;
	{// initialize program arguments
		char *p1 = SERVER_NAME = argv[0];
		char *p2 = p1;
		while ((p1 = strchr(p2, '/')) != NULL || (p1 = strchr(p2, '\\')) != NULL) {
			SERVER_NAME = ++p1;
			p2 = p1;
		}
		core->arg_c = argc;
		core->arg_v = argv;
		core->runflag = CORE_ST_RUN;
	}
	core_defaults();

	iMalloc->init();// needed for Show* in display_title() [FlavioJS]
	showmsg->init();

	cmdline->init();

	cmdline->exec(argc, argv, CMDLINE_OPT_SILENT);

	iMalloc->init_messages(); // Initialization messages (after buying us some time to suppress them if needed)

	sysinfo->init();

	if (!(showmsg->silent&0x1))
		console->display_title();

	if (!usercheck())
		return EXIT_FAILURE;

#ifdef MINICORE // minimalist Core
	do_init(argc,argv);
	do_final();
#else// not MINICORE
	set_server_type();

	Sql_Init();
	thread->init();
	DB->init();
	signals_init();

#ifdef _WIN32
	cevents_init();
#endif

	timer->init();

	/* timer first */
	rnd->init();

	console->init();

	HCache->init();

	HPM->init();

	sockt->init();

	do_init(argc,argv);

	// Main runtime cycle
	while (core->runflag != CORE_ST_STOP) {
		int next = timer->perform(timer->gettick_nocache());
		sockt->perform(next);
	}

	console->final();

	retval = do_final();
	HPM->final();
	timer->final();
	sockt->final();
	DB->final();
	thread->final();
	ers_final();
	rnd->final();
#endif
	cmdline->final();
	//sysinfo->final(); Called by iMalloc->final()

	iMalloc->final();
	showmsg->final(); // Should be after iMalloc->final()

	return retval;
}
