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
#ifndef COMMON_CONF_H
#define COMMON_CONF_H

#include "common/hercules.h"

#include <libconfig/libconfig.h>

/**
 * The libconfig interface -- specially for plugins, but we enforce it throughout the core to be consistent
 **/
struct libconfig_interface {
	int (*read) (struct config_t *config, FILE *stream);
	void (*write) (const struct config_t *config, FILE *stream);
	/* */
	void (*set_options) (struct config_t *config, int options);
	int (*get_options) (const struct config_t *config);
	/* */
	int (*read_string) (struct config_t *config, const char *str);
	int (*read_file_src) (struct config_t *config, const char *filename);
	int (*write_file) (struct config_t *config, const char *filename);

	void (*set_destructor) (struct config_t *config, void (*destructor)(void *));
	void (*set_include_dir) (struct config_t *config, const char *include_dir);

	void (*init) (struct config_t *config);
	void (*destroy) (struct config_t *config);

	int (*setting_get_int) (const struct config_setting_t *setting);
	int64 (*setting_get_int64) (const struct config_setting_t *setting);
	double (*setting_get_float) (const struct config_setting_t *setting);

	int (*setting_get_bool) (const struct config_setting_t *setting);

	const char * (*setting_get_string) (const struct config_setting_t *setting);

	struct config_setting_t * (*setting_lookup) (struct config_setting_t *setting, const char *name);
	int (*setting_lookup_int) (const struct config_setting_t *setting, const char *name, int *value);
	int (*setting_lookup_int64) (const struct config_setting_t *setting, const char *name, int64 *value);
	int (*setting_lookup_float) (const struct config_setting_t *setting, const char *name, double *value);
	int (*setting_lookup_bool) (const struct config_setting_t *setting, const char *name, int *value);
	int (*setting_lookup_string) (const struct config_setting_t *setting, const char *name, const char **value);
	int (*setting_set_int) (struct config_setting_t *setting, int value);
	int (*setting_set_int64) (struct config_setting_t *setting, int64 value);
	int (*setting_set_float) (struct config_setting_t *setting, double value);
	int (*setting_set_bool) (struct config_setting_t *setting, int value);
	int (*setting_set_string) (struct config_setting_t *setting, const char *value);

	int (*setting_set_format) (struct config_setting_t *setting, short format);
	short (*setting_get_format) (const struct config_setting_t *setting);

	int (*setting_get_int_elem) (const struct config_setting_t *setting, int idx);
	int64 (*setting_get_int64_elem) (const struct config_setting_t *setting, int idx);
	double (*setting_get_float_elem) (const struct config_setting_t *setting, int idx);
	int (*setting_get_bool_elem) (const struct config_setting_t *setting, int idx);
	const char * (*setting_get_string_elem) (const struct config_setting_t *setting, int idx);
	struct config_setting_t * (*setting_set_int_elem) (struct config_setting_t *setting, int idx, int value);
	struct config_setting_t * (*setting_set_int64_elem) (struct config_setting_t *setting, int idx, int64 value);
	struct config_setting_t * (*setting_set_float_elem) (struct config_setting_t *setting, int idx, double value);
	struct config_setting_t * (*setting_set_bool_elem) (struct config_setting_t *setting, int idx, int value);
	struct config_setting_t * (*setting_set_string_elem) (struct config_setting_t *setting, int idx, const char *value);

	int (*setting_index) (const struct config_setting_t *setting);
	int (*setting_length) (const struct config_setting_t *setting);

	struct config_setting_t * (*setting_get_elem) (const struct config_setting_t *setting, unsigned int idx);
	struct config_setting_t * (*setting_get_member) (const struct config_setting_t *setting, const char *name);

	struct config_setting_t * (*setting_add) (struct config_setting_t *parent, const char *name, int type);
	int (*setting_remove) (struct config_setting_t *parent, const char *name);

	int (*setting_remove_elem) (struct config_setting_t *parent, unsigned int idx);
	void (*setting_set_hook) (struct config_setting_t *setting, void *hook);

	struct config_setting_t * (*lookup) (const struct config_t *config, const char *filepath);
	int (*lookup_int) (const struct config_t *config, const char *filepath, int *value);
	int (*lookup_int64) (const struct config_t *config, const char *filepath, int64 *value);
	int (*lookup_float) (const struct config_t *config, const char *filepath, double *value);
	int (*lookup_bool) (const struct config_t *config, const char *filepath, int *value);
	int (*lookup_string) (const struct config_t *config, const char *filepath, const char **value);

	/* those are custom and are from src/common/conf.c */
	/* Functions to copy settings from libconfig/contrib */
	int (*load_file) (struct config_t *config, const char *config_filename);
	void (*setting_copy_simple) (struct config_setting_t *parent, const struct config_setting_t *src);
	void (*setting_copy_elem) (struct config_setting_t *parent, const struct config_setting_t *src);
	void (*setting_copy_aggregate) (struct config_setting_t *parent, const struct config_setting_t *src);
	int (*setting_copy) (struct config_setting_t *parent, const struct config_setting_t *src);
	/* Functions to get other types */
	bool (*setting_get_bool_real) (const struct config_setting_t *setting);
	uint32 (*setting_get_uint32) (const struct config_setting_t *setting);
	uint16 (*setting_get_uint16) (const struct config_setting_t *setting);
	int16 (*setting_get_int16) (const struct config_setting_t *setting);

	int (*setting_lookup_bool_real) (const struct config_setting_t *setting, const char *name, bool *value);
	int (*setting_lookup_uint32) (const struct config_setting_t *setting, const char *name, uint32 *value);
	int (*setting_lookup_uint16) (const struct config_setting_t *setting, const char *name, uint16 *value);
	int (*setting_lookup_int16) (const struct config_setting_t *setting, const char *name, int16 *value);
	int (*setting_lookup_mutable_string) (const struct config_setting_t *setting, const char *name, char *out, size_t out_size);
	int (*lookup_mutable_string) (const struct config_t *config, const char *name, char *out, size_t out_size);
};

#ifdef HERCULES_CORE
void libconfig_defaults(void);
#endif // HERCULES_CORE

HPShared struct libconfig_interface *libconfig;

#endif // COMMON_CONF_H
