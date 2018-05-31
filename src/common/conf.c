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

#include "conf.h"

#include "common/showmsg.h" // ShowError
#include "common/strlib.h" // safestrncpy
#include "common/utils.h" // exists

#include <libconfig/libconfig.h>

/* interface source */
struct libconfig_interface libconfig_s;
struct libconfig_interface *libconfig;

/**
 * Initializes 'config' and loads a configuration file.
 *
 * Shows error and destroys 'config' in case of failure.
 * It is the caller's care to destroy 'config' in case of success.
 *
 * @param config          The config file to initialize.
 * @param config_filename The file to read.
 *
 * @retval CONFIG_TRUE  in case of success.
 * @retval CONFIG_FALSE in case of failure.
 */
int config_load_file(struct config_t *config, const char *config_filename)
{
	libconfig->init(config);
	if (!exists(config_filename)) {
		ShowError("Unable to load '%s' - File not found\n", config_filename);
		return CONFIG_FALSE;
	}
	if (libconfig->read_file_src(config, config_filename) != CONFIG_TRUE) {
		ShowError("%s:%d - %s\n", config_error_file(config),
		          config_error_line(config), config_error_text(config));
		libconfig->destroy(config);
		return CONFIG_FALSE;
	}
	return CONFIG_TRUE;
}

//
// Functions to copy settings from libconfig/contrib
//
void config_setting_copy_simple(struct config_setting_t *parent, const struct config_setting_t *src)
{
	if (config_setting_is_aggregate(src)) {
		libconfig->setting_copy_aggregate(parent, src);
	} else {
		struct config_setting_t *set;

		if( libconfig->setting_get_member(parent, config_setting_name(src)) != NULL )
			return;

		if ((set = libconfig->setting_add(parent, config_setting_name(src), config_setting_type(src))) == NULL)
			return;

		if (CONFIG_TYPE_INT == config_setting_type(src)) {
			libconfig->setting_set_int(set, libconfig->setting_get_int(src));
			libconfig->setting_set_format(set, src->format);
		} else if (CONFIG_TYPE_INT64 == config_setting_type(src)) {
			libconfig->setting_set_int64(set, libconfig->setting_get_int64(src));
			libconfig->setting_set_format(set, src->format);
		} else if (CONFIG_TYPE_FLOAT == config_setting_type(src)) {
			libconfig->setting_set_float(set, libconfig->setting_get_float(src));
		} else if (CONFIG_TYPE_STRING == config_setting_type(src)) {
			libconfig->setting_set_string(set, libconfig->setting_get_string(src));
		} else if (CONFIG_TYPE_BOOL == config_setting_type(src)) {
			libconfig->setting_set_bool(set, libconfig->setting_get_bool(src));
		}
	}
}

void config_setting_copy_elem(struct config_setting_t *parent, const struct config_setting_t *src)
{
	struct config_setting_t *set = NULL;

	if (config_setting_is_aggregate(src))
		libconfig->setting_copy_aggregate(parent, src);
	else if (CONFIG_TYPE_INT == config_setting_type(src)) {
		set = libconfig->setting_set_int_elem(parent, -1, libconfig->setting_get_int(src));
		libconfig->setting_set_format(set, src->format);
	} else if (CONFIG_TYPE_INT64 == config_setting_type(src)) {
		set = libconfig->setting_set_int64_elem(parent, -1, libconfig->setting_get_int64(src));
		libconfig->setting_set_format(set, src->format);
	} else if (CONFIG_TYPE_FLOAT == config_setting_type(src)) {
		libconfig->setting_set_float_elem(parent, -1, libconfig->setting_get_float(src));
	} else if (CONFIG_TYPE_STRING == config_setting_type(src)) {
		libconfig->setting_set_string_elem(parent, -1, libconfig->setting_get_string(src));
	} else if (CONFIG_TYPE_BOOL == config_setting_type(src)) {
		libconfig->setting_set_bool_elem(parent, -1, libconfig->setting_get_bool(src));
	}
}

void config_setting_copy_aggregate(struct config_setting_t *parent, const struct config_setting_t *src)
{
	struct config_setting_t *newAgg;
	int i, n;

	if( libconfig->setting_get_member(parent, config_setting_name(src)) != NULL )
		return;

	newAgg = libconfig->setting_add(parent, config_setting_name(src), config_setting_type(src));

	if (newAgg == NULL)
		return;

	n = config_setting_length(src);

	for (i = 0; i < n; i++) {
		if (config_setting_is_group(src)) {
			libconfig->setting_copy_simple(newAgg, libconfig->setting_get_elem(src, i));
		} else {
			libconfig->setting_copy_elem(newAgg, libconfig->setting_get_elem(src, i));
		}
	}
}

