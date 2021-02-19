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
#ifndef API_JSONPARSER_H
#define API_JSONPARSER_H

#include "common/hercules.h"

#include <cJSON/cJSON.h>

#include <stdarg.h>

typedef cJSON JsonP;
typedef cJSON_bool JsonPBool;

#define JSONPARSER_FOR_EACH(element, array) for(JsonP *element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

/**
 * jsonparser.c Interface
 **/
struct jsonparser_interface {
	int (*init) (bool minimal);
	void (*final) (void);
	JsonP *(*parse) (const char *text);
	void (*print) (const JsonP *parent);
	char* (*get_string) (const JsonP *parent);
	char* (*get_formatted_string) (const JsonP *parent);
	bool (*is_null) (const JsonP *parent);
	bool (*is_null_or_missing) (const JsonP *parent);
	JsonP *(*get) (const JsonP *parent, const char *name);
	int (*get_array_size) (const JsonP *parent);
	char *(*get_string_value) (const JsonP *parent);
	double (*get_number_value) (const JsonP *parent);
	int (*get_int_value) (const JsonP *parent);
	char* (*get_child_string_value) (const JsonP *parent, const char *name);
	double (*get_child_number_value) (const JsonP *parent, const char *name);
	int (*get_child_int_value) (const JsonP *parent, const char *name);
	void (*free) (char *ptr);
	void (*delete) (JsonP *ptr);
};

#ifdef HERCULES_CORE
void jsonparser_defaults(void);
#endif // HERCULES_CORE

HPShared struct jsonparser_interface *jsonparser;

#endif /* API_JSONPARSER_H */
