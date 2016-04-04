/* ----------------------------------------------------------------------------
   libconfig - A library for processing structured configuration files
   Copyright (C) 2013-2016  Hercules Dev Team
   Copyright (C) 2005-2014  Mark A Lindner

   This file is part of libconfig.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library.  If not, see <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------------
*/

#ifndef __libconfig_h
#define __libconfig_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#if defined(LIBCONFIG_STATIC)
#define LIBCONFIG_API
#elif defined(LIBCONFIG_EXPORTS)
#define LIBCONFIG_API __declspec(dllexport)
#else /* ! LIBCONFIG_EXPORTS */
#define LIBCONFIG_API __declspec(dllimport)
#endif /* LIBCONFIG_STATIC */
#else /* ! WIN32 */
#define LIBCONFIG_API
#endif /* WIN32 */

#define LIBCONFIG_VER_MAJOR    1
#define LIBCONFIG_VER_MINOR    5
#define LIBCONFIG_VER_REVISION 0

#include <stdio.h>

#define CONFIG_TYPE_NONE    0
#define CONFIG_TYPE_GROUP   1
#define CONFIG_TYPE_INT     2
#define CONFIG_TYPE_INT64   3
#define CONFIG_TYPE_FLOAT   4
#define CONFIG_TYPE_STRING  5
#define CONFIG_TYPE_BOOL    6
#define CONFIG_TYPE_ARRAY   7
#define CONFIG_TYPE_LIST    8

#define CONFIG_FORMAT_DEFAULT  0
#define CONFIG_FORMAT_HEX      1

#define CONFIG_OPTION_AUTOCONVERT                     0x01
#define CONFIG_OPTION_SEMICOLON_SEPARATORS            0x02
#define CONFIG_OPTION_COLON_ASSIGNMENT_FOR_GROUPS     0x04
#define CONFIG_OPTION_COLON_ASSIGNMENT_FOR_NON_GROUPS 0x08
#define CONFIG_OPTION_OPEN_BRACE_ON_SEPARATE_LINE     0x10

#define CONFIG_TRUE  (1)
#define CONFIG_FALSE (0)

union config_value_t
{
  int ival;
  long long llval;
  double fval;
  char *sval;
  struct config_list_t *list;
};

struct config_setting_t
{
  char *name;
  short type;
  short format;
  union config_value_t value;
  struct config_setting_t *parent;
  struct config_t *config;
  void *hook;
  unsigned int line;
  const char *file;
};

enum config_error_t
{
  CONFIG_ERR_NONE = 0,
  CONFIG_ERR_FILE_IO = 1,
  CONFIG_ERR_PARSE = 2
};

struct config_list_t
{
  unsigned int length;
  struct config_setting_t **elements;
};

struct config_t
{
  struct config_setting_t *root;
  void (*destructor)(void *);
  int options;
  unsigned short tab_width;
  short default_format;
  char *include_dir;
  const char *error_text;
  const char *error_file;
  int error_line;
  enum config_error_t error_type;
  char **filenames;
  unsigned int num_filenames;
};

extern LIBCONFIG_API int config_read(struct config_t *config, FILE *stream);
extern LIBCONFIG_API void config_write(const struct config_t *config, FILE *stream);

extern LIBCONFIG_API void config_set_default_format(struct config_t *config,
                                                    short format);

extern LIBCONFIG_API void config_set_options(struct config_t *config, int options);
extern LIBCONFIG_API int config_get_options(const struct config_t *config);

extern LIBCONFIG_API void config_set_auto_convert(struct config_t *config, int flag);
extern LIBCONFIG_API int config_get_auto_convert(const struct config_t *config);

extern LIBCONFIG_API int config_read_string(struct config_t *config, const char *str);

extern LIBCONFIG_API int config_read_file(struct config_t *config,
                                          const char *filename);
extern LIBCONFIG_API int config_write_file(struct config_t *config,
                                           const char *filename);

extern LIBCONFIG_API void config_set_destructor(struct config_t *config,
                                                void (*destructor)(void *));
extern LIBCONFIG_API void config_set_include_dir(struct config_t *config,
                                                 const char *include_dir);