int config_setting_copy(struct config_setting_t *parent, const struct config_setting_t *src)
{
	if (!config_setting_is_group(parent) && !config_setting_is_list(parent))
		return CONFIG_FALSE;

	if (config_setting_is_aggregate(src)) {
		libconfig->setting_copy_aggregate(parent, src);
	} else {
		libconfig->setting_copy_simple(parent, src);
	}
	return CONFIG_TRUE;
}

/**
 * Converts the value of a setting that is type CONFIG_TYPE_BOOL to bool.
 *
 * @param setting The setting to read.
 *
 * @return The converted value.
 * @retval false in case of failure.
 */
bool config_setting_get_bool_real(const struct config_setting_t *setting)
{
	if (setting == NULL || setting->type != CONFIG_TYPE_BOOL)
		return false;

	return setting->value.ival ? true : false;
}

/**
 * Same as config_setting_lookup_bool, but uses bool instead of int.
 *
 * @param[in]  setting The setting to read.
 * @param[in]  name    The setting name to lookup.
 * @param[out] value   The output value.
 *
 * @retval CONFIG_TRUE  in case of success.
 * @retval CONFIG_FALSE in case of failure.
 */
int config_setting_lookup_bool_real(const struct config_setting_t *setting, const char *name, bool *value)
{
	struct config_setting_t *member = config_setting_get_member(setting, name);

	if (!member)
		return CONFIG_FALSE;

	if (config_setting_type(member) != CONFIG_TYPE_BOOL)
		return CONFIG_FALSE;

	*value = config_setting_get_bool_real(member);

	return CONFIG_TRUE;
}

/**
 * Converts and returns a configuration that is CONFIG_TYPE_INT to unsigned int (uint32).
 *
 * @param setting The setting to read.
 *
 * @return The converted value.
 * @retval 0 in case of failure.
 */
uint32 config_setting_get_uint32(const struct config_setting_t *setting)
{
	if (setting == NULL || setting->type != CONFIG_TYPE_INT)
		return 0;

	if (setting->value.ival < 0)
		return 0;

	return (uint32)setting->value.ival;
}

/**
 * Looks up a configuration entry of type CONFIG_TYPE_INT and reads it as uint32.
 *
 * @param[in]  setting The setting to read.
 * @param[in]  name    The setting name to lookup.
 * @param[out] value   The output value.
 *
 * @retval CONFIG_TRUE  in case of success.
 * @retval CONFIG_FALSE in case of failure.
 */
int config_setting_lookup_uint32(const struct config_setting_t *setting, const char *name, uint32 *value)
{
	struct config_setting_t *member = config_setting_get_member(setting, name);

	if (!member)
		return CONFIG_FALSE;

	if (config_setting_type(member) != CONFIG_TYPE_INT)
		return CONFIG_FALSE;

	*value = config_setting_get_uint32(member);

	return CONFIG_TRUE;
}

/**
 * Converts and returns a configuration that is CONFIG_TYPE_INT to uint16
 *
 * @param setting The setting to read.
 *
 * @return The converted value.
 * @retval 0 in case of failure.
 */
uint16 config_setting_get_uint16(const struct config_setting_t *setting)
{
	if (setting == NULL || setting->type != CONFIG_TYPE_INT)
		return 0;

	if (setting->value.ival > UINT16_MAX)
		return UINT16_MAX;
	if (setting->value.ival < UINT16_MIN)
		return UINT16_MIN;

	return (uint16)setting->value.ival;
}

/**
 * Looks up a configuration entry of type CONFIG_TYPE_INT and reads it as uint16.
 *
 * @param[in]  setting The setting to read.
 * @param[in]  name    The setting name to lookup.
 * @param[out] value   The output value.
 *
 * @retval CONFIG_TRUE  in case of success.
 * @retval CONFIG_FALSE in case of failure.
 */
int config_setting_lookup_uint16(const struct config_setting_t *setting, const char *name, uint16 *value)
{
	struct config_setting_t *member = config_setting_get_member(setting, name);

	if (!member)
		return CONFIG_FALSE;

	if (config_setting_type(member) != CONFIG_TYPE_INT)
		return CONFIG_FALSE;

	*value = config_setting_get_uint16(member);

	return CONFIG_TRUE;
}

/**
 * Converts and returns a configuration that is CONFIG_TYPE_INT to int16
 *
 * @param setting The setting to read.
 *
 * @return The converted value.
 * @retval 0 in case of failure.
 */
