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
#ifndef API_JSONWRITER_H
#define API_JSONWRITER_H

#include "common/hercules.h"

#include <cJSON/cJSON.h>

#include <stdarg.h>

typedef cJSON JsonW;
typedef cJSON_bool JsonWBool;

/**
 * jsonwriter.c Interface
 **/
struct jsonwriter_interface {
	int (*init) (bool minimal);
	void (*final) (void);

	JsonW *(*create) (const char *text);
	JsonW *(*new_array) (void);
	JsonW *(*new_object) (void);
	JsonW *(*new_string) (const char *str);
	JsonW *(*new_null) (void);
	JsonWBool (*add_node) (JsonW *parent, const char *name, JsonW *child);
	JsonWBool (*add_node_to_array) (JsonW *parent, JsonW *child);
	JsonW *(*add_new_array) (JsonW *parent, const char *name);
	JsonW *(*add_new_object) (JsonW *parent, const char *name);
	JsonW *(*add_new_null) (JsonW *parent, const char *name);
	JsonW *(*add_string_to_array) (JsonW *parent, const char *str);
	JsonW *(*add_strings_to_array) (JsonW *parent, ...);

	void (*print) (const JsonW *parent);
	char* (*get_string) (const JsonW *parent);
	char* (*get_formatted_string) (const JsonW *parent);
	void (*free) (char *ptr);
	void (*delete) (JsonW *ptr);
};

#ifdef HERCULES_CORE
void jsonwriter_defaults(void);
#endif // HERCULES_CORE

HPShared struct jsonwriter_interface *jsonwriter;

#endif /* API_JSONWRITER_H */
