/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2015  Hercules Dev Team
 * Copyright (C) 2015  Haru <haru@dotalux.com>
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

/// Base author: Haru <haru@dotalux.com>

#define HERCULES_CORE

#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/core.h"
#include "common/showmsg.h"
#include "common/strlib.h"

#include <stdlib.h>

#define TEST(name, function, ...) do { \
	const char *message = NULL; \
	ShowMessage("-------------------------------------------------------------------------------\n"); \
	ShowNotice("Testing %s...\n", (name)); \
	if ((message = (function)(##__VA_ARGS__)) != NULL) { \
		ShowError("Failed. %s\n", message); \
		ShowMessage("===============================================================================\n"); \
		ShowFatalError("Failure. Aborting further tests.\n"); \
		exit(EXIT_FAILURE); \
	} \
	ShowInfo("Test passed.\n"); \
} while (false)

static const char *test_libconfig_truefalse(void)
{
	if (CONFIG_TRUE != true) {
		return "CONFIG_TRUE != true";
	}
	if (CONFIG_FALSE != false) {
		return "CONFIG_FALSE != false";
	}
	return NULL;
}

static const char *test_libconfig_defaults(void)
{
	if (libconfig == NULL) {
		return "Unable to find libconfig interface.";
	}
	if (libconfig->init == NULL) {
		return "Unable to find libconfig methods";
	}
	if (libconfig->read_file_src == NULL) {
		return "Unable to find libconfig core methods";
	}
	return NULL;
}

static const char *test_libconfig_init_destroy(void)
{
	struct config_t config;
	libconfig->init(&config);
	if (config.root == NULL || config.root != config_root_setting(&config)) {
		return "Unable to create config.";
	}
	libconfig->destroy(&config);
	if (config.root != NULL) {
		return "Unable to destroy config.";
	}
	return NULL;
}

static const char *test_libconfig_read_file_src(void)
{
	struct config_t config;
#define FILENAME "src/test/libconfig/test.conf"
	if (libconfig->read_file_src(&config, FILENAME) == CONFIG_FALSE) {
		libconfig->destroy(&config);
		return "Unable to read file '" FILENAME "'.";
	}
#undef FILENAME
	if (config.root == NULL) {
		libconfig->destroy(&config);
		return "Invalid config.";
	}
	libconfig->destroy(&config);
	return NULL;
}

static const char *test_libconfig_read(void)
{
	struct config_t config;
#define FILENAME "src/test/libconfig/test.conf"
	FILE *fp = fopen(FILENAME, "r");
	if (!fp) {
		return "File not found: '" FILENAME "'.";
	}
	if (libconfig->read(&config, fp) == CONFIG_FALSE) {
		fclose(fp);
		libconfig->destroy(&config);
		return "Unable to read from file '" FILENAME "'.";
	}
#undef FILENAME
	if (config.root == NULL) {
		libconfig->destroy(&config);
		return "Invalid config.";
	}
	libconfig->destroy(&config);
	return NULL;
}

static const char *test_libconfig_load_file(void)
{
	struct config_t config;
#define FILENAME "src/test/libconfig/test.conf"
	if (libconfig->load_file(&config, FILENAME) == CONFIG_FALSE) {
		return "Unable to read file '" FILENAME "'.";
	}
#undef FILENAME
	if (config.root == NULL || !config_setting_is_root(config.root)) {
		libconfig->destroy(&config);
		return "Invalid config.";
	}
	libconfig->destroy(&config);
	return NULL;
}

static const char *test_libconfig_write(void)
{
	//void (*write) (const struct config_t *config, FILE *stream);
	return "TEST NOT IMPLEMENTED";
}

static const char *test_libconfig_write_file(void)
{
	//int (*write_file) (struct config_t *config, const char *filename);
	return "TEST NOT IMPLEMENTED";
}

static const char *test_libconfig_read_string(void)
{
	struct config_t config;
	if (libconfig->read_string(&config, "") == CONFIG_FALSE) {
		libconfig->destroy(&config);
		return "Unable to read from string.";
	}
	if (config.root == NULL) {
		libconfig->destroy(&config);
		return "Invalid config.";
	}
	libconfig->destroy(&config);
	return NULL;
}

static const char *test_libconfig_syntax(void)
{
	struct config_t config;
	const char *input = "/* Test File */\n"
		"Setting_Int: 1;\n"
		"Setting_Int64: 1L;\n"
		"Setting_Float: 1.0;\n"
		"Setting_Bool: true;\n"
		"Setting_String: \"1\";\n"
		"Setting_Array: [ ];\n"
		"Setting_Group: { };\n"
		"Setting_List: ( );\n"
		"/* End test file */\n";

	if (libconfig->read_string(&config, input) == CONFIG_FALSE) {
		libconfig->destroy(&config);
		return "Unable to read from string.";
	}
	if (config.root == NULL) {
		libconfig->destroy(&config);
		return "Invalid config.";
	}
	libconfig->destroy(&config);
	return NULL;
}

static const char *test_libconfig_set_include_dir(void)
{
	//void (*set_include_dir) (struct config_t *config, const char *include_dir);
	return "TEST NOT IMPLEMENTED";
}

static const char *test_libconfig_lookup(void)
{
	struct config_t config;
	struct config_setting_t *t = NULL;
	int32 i32;
	int64 i64;
	double f;
	const char *str;
	const char *input = "/* Test File */\n"
		"Setting_Int: 1;\n"
		"Setting_Int64: 1L;\n"
		"Setting_Float: 1.0;\n"
		"Setting_Bool: true;\n"
		"Setting_String: \"1\";\n"
		"Setting_Array: [ ];\n"
		"Setting_Group: { };\n"
		"Setting_List: ( );\n"
		"/* End test file */\n";

	if (libconfig->read_string(&config, input) == CONFIG_FALSE) {
		libconfig->destroy(&config);
		return "Unable to parse configuration.";
	}

	if ((t = libconfig->lookup(&config, "Setting_Int")) == NULL) {
		libconfig->destroy(&config);
		return "libconfig->lookup failed.";
	}

	if ((t = libconfig->setting_lookup(config.root, "Setting_Int")) == NULL) {
		libconfig->destroy(&config);
		return "libconfig->setting_lookup failed.";
	}

	if (libconfig->lookup_int(&config, "Setting_Int", &i32) == CONFIG_FALSE || i32 != 1) {
		libconfig->destroy(&config);
		return "libconfig->lookup_int failed.";
	}

	if (libconfig->lookup_int64(&config, "Setting_Int64", &i64) == CONFIG_FALSE || i64 != 1) {
		libconfig->destroy(&config);
		return "libconfig->lookup_int64 failed.";
	}

	if (libconfig->lookup_float(&config, "Setting_Float", &f) == CONFIG_FALSE || f < 1.0 - 0.1 || f > 1.0 + 0.1) {
		libconfig->destroy(&config);
		return "libconfig->lookup_float failed.";
	}

	if (libconfig->lookup_bool(&config, "Setting_Bool", &i32) == CONFIG_FALSE || i32 != 1) {
		libconfig->destroy(&config);
		return "libconfig->lookup_bool failed.";
	}

	if (libconfig->lookup_string(&config, "Setting_String", &str) == CONFIG_FALSE || str == NULL || str[0] != '1' || str[1] != '\0') {
		libconfig->destroy(&config);
		return "libconfig->lookup_string failed.";
	}

	libconfig->destroy(&config);

	return NULL;
}

static const char *test_libconfig_setting_get(void)
{
	struct config_t config;
	struct config_setting_t *t = NULL;
	double f;
	const char *str;
	const char *input = "/* Test File */\n"
		"Setting_Int: 1;\n"
		"Setting_Int64: 1L;\n"
		"Setting_Float: 1.0;\n"
		"Setting_Bool: true;\n"
		"Setting_String: \"1\";\n"
		"Setting_Array: [ ];\n"
		"Setting_Group: { };\n"
		"Setting_List: ( );\n"
		"/* End test file */\n";

	if (libconfig->read_string(&config, input) == CONFIG_FALSE) {
		libconfig->destroy(&config);
		return "Unable to parse configuration.";
	}

	if ((t = libconfig->lookup(&config, "Setting_Int")) == NULL || libconfig->setting_get_int(t) != 1) {
		libconfig->destroy(&config);
		return "libconfig->setting_get_int failed.";
	}

	if ((t = libconfig->lookup(&config, "Setting_Int64")) == NULL || libconfig->setting_get_int64(t) != 1) {
		libconfig->destroy(&config);
		return "libconfig->lookup_int64 failed.";
	}

	if ((t = libconfig->lookup(&config, "Setting_Float")) == NULL || (f = libconfig->setting_get_float(t)) < 1.0 - 0.1 || f > 1.0 + 0.1) {
		libconfig->destroy(&config);
		return "libconfig->lookup_float failed.";
	}

	if ((t = libconfig->lookup(&config, "Setting_Bool")) == NULL || libconfig->setting_get_bool(t) != 1) {
		libconfig->destroy(&config);
		return "libconfig->lookup_bool failed.";
	}

	if ((t = libconfig->lookup(&config, "Setting_String")) == NULL || (str = libconfig->setting_get_string(t)) == NULL || str[0] != '1' || str[1] != '\0') {
		libconfig->destroy(&config);
		return "libconfig->lookup_string failed.";
	}

	t = config_root_setting(&config);

	if (libconfig->setting_get_int_elem(t, 0) != 1) {
		libconfig->destroy(&config);
		return "libconfig->setting_get_int_elem failed.";
	}

	if (libconfig->setting_get_int64_elem(t, 1) != 1) {
		libconfig->destroy(&config);
		return "libconfig->setting_get_int64_elem failed.";
	}

	if ((f = libconfig->setting_get_float_elem(t, 2)) < 1.0 - 0.1 || f > 1.0 + 0.1) {
		libconfig->destroy(&config);
		return "libconfig->setting_get_float_elem failed.";
	}

	if (libconfig->setting_get_bool_elem(t, 3) != 1) {
		libconfig->destroy(&config);
		return "libconfig->setting_get_bool_elem failed.";
	}

	if ((str = libconfig->setting_get_string_elem(t, 4)) == NULL || str[0] != '1' || str[1] != '\0') {
		libconfig->destroy(&config);
		return "libconfig->setting_get_string_elem failed.";
	}

	if ((t = libconfig->setting_get_elem(config.root, 0)) == NULL || libconfig->setting_get_int(t) != 1) {
		libconfig->destroy(&config);
		return "libconfig->setting_get_elem failed.";
	}

	if ((t = libconfig->setting_get_member(config.root, "Setting_Int")) == NULL || libconfig->setting_get_int(t) != 1 || strcmp(config_setting_name(t), "Setting_Int") != 0) {
		libconfig->destroy(&config);
		return "libconfig->setting_get_member failed.";
	}

	if ((t = libconfig->setting_get_elem(config.root, 0)) == NULL || strcmp(config_setting_name(t), "Setting_Int") != 0) {
		libconfig->destroy(&config);
		return "config_setting_name failed.";
	}

	if ((t = libconfig->setting_get_member(config.root, "Setting_Int")) == NULL || libconfig->setting_index(t) != 0) {
		libconfig->destroy(&config);
		return "libconfig->setting_index failed.";
	}

	if (libconfig->setting_length(config.root) != 8) {
		libconfig->destroy(&config);
		return "libconfig->setting_length failed.";
	}

	libconfig->destroy(&config);
	return NULL;
}

static const char *test_libconfig_set(void)
{
	//int (*setting_set_int) (struct config_setting_t *setting ,int value);
	//int (*setting_set_int64) (struct config_setting_t *setting, long long value);
	//int (*setting_set_float) (struct config_setting_t *setting, double value);
	//int (*setting_set_bool) (struct config_setting_t *setting, int value);
	//int (*setting_set_string) (struct config_setting_t *setting, const char *value);
	return "TEST NOT IMPLEMENTED";
}

static const char *test_libconfig_setting_lookup(void)
{
	struct config_t config;
	int32 i32;
	int64 i64;
	double f;
	const char *str;
	const char *input = "/* Test File */\n"
		"Setting_Int: 1;\n"
		"Setting_Int64: 1L;\n"
		"Setting_Float: 1.0;\n"
		"Setting_Bool: true;\n"
		"Setting_String: \"1\";\n"
		"Setting_Array: [ ];\n"
		"Setting_Group: { };\n"
		"Setting_List: ( );\n"
		"/* End test file */\n";

	if (libconfig->read_string(&config, input) == CONFIG_FALSE) {
		libconfig->destroy(&config);
		return "Unable to parse configuration.";
	}

	if (libconfig->setting_lookup_int(config.root, "Setting_Int", &i32) == CONFIG_FALSE || i32 != 1) {
		libconfig->destroy(&config);
		return "libconfig->setting_lookup_int failed.";
	}

	if (libconfig->setting_lookup_int64(config.root, "Setting_Int64", &i64) == CONFIG_FALSE || i64 != 1) {
		libconfig->destroy(&config);
		return "libconfig->setting_lookup_int64 failed.";
	}

	if (libconfig->setting_lookup_float(config.root, "Setting_Float", &f) == CONFIG_FALSE || f < 1.0 - 0.1 || f > 1.0 + 0.1) {
		libconfig->destroy(&config);
		return "libconfig->setting_lookup_float failed.";
	}

	if (libconfig->setting_lookup_bool(config.root, "Setting_Bool", &i32) == CONFIG_FALSE || i32 != 1) {
		libconfig->destroy(&config);
		return "libconfig->setting_lookup_bool failed.";
	}

	if (libconfig->setting_lookup_string(config.root, "Setting_String", &str) == CONFIG_FALSE || str == NULL || str[0] != '1' || str[1] != '\0') {
		libconfig->destroy(&config);
		return "libconfig->setting_lookup_string failed.";
	}

	libconfig->destroy(&config);

	return NULL;
}

static const char *test_libconfig_setting_types(void)
{
	struct config_t config;
	struct config_setting_t *t;
	const char *input = "/* Test File */\n"
		"Setting_Int: 1;\n"
		"Setting_Int64: 1L;\n"
		"Setting_Float: 1.0;\n"
		"Setting_Bool: true;\n"
		"Setting_String: \"1\";\n"
		"Setting_Array: [ ];\n"
		"Setting_Group: { };\n"
		"Setting_List: ( );\n"
		"/* End test file */\n";

	if (libconfig->read_string(&config, input) == CONFIG_FALSE) {
		libconfig->destroy(&config);
		return "Unable to parse configuration.";
	}

	if (config_setting_type(config.root) != CONFIG_TYPE_GROUP) {
		libconfig->destroy(&config);
		return "CONFIG_TYPE_GROUP failed.";
	}

	if ((t = libconfig->lookup(&config, "Setting_Int")) == NULL || config_setting_type(t) != CONFIG_TYPE_INT
			|| config_setting_is_group(t) || config_setting_is_array(t) || config_setting_is_list(t)
			|| config_setting_is_aggregate(t) || !config_setting_is_scalar(t) || !config_setting_is_number(t)
	) {
		libconfig->destroy(&config);
		return "CONFIG_TYPE_INT failed.";
	}

	if ((t = libconfig->lookup(&config, "Setting_Int64")) == NULL || config_setting_type(t) != CONFIG_TYPE_INT64
			|| config_setting_is_group(t) || config_setting_is_array(t) || config_setting_is_list(t)
			|| config_setting_is_aggregate(t) || !config_setting_is_scalar(t) || !config_setting_is_number(t)
	) {
		libconfig->destroy(&config);
		return "CONFIG_TYPE_INT64 failed.";
	}

	if ((t = libconfig->lookup(&config, "Setting_Float")) == NULL || config_setting_type(t) != CONFIG_TYPE_FLOAT
			|| config_setting_is_group(t) || config_setting_is_array(t) || config_setting_is_list(t)
			|| config_setting_is_aggregate(t) || !config_setting_is_scalar(t) || !config_setting_is_number(t)
	) {
		libconfig->destroy(&config);
		return "CONFIG_TYPE_FLOAT failed.";
	}

	if ((t = libconfig->lookup(&config, "Setting_Bool")) == NULL || config_setting_type(t) != CONFIG_TYPE_BOOL
			|| config_setting_is_group(t) || config_setting_is_array(t) || config_setting_is_list(t)
			|| config_setting_is_aggregate(t) || !config_setting_is_scalar(t) || config_setting_is_number(t)
	) {
		libconfig->destroy(&config);
		return "CONFIG_TYPE_BOOL failed.";
	}

	if ((t = libconfig->lookup(&config, "Setting_String")) == NULL || config_setting_type(t) != CONFIG_TYPE_STRING
			|| config_setting_is_group(t) || config_setting_is_array(t) || config_setting_is_list(t)
			|| config_setting_is_aggregate(t) || !config_setting_is_scalar(t) || config_setting_is_number(t)
	) {
		libconfig->destroy(&config);
		return "CONFIG_TYPE_STRING failed.";
	}

	if ((t = libconfig->lookup(&config, "Setting_Array")) == NULL || config_setting_type(t) != CONFIG_TYPE_ARRAY
			|| config_setting_is_group(t) || !config_setting_is_array(t) || config_setting_is_list(t)
			|| !config_setting_is_aggregate(t) || config_setting_is_scalar(t) || config_setting_is_number(t)
	) {
		libconfig->destroy(&config);
		return "CONFIG_TYPE_ARRAY failed.";
	}

	if ((t = libconfig->lookup(&config, "Setting_Group")) == NULL || config_setting_type(t) != CONFIG_TYPE_GROUP
			|| !config_setting_is_group(t) || config_setting_is_array(t) || config_setting_is_list(t)
			|| !config_setting_is_aggregate(t) || config_setting_is_scalar(t) || config_setting_is_number(t)
	) {
		libconfig->destroy(&config);
		return "CONFIG_TYPE_GROUP failed.";
	}

	if ((t = libconfig->lookup(&config, "Setting_List")) == NULL || config_setting_type(t) != CONFIG_TYPE_LIST
			|| config_setting_is_group(t) || config_setting_is_array(t) || !config_setting_is_list(t)
			|| !config_setting_is_aggregate(t) || config_setting_is_scalar(t) || config_setting_is_number(t)
	) {
		libconfig->destroy(&config);
		return "CONFIG_TYPE_LIST failed.";
	}

	libconfig->destroy(&config);

	return NULL;
}

static const char *test_libconfig_values(void)
{
	struct config_t config;
	int32 i32;
	int64 i64;
	const char *input = "/* Test File */\n"
		"Setting_Int1: 1;\n"
		"Setting_IntHex: 0x10;\n"
		"Setting_IntNegative: -1;\n"
		"Setting_Int64: 1L;\n"
		"Setting_Int64Hex: 0x10L;\n"
		"Setting_Int64Negative: -1L;\n"
		"Setting_IntSignedMax: 0x7fffffff;\n"
		"/* End test file */\n";

	if (libconfig->read_string(&config, input) == CONFIG_FALSE) {
		libconfig->destroy(&config);
		return "Unable to parse configuration.";
	}

	if (libconfig->setting_lookup_int(config.root, "Setting_Int1", &i32) == CONFIG_FALSE || i32 != 1) {
		libconfig->destroy(&config);
		return "(int) 1 failed.";
	}

	if (libconfig->setting_lookup_int(config.root, "Setting_IntHex", &i32) == CONFIG_FALSE || i32 != 0x10) {
		libconfig->destroy(&config);
		return "(int) 0x10 failed.";
	}

	if (libconfig->setting_lookup_int(config.root, "Setting_IntNegative", &i32) == CONFIG_FALSE || i32 != -1) {
		libconfig->destroy(&config);
		return "(int) -1 failed.";
	}

	if (libconfig->setting_lookup_int64(config.root, "Setting_Int64", &i64) == CONFIG_FALSE || i64 != 1) {
		libconfig->destroy(&config);
		return "(int64) 1 failed.";
	}

	if (libconfig->setting_lookup_int64(config.root, "Setting_Int64Hex", &i64) == CONFIG_FALSE || i64 != 0x10) {
		libconfig->destroy(&config);
		return "(int64) 0x10 failed.";
	}

	if (libconfig->setting_lookup_int64(config.root, "Setting_Int64Negative", &i64) == CONFIG_FALSE || i64 != -1) {
		libconfig->destroy(&config);
		return "(int64) -1 failed.";
	}

	if (libconfig->setting_lookup_int(config.root, "Setting_IntSignedMax", &i32) == CONFIG_FALSE || i32 != INT32_MAX) {
		libconfig->destroy(&config);
		return "(int) INT32_MAX failed.";
	}

	libconfig->destroy(&config);

	return NULL;
}

static const char *test_libconfig_path_lookup(void)
{
	struct config_t config;
	int32 i32;
	const char *input = "/* Test File */\n"
		"Setting_Array: [1, 2, 3];\n"
		"Setting_Group: {\n"
		"    Group_Nested1: 4;\n"
		"    Group_Nested2: 5;\n"
		"    Group_Nested3: 6;\n"
		"    Group_Nested4: 7;\n"
		"};\n"
		"Setting_List: (\n"
		"    (\"List_Nested1\", 8),\n"
		"    (\"List_Nested2\", 9),\n"
		"    10,\n"
		");\n"
		"/* End test file */\n";
	if (libconfig->read_string(&config, input) == CONFIG_FALSE) {
		libconfig->destroy(&config);
		return "Unable to parse configuration.";
	}

	if (libconfig->lookup_int(&config, "Setting_Array/[0]", &i32) == CONFIG_FALSE || i32 != 1) {
		libconfig->destroy(&config);
		return "Setting_Array/[0] failed.";
	}

	if (libconfig->lookup_int(&config, "Setting_Array:[0]", &i32) == CONFIG_FALSE || i32 != 1) {
		libconfig->destroy(&config);
		return "Setting_Array:[0] failed.";
	}

	if (libconfig->lookup_int(&config, "Setting_Array/[1]", &i32) == CONFIG_FALSE || i32 != 2) {
		ShowDebug("%d\n", i32);
		libconfig->destroy(&config);
		return "Setting_Array/[1] failed.";
	}

	if (libconfig->lookup_int(&config, "Setting_Array/[2]", &i32) == CONFIG_FALSE || i32 != 3) {
		libconfig->destroy(&config);
		return "Setting_Array/[2] failed.";
	}

	if (libconfig->lookup_int(&config, "Setting_Group/Group_Nested1", &i32) == CONFIG_FALSE || i32 != 4) {
		libconfig->destroy(&config);
		return "Setting_Group/Group_Nested1 failed.";
	}

	if (libconfig->lookup_int(&config, "Setting_Group/Group_Nested2", &i32) == CONFIG_FALSE || i32 != 5) {
		libconfig->destroy(&config);
		return "Setting_Group/Group_Nested2 failed.";
	}

	if (libconfig->lookup_int(&config, "Setting_Group/Group_Nested3", &i32) == CONFIG_FALSE || i32 != 6) {
		libconfig->destroy(&config);
		return "Setting_Group/Group_Nested3 failed.";
	}

	if (libconfig->lookup_int(&config, "Setting_Group/Group_Nested4", &i32) == CONFIG_FALSE || i32 != 7) {
		libconfig->destroy(&config);
		return "Setting_Group/Group_Nested4 failed.";
	}

	if (libconfig->lookup_int(&config, "Setting_List/[0]/[1]", &i32) == CONFIG_FALSE || i32 != 8) {
		libconfig->destroy(&config);
		return "Setting_List/[0]/[1] failed.";
	}

	if (libconfig->lookup_int(&config, "Setting_List/[1]/[1]", &i32) == CONFIG_FALSE || i32 != 9) {
		libconfig->destroy(&config);
		return "Setting_List/[1]/[1] failed.";
	}

	if (libconfig->lookup_int(&config, "Setting_List/[2]", &i32) == CONFIG_FALSE || i32 != 10) {
		libconfig->destroy(&config);
		return "Setting_List/[2] failed.";
	}

	libconfig->destroy(&config);
	return NULL;
}

static const char *test_libconfig_setting_names(void)
{
	struct config_t config;
	int32 i32;
	const char *input = "/* Test File */\n"
		"Setting'with'apostrophes: 1;\n"
		"Setting.with.periods: 2;\n"
		"Setting: {\n"
		"    with: {\n"
		"        periods: 3;\n"
		"    };\n"
		"    nested: {\n"
		"        in: {\n"
		"            groups: 4;\n"
		"        };\n"
		"    };\n"
		"};\n"
		"1st_setting_with_numbers: 5;\n"
		"/* End test file */\n";
	if (libconfig->read_string(&config, input) == CONFIG_FALSE) {
		libconfig->destroy(&config);
		return "Unable to parse configuration.";
	}

	if (libconfig->lookup_int(&config, "Setting'with'apostrophes", &i32) == CONFIG_FALSE || i32 != 1) {
		libconfig->destroy(&config);
		return "Setting'with'apostrophes failed.";
	}

	if (libconfig->lookup_int(&config, "Setting.with.periods", &i32) == CONFIG_FALSE || i32 != 2) {
		libconfig->destroy(&config);
		return "Setting.with.periods failed.";
	}

	if (libconfig->lookup_int(&config, "Setting:with:periods", &i32) == CONFIG_FALSE || i32 != 3) {
		libconfig->destroy(&config);
		return "Setting:with:periods failed.";
	}

	if (libconfig->lookup_int(&config, "Setting:nested:in:groups", &i32) == CONFIG_FALSE || i32 != 4) {
		libconfig->destroy(&config);
		return "Setting:nested:in:groups failed.";
	}

	if (libconfig->lookup_int(&config, "Setting/nested/in/groups", &i32) == CONFIG_FALSE || i32 != 4) {
		libconfig->destroy(&config);
		return "Setting/nested/in/groups failed.";
	}

	if (libconfig->lookup_int(&config, "1st_setting_with_numbers", &i32) == CONFIG_FALSE || i32 != 5) {
		libconfig->destroy(&config);
		return "1st_setting_with_numbers failed.";
	}

	libconfig->destroy(&config);
	return NULL;
}

static const char *test_libconfig_duplicate_keys(void)
{
	struct config_t config;
	int32 i32;
	struct config_setting_t *t, *tt;
	int i = 0;
	const char *input = "/* Test File */\n"
		"Setting_Group: {\n"
		"    Duplicate: 1;\n"
		"    Duplicate: 2;\n"
		"    Duplicate: 3;\n"
		"    Duplicate: 4;\n"
		"};\n"
		"/* End test file */\n";
	if (libconfig->read_string(&config, input) == CONFIG_FALSE) {
		libconfig->destroy(&config);
		return "Unable to parse configuration.";
	}

	if (libconfig->lookup_int(&config, "Setting_Group/Duplicate", &i32) == CONFIG_FALSE || i32 != 1) {
		libconfig->destroy(&config);
		return "Setting_Group/Duplicate failed.";
	}

	if ((t = libconfig->lookup(&config, "Setting_Group")) == NULL) {
		libconfig->destroy(&config);
		return "Setting_Group failed.";
	}

	if (libconfig->setting_length(t) != 4) {
		libconfig->destroy(&config);
		return "Wrong amount of duplicates.";
	}

	while ((tt = libconfig->setting_get_elem(t, i++)) != NULL) {
		if (i != libconfig->setting_get_int(tt)) {
			libconfig->destroy(&config);
			return "Duplicate ordering error.";
		}
	}

	if (i != 5) {
		libconfig->destroy(&config);
		return "Wrong amount of duplicates scanned.";
	}


	libconfig->destroy(&config);
	return NULL;
}

static const char *test_libconfig_special_string_syntax(void)
{
	struct config_t config;
	const char *str;
	const char *input = "/* Test File */\n"
		"SpecialString: <\"This is an \"Item_Script\" Special String\n\tWith a line-break inside.\">;\n"
		"/* End test file */\n";
	if (libconfig->read_string(&config, input) == CONFIG_FALSE) {
		libconfig->destroy(&config);
		return "Unable to parse configuration.";
	}

	if (libconfig->lookup_string(&config, "SpecialString", &str) == CONFIG_FALSE || str == NULL) {
		libconfig->destroy(&config);
		return "String lookup failed.";
	}

	if (strcmp("This is an \"Item_Script\" Special String\n\tWith a line-break inside.", str) != 0) {
		libconfig->destroy(&config);
		return "String mismatch.";
	}

	libconfig->destroy(&config);
	return NULL;
}

int do_init(int argc, char **argv)
{
	ShowMessage("===============================================================================\n");
	ShowStatus("Starting tests.\n");

	TEST("CONFIG_TRUE and CONFIG_FALSE", test_libconfig_truefalse);
	TEST("libconfig availability", test_libconfig_defaults);
	TEST("libconfig->init and libconfig->destroy", test_libconfig_init_destroy);
	TEST("libconfig->read_file_src", test_libconfig_read_file_src);
	TEST("libconfig->read", test_libconfig_read);
	TEST("libconfig->load_file", test_libconfig_load_file);
	(void)test_libconfig_write; //TEST("libconfig->write", test_libconfig_write);
	(void)test_libconfig_write_file; //TEST("libconfig->write_file", test_libconfig_write_file);
	TEST("libconfig->read_string", test_libconfig_read_string);
	TEST("libconfig syntax", test_libconfig_syntax);
	(void)test_libconfig_set_include_dir; //TEST("libconfig->set_include_dir", test_libconfig_set_include_dir);
	//int (*setting_set_format) (struct config_setting_t *setting, short format);
	//short (*setting_get_format) (const struct config_setting_t *setting);
	//struct config_setting_t * (*setting_set_int_elem) (struct config_setting_t *setting, int idx, int value);
	//struct config_setting_t * (*setting_set_int64_elem) (struct config_setting_t *setting, int idx, long long value);
	//struct config_setting_t * (*setting_set_float_elem) (struct config_setting_t *setting, int idx, double value);
	//struct config_setting_t * (*setting_set_bool_elem) (struct config_setting_t *setting, int idx, int value);
	//struct config_setting_t * (*setting_set_string_elem) (struct config_setting_t *setting, int idx, const char *value);
	//struct config_setting_t * (*setting_add) (struct config_setting_t *parent, const char *name, int type);
	//int (*setting_remove) (struct config_setting_t *parent, const char *name);
	//int (*setting_remove_elem) (struct config_setting_t *parent, unsigned int idx);
	//void (*setting_set_hook) (struct config_setting_t *setting, void *hook);
	//void (*set_destructor) (struct config_t *config, void (*destructor)(void *));
	TEST("libconfig->lookup_*", test_libconfig_lookup);
	TEST("libconfig->setting_get_*", test_libconfig_setting_get);
	(void)test_libconfig_set; //TEST("libconfig->setting_set_*", test_libconfig_setting_set);
	TEST("libconfig->setting_lookup_*", test_libconfig_setting_lookup);
	TEST("setting types", test_libconfig_setting_types);
	//void (*setting_copy_simple) (struct config_setting_t *parent, const struct config_setting_t *src);
	//void (*setting_copy_elem) (struct config_setting_t *parent, const struct config_setting_t *src);
	//void (*setting_copy_aggregate) (struct config_setting_t *parent, const struct config_setting_t *src);
	//int (*setting_copy) (struct config_setting_t *parent, const struct config_setting_t *src);
	TEST("values", test_libconfig_values);
	TEST("path lookup", test_libconfig_path_lookup);
	TEST("setting key names", test_libconfig_setting_names);
	TEST("duplicate keys", test_libconfig_duplicate_keys);
	TEST("special string syntax", test_libconfig_special_string_syntax);

	core->runflag = CORE_ST_STOP;
	return EXIT_SUCCESS;
}

int do_final(void) {
	ShowMessage("===============================================================================\n");
	ShowStatus("All tests passed.\n");
	return EXIT_SUCCESS;
}

void do_abort(void) { }

void set_server_type(void)
{
	SERVER_TYPE = SERVER_TYPE_UNKNOWN;
}

void cmdline_args_init_local(void) { }
