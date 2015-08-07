// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#define HERCULES_CORE

#include "nullpo.h"

#include "common/showmsg.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#ifdef __GNUC__
#include <execinfo.h>
#endif

struct nullpo_interface nullpo_s;

/**
 * Reports failed assertions or NULL pointers
 *
 * @param file       Source file where the error was detected
 * @param line       Line
 * @param func       Function
 * @param targetname Name of the checked symbol
 * @param title      Message title to display (i.e. failed assertion or nullpo info)
 */
void assert_report(const char *file, int line, const char *func, const char *targetname, const char *title) {
#ifdef __GNUC__
	void *array[10];
	int size;
	char **strings;
	int i;
#endif
	if (file == NULL)
		file = "??";

	if (func == NULL || *func == '\0')
		func = "unknown";

	ShowError("--- %s --------------------------------------------\n", title);
	ShowError("%s:%d: '%s' in function `%s'\n", file, line, targetname, func);
#ifdef __GNUC__
	size = (int)backtrace(array, 10);
	strings = backtrace_symbols(array, size);
	for (i = 0; i < size; i++)
		ShowError("%s\n", strings[i]);
	free(strings);
#endif
	ShowError("--- end %s ----------------------------------------\n", title);
}

/**
 *
 **/
void nullpo_defaults(void) {
	nullpo = &nullpo_s;
	
	nullpo->assert_report = assert_report;
}
