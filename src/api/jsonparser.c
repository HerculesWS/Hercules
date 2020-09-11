/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2020 Hercules Dev Team
 * Copyright (C) 2020 Andrei Karas (4144)
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
#include "api/jsonparser.h"

#include "common/HPM.h"
#include "common/cbasetypes.h"
#include "common/memmgr.h"
#include "common/nullpo.h"

#include <stdlib.h>

static struct jsonparser_interface jsonparser_s;
struct jsonparser_interface *jsonparser;

static int do_init_jsonparser(bool minimal)
{
	return 0;
}

static void do_final_jsonparser(void)
{
}

static JsonP *jsonparser_parse(const char *text)
{
	nullpo_retr(NULL, text);

	return cJSON_Parse(text);
}

char* jsonparser_get_formatted_string(const JsonP *parent)
{
	nullpo_retr(NULL, parent);

	return cJSON_Print(parent);
}

char* jsonparser_get_string(const JsonP *parent)
{
	nullpo_retr(NULL, parent);

	return cJSON_PrintUnformatted(parent);
}

void jsonparser_print(const JsonP *parent)
{
	nullpo_retv(parent);
	char *str = jsonparser->get_formatted_string(parent);
	ShowInfo("Json string: '%s'\n", str);
	jsonparser->free(str);
}

JsonP *jsonparser_get(const JsonP *parent, const char *name)
{
	nullpo_retr(NULL, parent);
	nullpo_retr(NULL, name);

	return cJSON_GetObjectItemCaseSensitive(parent, name);
}

int jsonparser_get_array_size(const JsonP *parent)
{
	nullpo_retr(false, parent);
	Assert_retr(0, cJSON_IsArray(parent));

	return cJSON_GetArraySize(parent);
}

char *jsonparser_get_string_value(const JsonP *parent)
{
	nullpo_retr(false, parent);
	Assert_retr(0, cJSON_IsString(parent));

	return parent->valuestring;
}

bool jsonparser_is_null(const JsonP *parent)
{
	nullpo_retr(false, parent);

	return cJSON_IsNull(parent);
}

bool jsonparser_is_null_or_missing(const JsonP *parent)
{
	return parent == NULL || cJSON_IsNull(parent);
}

void jsonparser_free(char *ptr)
{
	// need call real free because cJSON using direct malloc/free
	free(ptr);
}

void jsonparser_delete(JsonP *ptr)
{
	cJSON_Delete(ptr);
}

void jsonparser_defaults(void)
{
	jsonparser = &jsonparser_s;
	/* core */
	jsonparser->init = do_init_jsonparser;
	jsonparser->final = do_final_jsonparser;

	jsonparser->parse = jsonparser_parse;
	jsonparser->get_string = jsonparser_get_string;
	jsonparser->get_formatted_string = jsonparser_get_formatted_string;
	jsonparser->get = jsonparser_get;
	jsonparser->get_array_size = jsonparser_get_array_size;
	jsonparser->get_string_value = jsonparser_get_string_value;
	jsonparser->is_null = jsonparser_is_null;
	jsonparser->is_null_or_missing = jsonparser_is_null_or_missing;
	jsonparser->print = jsonparser_print;
	jsonparser->free = jsonparser_free;
	jsonparser->delete = jsonparser_delete;
}