extern LIBCONFIG_API void config_init(struct config_t *config);
extern LIBCONFIG_API void config_destroy(struct config_t *config);

extern LIBCONFIG_API int config_setting_get_int(
  const struct config_setting_t *setting);
extern LIBCONFIG_API long long config_setting_get_int64(
  const struct config_setting_t *setting);
extern LIBCONFIG_API double config_setting_get_float(
  const struct config_setting_t *setting);
extern LIBCONFIG_API int config_setting_get_bool(
  const struct config_setting_t *setting);
extern LIBCONFIG_API const char *config_setting_get_string(
  const struct config_setting_t *setting);

extern LIBCONFIG_API int config_setting_lookup_int(
  const struct config_setting_t *setting, const char *name, int *value);
extern LIBCONFIG_API int config_setting_lookup_int64(
  const struct config_setting_t *setting, const char *name, long long *value);
extern LIBCONFIG_API int config_setting_lookup_float(
  const struct config_setting_t *setting, const char *name, double *value);
extern LIBCONFIG_API int config_setting_lookup_bool(
  const struct config_setting_t *setting, const char *name, int *value);
extern LIBCONFIG_API int config_setting_lookup_string(
  const struct config_setting_t *setting, const char *name, const char **value);

extern LIBCONFIG_API int config_setting_set_int(struct config_setting_t *setting,
                                                int value);
extern LIBCONFIG_API int config_setting_set_int64(struct config_setting_t *setting,
                                                  long long value);
extern LIBCONFIG_API int config_setting_set_float(struct config_setting_t *setting,
                                                  double value);
extern LIBCONFIG_API int config_setting_set_bool(struct config_setting_t *setting,
                                                 int value);
extern LIBCONFIG_API int config_setting_set_string(struct config_setting_t *setting,
                                                   const char *value);

extern LIBCONFIG_API int config_setting_set_format(struct config_setting_t *setting,
                                                   short format);
extern LIBCONFIG_API short config_setting_get_format(
  const struct config_setting_t *setting);

extern LIBCONFIG_API int config_setting_get_int_elem(
  const struct config_setting_t *setting, int idx);
extern LIBCONFIG_API long long config_setting_get_int64_elem(
  const struct config_setting_t *setting, int idx);
extern LIBCONFIG_API double config_setting_get_float_elem(
  const struct config_setting_t *setting, int idx);
extern LIBCONFIG_API int config_setting_get_bool_elem(
  const struct config_setting_t *setting, int idx);
extern LIBCONFIG_API const char *config_setting_get_string_elem(
  const struct config_setting_t *setting, int idx);

extern LIBCONFIG_API struct config_setting_t *config_setting_set_int_elem(
  struct config_setting_t *setting, int idx, int value);
extern LIBCONFIG_API struct config_setting_t *config_setting_set_int64_elem(
  struct config_setting_t *setting, int idx, long long value);
extern LIBCONFIG_API struct config_setting_t *config_setting_set_float_elem(
  struct config_setting_t *setting, int idx, double value);
extern LIBCONFIG_API struct config_setting_t *config_setting_set_bool_elem(
  struct config_setting_t *setting, int idx, int value);
extern LIBCONFIG_API struct config_setting_t *config_setting_set_string_elem(
  struct config_setting_t *setting, int idx, const char *value);

#define /* const char * */ config_get_include_dir(/* const struct config_t * */ C) \
  ((const char *)(C)->include_dir)

#define /* int */ config_setting_type(/* const struct config_setting_t * */ S) \
  ((S)->type)

#define /* int */ config_setting_is_group(/* const struct config_setting_t * */ S) \
  ((S)->type == CONFIG_TYPE_GROUP)
#define /* int */ config_setting_is_array(/* const struct config_setting_t * */ S) \
  ((S)->type == CONFIG_TYPE_ARRAY)
#define /* int */ config_setting_is_list(/* const struct config_setting_t * */ S) \
  ((S)->type == CONFIG_TYPE_LIST)

#define /* int */ config_setting_is_aggregate( \
  /* const struct config_setting_t * */ S)                                     \
  (((S)->type == CONFIG_TYPE_GROUP) || ((S)->type == CONFIG_TYPE_LIST)  \
   || ((S)->type == CONFIG_TYPE_ARRAY))

