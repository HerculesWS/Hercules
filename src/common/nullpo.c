/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2023 Hercules Dev Team
 * Copyright (C) Athena Dev Teams
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

#include "nullpo.h"

#include "common/showmsg.h"
#include "common/strlib.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#if defined(HAVE_LIBBACKTRACE)
#include "libbacktrace/backtrace.h"
#include "libbacktrace/backtrace-supported.h"
#  if defined(WIN32)
#    include <windows.h>
#  elif defined(__sun)
#    include <limits.h>
#  elif defined(__linux) || defined(__linux__)
#    include <unistd.h>
#    include <limits.h>
#  elif defined(__APPLE__) && defined(__MACH__)
#    include <mach-o/dyld.h>
#  elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__)
#    include <sys/types.h>
#    include <sys/sysctl.h>
#  endif
#elif defined(HAVE_EXECINFO)
#include <execinfo.h>
#endif // HAVE_LIBBACKTRACE


static struct nullpo_interface nullpo_s;
struct nullpo_interface *nullpo;

#ifdef HAVE_LIBBACKTRACE
static char executable_path[PATH_MAX];

static void nullpo_error_callback(void *data, const char *msg, int errnum)
{
	ShowError("Error: %s (%d)\n", msg, errnum);
}

static int nullpo_print_callback(void *data, uintptr_t pc, const char *filename, int lineno, const char *function)
{
	ShowError("0x%lx %s\n",
		(unsigned long) pc,
		function == NULL ? "???" : function);
	ShowError("\t%s:%d\n",
		filename == NULL ? "???" : filename,
		lineno);
	return 0;
}

static void nullpo_backtrace_print(struct backtrace_state *state)
{
	backtrace_full(state, 0, nullpo_print_callback, nullpo_error_callback, state);
}

static bool nullpo_backtrace_get_executable_path(char *buf, size_t length)
{
#if defined(WIN32)
	char *exe_path = NULL;
	if (_get_pgmptr(&exe_path) != 0)
		return false;
	safestrncpy(buf, exe_path, length);
	return true;
#elif defined(__sun)
	if (length < MAX_PATH)
		return false;
	if (realpath(getexecname(), buf) == NULL)
		return false;
	buf[length - 1] = '\0';
	return true;
#elif defined(__linux) || defined(__linux__)
	ssize_t len = readlink("/proc/self/exe", buf, length);
	if (len <= 0 || len == length)
		return false;
	buf[len] = '\0';
	return true;
#elif defined(__APPLE__) && defined(__MACH__)
	uint32_t len = (uint32_t)length;
	if (_NSGetExecutablePath(buf, &len) != 0)
		return false; // buffer too small (!)
	// resolve symlinks, ., .. if possible
	char *canonical_path = realpath(buf, NULL);
	if (canonical_path != NULL) {
		safestrncpy(buf, canonical_path, length);
		free(canonical_path);
	}
	return true;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__)
	int mib[4];
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PATHNAME;
	mib[3] = -1;
	if (sysctl(mib, 4, buf, &length, NULL, 0) != 0)
		return false;
	return true;
#endif
	return false;
}
#endif  // HAVE_LIBBACKTRACE

/**
 * Reports failed assertions or NULL pointers
 *
 * @param file       Source file where the error was detected
 * @param line       Line
 * @param func       Function
 * @param targetname Name of the checked symbol
 * @param title      Message title to display (i.e. failed assertion or nullpo info)
 */
static void assert_report(const char *file, int line, const char *func, const char *targetname, const char *title)
{
	if (file == NULL)
		file = "??";

	if (func == NULL || *func == '\0')
		func = "unknown";

	ShowError("--- %s --------------------------------------------\n", title);
	ShowError("%s:%d: '%s' in function `%s'\n", file, line, targetname, func);
#ifdef HAVE_LIBBACKTRACE
	if (nullpo->backtrace_state != NULL)
		nullpo_backtrace_print(nullpo->backtrace_state);
#elif defined(HAVE_EXECINFO)
	void *array[10];
	int size = (int)backtrace(array, 10);
	char **strings = backtrace_symbols(array, size);
	for (int i = 0; i < size; i++)
		ShowError("%s\n", strings[i]);
	free(strings);
#endif // HAVE_LIBBACKTRACE
	ShowError("--- end %s ----------------------------------------\n", title);
}

static void nullpo_init(void)
{
#ifdef HAVE_LIBBACKTRACE
	if (!nullpo_backtrace_get_executable_path(executable_path, sizeof executable_path)) {
		safestrncpy(executable_path, "hercules", sizeof executable_path);
	}
	nullpo->backtrace_state = backtrace_create_state(executable_path, BACKTRACE_SUPPORTS_THREADS, nullpo_error_callback, NULL);
#endif
}

static void nullpo_final(void)
{
	// FIXME: libbacktrace doesn't provide a backtrace_free_state, and it's unsafe to pass the state to
	// backtrace_free (the function itself uses the state internally). For the time being, we'll leave the state
	// allocated until program termination as shown in their examples.
}

/**
 *
 **/
void nullpo_defaults(void)
{
	nullpo = &nullpo_s;
	nullpo->init = nullpo_init;
	nullpo->final = nullpo_final;
	nullpo->assert_report = assert_report;

	nullpo->backtrace_state = NULL;
}
