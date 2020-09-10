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
#include "api/jsonwriter.h"

#include "common/HPM.h"
#include "common/cbasetypes.h"
#include "common/memmgr.h"
#include "common/nullpo.h"

#include <stdlib.h>

static struct jsonwriter_interface jsonwriter_s;
struct jsonwriter_interface *jsonwriter;

static int do_init_jsonwriter(bool minimal)
{
	return 0;
}

static void do_final_jsonwriter(void)
{
}

static JsonW *jsonwriter_create(const char *text)
{
	nullpo_retr(NULL, text);

	return cJSON_Parse(text);
}

JsonW *jsonwriter_new_array(void)
{
	return cJSON_CreateArray();
}

JsonW *jsonwriter_new_object(void)
{
	return cJSON_CreateObject();
}

JsonW *jsonwriter_new_string(const char *str)
{
	return cJSON_CreateString(str);
}

JsonW *jsonwriter_new_null(void)
{
	return cJSON_CreateNull();
}

JsonWBool jsonwriter_add_node(JsonW *parent, const char *name, JsonW *child)
{
	nullpo_retr(cJSON_False, parent);
	nullpo_retr(cJSON_False, name);
	nullpo_retr(cJSON_False, child);

	return cJSON_AddItemToObject(parent, name, child);
}

JsonWBool jsonwriter_add_node_to_array(JsonW *parent, JsonW *child)
{
	nullpo_retr(cJSON_False, parent);
	nullpo_retr(cJSON_False, child);
	Assert_retr(cJSON_False, cJSON_IsArray(parent));

	return cJSON_AddItemToArray(parent, child);
}

JsonW *jsonwriter_add_new_object(JsonW *parent, const char *name)
{
	nullpo_retr(NULL, parent);
	nullpo_retr(NULL, name);

	JsonW *obj = jsonwriter->new_object();
	if (!jsonwriter->add_node(parent, name, obj)) {
		Assert_retr(NULL, 0);
	}
	return obj;
}

JsonW *jsonwriter_add_new_array(JsonW *parent, const char *name)
{
	nullpo_retr(NULL, parent);
	nullpo_retr(NULL, name);

	JsonW *obj = jsonwriter->new_array();
	Assert_retr(NULL, cJSON_IsArray(obj));
	if (!jsonwriter->add_node(parent, name, obj)) {
		jsonwriter->delete(obj);
		Assert_retr(NULL, 0);
	}
	return obj;
}

JsonW *jsonwriter_add_new_null(JsonW *parent, const char *name)
{
	nullpo_retr(NULL, parent);
	nullpo_retr(NULL, name);

	JsonW *obj = jsonwriter->new_null();
	Assert_retr(NULL, cJSON_IsNull(obj));
	if (!jsonwriter->add_node(parent, name, obj)) {
		jsonwriter->delete(obj);
		Assert_retr(NULL, 0);
	}
	return obj;
}

JsonW *jsonwriter_add_string_to_array(JsonW *parent, const char *str)
{
	nullpo_retr(NULL, parent);
	nullpo_retr(NULL, str);
	Assert_retr(NULL, cJSON_IsArray(parent));
	JsonW *obj = jsonwriter->new_string(str);
	Assert_retr(NULL, cJSON_IsString(obj));
	if (!jsonwriter->add_node_to_array(parent, obj)) {
		jsonwriter->delete(obj);
		Assert_retr(NULL, 0);
	}
	return obj;
}

JsonW *jsonwriter_add_strings_to_array(JsonW *parent, ...)
{
	nullpo_retr(NULL, parent);
	Assert_retr(NULL, cJSON_IsArray(parent));

	va_list va;
	va_start(va, parent);
	const char *str;
	JsonW *obj = NULL;
	while ((str = va_arg(va, const char*)) != NULL) {
		obj = jsonwriter->new_string(str);
		if (!cJSON_IsString(obj)) {
			Assert_report(0);
			va_end(va);
			return obj;
		}
		if (!jsonwriter->add_node_to_array(parent, obj)) {
			jsonwriter->delete(obj);
			Assert_retr(NULL, 0);
		}
	}
	va_end(va);

	return obj;
}

char* jsonwriter_get_formatted_string(const JsonW *parent)
{
	nullpo_retr(NULL, parent);

	return cJSON_Print(parent);
}

char* jsonwriter_get_string(const JsonW *parent)
{
	nullpo_retr(NULL, parent);

	return cJSON_PrintUnformatted(parent);
}

void jsonwriter_print(const JsonW *parent)
{
	char *str = jsonwriter->get_formatted_string(parent);
	ShowInfo("Json string: '%s'\n", str);
	jsonwriter->free(str);
}

void jsonwriter_free(char *ptr)
{
	// need call real free because cJSON using direct malloc/free
	free(ptr);
}

void jsonwriter_delete(JsonW *ptr)
{
	cJSON_Delete(ptr);
}

void jsonwriter_defaults(void)
{
	jsonwriter = &jsonwriter_s;
	/* core */
	jsonwriter->init = do_init_jsonwriter;
	jsonwriter->final = do_final_jsonwriter;
	jsonwriter->create = jsonwriter_create;
	jsonwriter->new_array = jsonwriter_new_array;
	jsonwriter->new_object = jsonwriter_new_object;
	jsonwriter->new_string = jsonwriter_new_string;
	jsonwriter->new_null = jsonwriter_new_null;
	jsonwriter->add_node = jsonwriter_add_node;
	jsonwriter->add_node_to_array = jsonwriter_add_node_to_array;
	jsonwriter->add_new_array = jsonwriter_add_new_array;
	jsonwriter->add_new_object = jsonwriter_add_new_object;
	jsonwriter->add_new_null = jsonwriter_add_new_null;
	jsonwriter->add_string_to_array = jsonwriter_add_string_to_array;
	jsonwriter->add_strings_to_array = jsonwriter_add_strings_to_array;
	jsonwriter->get_string = jsonwriter_get_string;
	jsonwriter->get_formatted_string = jsonwriter_get_formatted_string;
	jsonwriter->print = jsonwriter_print;
	jsonwriter->free = jsonwriter_free;
	jsonwriter->delete = jsonwriter_delete;
}
