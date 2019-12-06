/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_LIBBACKTRACE
#include "libbacktrace/backtrace.h"
#include "libbacktrace/backtrace-supported.h"
#elif defined(HAVE_EXECINFO)
#include <execinfo.h>
#endif // HAVE_LIBBACKTRACE


static struct nullpo_interface nullpo_s;
struct nullpo_interface *nullpo;

#ifdef HAVE_LIBBACKTRACE
static void nullpo_error_callback(void *data, const char *msg, int errnum)
{
	ShowError("Error: %s (%d)", msg, errnum);
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
	nullpo->backtrace_state = backtrace_create_state("hercules", BACKTRACE_SUPPORTS_THREADS, nullpo_error_callback, NULL);
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