#define /* int */ config_setting_is_number(/* const struct config_setting_t * */ S) \
  (((S)->type == CONFIG_TYPE_INT)                                       \
   || ((S)->type == CONFIG_TYPE_INT64)                                  \
   || ((S)->type == CONFIG_TYPE_FLOAT))

#define /* int */ config_setting_is_scalar(/* const struct config_setting_t * */ S) \
  (((S)->type == CONFIG_TYPE_BOOL) || ((S)->type == CONFIG_TYPE_STRING) \
   || config_setting_is_number(S))

#define /* const char * */ config_setting_name( \
  /* const struct config_setting_t * */ S)             \
  ((S)->name)

#define /* struct config_setting_t * */ config_setting_parent( \
  /* const struct config_setting_t * */ S)                     \
  ((S)->parent)

#define /* int */ config_setting_is_root(       \
  /* const struct config_setting_t * */ S)             \
  ((S)->parent ? CONFIG_FALSE : CONFIG_TRUE)

extern LIBCONFIG_API int config_setting_index(const struct config_setting_t *setting);

extern LIBCONFIG_API int config_setting_length(
  const struct config_setting_t *setting);
extern LIBCONFIG_API struct config_setting_t *config_setting_get_elem(
  const struct config_setting_t *setting, unsigned int idx);

extern LIBCONFIG_API struct config_setting_t *config_setting_get_member(
  const struct config_setting_t *setting, const char *name);

extern LIBCONFIG_API struct config_setting_t *config_setting_add(
  struct config_setting_t *parent, const char *name, int type);
extern LIBCONFIG_API int config_setting_remove(struct config_setting_t *parent,
                                               const char *name);
extern LIBCONFIG_API int config_setting_remove_elem(struct config_setting_t *parent,
                                                    unsigned int idx);
extern LIBCONFIG_API void config_setting_set_hook(struct config_setting_t *setting,
                                                  void *hook);

#define config_setting_get_hook(S) ((S)->hook)

extern LIBCONFIG_API struct config_setting_t *config_lookup(const struct config_t *config,
                                                     const char *path);
extern LIBCONFIG_API struct config_setting_t *config_setting_lookup(
  struct config_setting_t *setting, const char *path);

extern LIBCONFIG_API int config_lookup_int(const struct config_t *config,
                                           const char *path, int *value);
extern LIBCONFIG_API int config_lookup_int64(const struct config_t *config,
                                             const char *path,
                                             long long *value);
extern LIBCONFIG_API int config_lookup_float(const struct config_t *config,
                                             const char *path, double *value);
extern LIBCONFIG_API int config_lookup_bool(const struct config_t *config,
                                            const char *path, int *value);
extern LIBCONFIG_API int config_lookup_string(const struct config_t *config,
                                              const char *path,
                                              const char **value);

#define /* struct config_setting_t * */ config_root_setting( \
  /* const struct config_t * */ C)                           \
  ((C)->root)

#define  /* void */ config_set_default_format(/* struct config_t * */ C,       \
                                              /* short */ F)            \
  (C)->default_format = (F)

#define /* short */ config_get_default_format(/* struct config_t * */ C)       \
  ((C)->default_format)

#define /* void */ config_set_tab_width(/* struct config_t * */ C,     \
                                        /* unsigned short */ W) \
  (C)->tab_width = ((W) & 0x0F)

#define /* unsigned char */ config_get_tab_width(/* const struct config_t * */ C) \
  ((C)->tab_width)

#define /* unsigned short */ config_setting_source_line(   \
  /* const struct config_setting_t * */ S)                        \
  ((S)->line)

#define /* const char */ config_setting_source_file(    \
  /* const struct config_setting_t * */ S)                     \
  ((S)->file)

#define /* const char * */ config_error_text(/* const struct config_t * */ C)  \
  ((C)->error_text)

#define /* const char * */ config_error_file(/* const struct config_t * */ C)  \
  ((C)->error_file)

#define /* int */ config_error_line(/* const struct config_t * */ C)   \
  ((C)->error_line)

#define /* enum config_error_t */ config_error_type(/* const struct config_t * */ C) \
  ((C)->error_type)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __libconfig_h */