int16 config_setting_get_int16(const struct config_setting_t *setting)
{
	if (setting == NULL || setting->type != CONFIG_TYPE_INT)
		return 0;

	if (setting->value.ival > INT16_MAX)
		return INT16_MAX;
	if (setting->value.ival < INT16_MIN)
		return INT16_MIN;

	return (int16)setting->value.ival;
}

/**
 * Looks up a configuration entry of type CONFIG_TYPE_INT and reads it as int16.
 *
 * @param[in]  setting The setting to read.
 * @param[in]  name    The setting name to lookup.
 * @param[out] value   The output value.
 *
 * @retval CONFIG_TRUE  in case of success.
 * @retval CONFIG_FALSE in case of failure.
 */
int config_setting_lookup_int16(const struct config_setting_t *setting, const char *name, int16 *value)
{
	struct config_setting_t *member = config_setting_get_member(setting, name);

	if (!member)
		return CONFIG_FALSE;

	if (config_setting_type(member) != CONFIG_TYPE_INT)
		return CONFIG_FALSE;

	*value = config_setting_get_int16(member);

	return CONFIG_TRUE;
}

/**
 * Looks up a configuration entry of type CONFIG_TYPE_STRING inside a struct config_setting_t and copies it into a (non-const) char buffer.
 *
 * @param[in]  setting  The setting to read.
 * @param[in]  name     The setting name to lookup.
 * @param[out] out      The output buffer.
 * @param[in]  out_size The size of the output buffer.
 *
 * @retval CONFIG_TRUE  in case of success.
 * @retval CONFIG_FALSE in case of failure.
 */
int config_setting_lookup_mutable_string(const struct config_setting_t *setting, const char *name, char *out, size_t out_size)
{
	const char *str = NULL;

	if (libconfig->setting_lookup_string(setting, name, &str) == CONFIG_TRUE) {
		safestrncpy(out, str, out_size);
		return CONFIG_TRUE;
	}

	return CONFIG_FALSE;
}

/**
 * Looks up a configuration entry of type CONFIG_TYPE_STRING inside a struct config_t and copies it into a (non-const) char buffer.
 *
 * @param[in]  config   The configuration to read.
 * @param[in]  name     The setting name to lookup.
 * @param[out] out      The output buffer.
 * @param[in]  out_size The size of the output buffer.
 *
 * @retval CONFIG_TRUE  in case of success.
 * @retval CONFIG_FALSE in case of failure.
 */
int config_lookup_mutable_string(const struct config_t *config, const char *name, char *out, size_t out_size)
{
	const char *str = NULL;

	if (libconfig->lookup_string(config, name, &str) == CONFIG_TRUE) {
		safestrncpy(out, str, out_size);
		return CONFIG_TRUE;
	}

	return CONFIG_FALSE;
}

/**
 * Wrapper for config_setting_get_int64() using defined-size variables
 *
 * @see config_setting_get_int64_real()
 */
int64 config_setting_get_int64_real(const struct config_setting_t *setting)
{
	return (int64)config_setting_get_int64(setting);
}

/**
 * Wrapper for config_setting_lookup_int64() using defined-size variables
 *
 * @see config_setting_lookup_int64()
 */
int config_setting_lookup_int64_real(const struct config_setting_t *setting, const char *name, int64 *value)
{
	long long int lli = 0;

	if (config_setting_lookup_int64(setting, name, &lli) != CONFIG_TRUE)
		return CONFIG_FALSE;

	*value = (int64)lli;

	return CONFIG_TRUE;
}

/**
 * Wrapper for config_setting_set_int64() using defined-size variables
 *
 * @see config_setting_set_int64()
 */
int config_setting_set_int64_real(struct config_setting_t *setting, int64 value)
{
	return config_setting_set_int64(setting, (long long int)value);
}

/**
 * Wrapper for config_setting_get_int64_elem() using defined-size variables
 *
 * @see config_setting_get_int64_elem()
 */
int64 config_setting_get_int64_elem_real(const struct config_setting_t *setting, int idx)
{
	return (int64)config_setting_get_int64_elem(setting, idx);
}

/**
 * Wrapper for config_setting_set_int64_elem() using defined-size variables
 *
 * @see config_setting_set_int64_elem()
 */
struct config_setting_t *config_setting_set_int64_elem_real(struct config_setting_t *setting, int idx, int64 value)
{
	return config_setting_set_int64_elem(setting, idx, (long long int)value);
}

/**
 * Wrapper for config_lookup_int64() using defined-size variables
 *
 * @see config_lookup_int64()
 */
int config_lookup_int64_real(const struct config_t *config, const char *filepath, int64 *value)
{
	long long int lli = 0;

	if (config_lookup_int64(config, filepath, &lli) != CONFIG_TRUE)
		return CONFIG_FALSE;

	*value = (int64)lli;

	return CONFIG_TRUE;
}

