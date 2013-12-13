// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "nullpo.h"
#include "../common/showmsg.h"

/**
 * Reports NULL pointer information
 */
void nullpo_info(const char *file, int line, const char *func, const char *targetname) {
	if (file == NULL)
		file = "??";
	
	if (func == NULL || *func == '\0')
		func = "unknown";
	
	ShowMessage("--- nullpo info --------------------------------------------\n");
	ShowMessage("%s:%d: target '%s' in func `%s'\n", file, line, targetname, func);
	ShowMessage("--- end nullpo info ----------------------------------------\n");
}