void libconfig_defaults(void) {
	libconfig = &libconfig_s;

	libconfig->read = config_read;
	libconfig->write = config_write;
	/* */
	libconfig->set_options = config_set_options;
	libconfig->get_options = config_get_options;
	/* */
	libconfig->read_string = config_read_string;
	libconfig->read_file_src = config_read_file;
	libconfig->write_file = config_write_file;
	/* */
	libconfig->set_destructor = config_set_destructor;
	libconfig->set_include_dir = config_set_include_dir;
	/* */
	libconfig->init = config_init;
	libconfig->destroy = config_destroy;
	/* */
	libconfig->setting_get_int = config_setting_get_int;
	libconfig->setting_get_int64 = config_setting_get_int64_real;
	libconfig->setting_get_float = config_setting_get_float;
	libconfig->setting_get_bool = config_setting_get_bool;
	libconfig->setting_get_string = config_setting_get_string;
	/* */
	libconfig->setting_lookup = config_setting_lookup;
	libconfig->setting_lookup_int = config_setting_lookup_int;
	libconfig->setting_lookup_int64 = config_setting_lookup_int64_real;
	libconfig->setting_lookup_float = config_setting_lookup_float;
	libconfig->setting_lookup_bool = config_setting_lookup_bool;
	libconfig->setting_lookup_string = config_setting_lookup_string;
	/* */
	libconfig->setting_set_int = config_setting_set_int;
	libconfig->setting_set_int64 = config_setting_set_int64_real;
	libconfig->setting_set_float = config_setting_set_float;
	libconfig->setting_set_bool = config_setting_set_bool;
	libconfig->setting_set_string = config_setting_set_string;
	/* */
	libconfig->setting_set_format = config_setting_set_format;
	libconfig->setting_get_format = config_setting_get_format;
	/* */
	libconfig->setting_get_int_elem = config_setting_get_int_elem;
	libconfig->setting_get_int64_elem = config_setting_get_int64_elem_real;
	libconfig->setting_get_float_elem = config_setting_get_float_elem;
	libconfig->setting_get_bool_elem = config_setting_get_bool_elem;
	libconfig->setting_get_string_elem = config_setting_get_string_elem;
	/* */
	libconfig->setting_set_int_elem = config_setting_set_int_elem;
	libconfig->setting_set_int64_elem = config_setting_set_int64_elem_real;
	libconfig->setting_set_float_elem = config_setting_set_float_elem;
	libconfig->setting_set_bool_elem = config_setting_set_bool_elem;
	libconfig->setting_set_string_elem = config_setting_set_string_elem;
	/* */
	libconfig->setting_index = config_setting_index;
	libconfig->setting_length = config_setting_length;
	/* */
	libconfig->setting_get_elem = config_setting_get_elem;
	libconfig->setting_get_member = config_setting_get_member;
	/* */
	libconfig->setting_add = config_setting_add;
	libconfig->setting_remove = config_setting_remove;
	libconfig->setting_remove_elem = config_setting_remove_elem;
	/* */
	libconfig->setting_set_hook = config_setting_set_hook;
	/* */
	libconfig->lookup = config_lookup;
	/* */
	libconfig->lookup_int = config_lookup_int;
	libconfig->lookup_int64 = config_lookup_int64_real;
	libconfig->lookup_float = config_lookup_float;
	libconfig->lookup_bool = config_lookup_bool;
	libconfig->lookup_string = config_lookup_string;
	/* those are custom and are from src/common/conf.c */
	libconfig->load_file = config_load_file;
	libconfig->setting_copy_simple = config_setting_copy_simple;
	libconfig->setting_copy_elem = config_setting_copy_elem;
	libconfig->setting_copy_aggregate = config_setting_copy_aggregate;
	libconfig->setting_copy = config_setting_copy;

	/* Functions to get different types */
	libconfig->setting_get_bool_real = config_setting_get_bool_real;
	libconfig->setting_get_uint32 = config_setting_get_uint32;
	libconfig->setting_get_uint16 = config_setting_get_uint16;
	libconfig->setting_get_int16 = config_setting_get_int16;

	/* Functions to lookup different types */
	libconfig->setting_lookup_int16 = config_setting_lookup_int16;
	libconfig->setting_lookup_bool_real = config_setting_lookup_bool_real;
	libconfig->setting_lookup_uint32 = config_setting_lookup_uint32;
	libconfig->setting_lookup_uint16 = config_setting_lookup_uint16;
	libconfig->setting_lookup_mutable_string = config_setting_lookup_mutable_string;
	libconfig->lookup_mutable_string = config_lookup_mutable_string;
}
