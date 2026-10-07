/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2015-2026 Hercules Dev Team
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
#ifndef COMMON_HERCULES_H
#define COMMON_HERCULES_H

#include "config/core.h"
#include "common/cbasetypes.h"

/// Helper (not used directly)
#ifdef WIN32
  #define HERCAPI_EXPORT_ __declspec(dllexport)
  #define HERCAPI_IMPORT_ __declspec(dllimport)
#else
  #define HERCAPI_EXPORT_ __attribute__((visibility("default")))
  #define HERCAPI_IMPORT_
#endif

/// For symbols exported by common
#ifdef HERCULES_CORE_COMMON
  #define HERCAPI_COMMON_EXPORT HERCAPI_EXPORT_
  #define HERCAPI_COMMON_EXTERN extern HERCAPI_EXPORT_
#else
  #define HERCAPI_COMMON_EXPORT HERCAPI_IMPORT_
  #define HERCAPI_COMMON_EXTERN extern HERCAPI_IMPORT_
#endif

/// For symbols exported by the api server
#ifdef HERCULES_CORE_API
  #define HERCAPI_API_EXPORT HERCAPI_EXPORT_
  #define HERCAPI_API_EXTERN extern HERCAPI_EXPORT_
#else
  #define HERCAPI_API_EXPORT HERCAPI_IMPORT_
  #define HERCAPI_API_EXTERN extern HERCAPI_IMPORT_
#endif

/// For symbols exported by the char server
#ifdef HERCULES_CORE_CHAR
  #define HERCAPI_CHAR_EXPORT HERCAPI_EXPORT_
  #define HERCAPI_CHAR_EXTERN extern HERCAPI_EXPORT_
#else
  #define HERCAPI_CHAR_EXPORT HERCAPI_IMPORT_
  #define HERCAPI_CHAR_EXTERN extern HERCAPI_IMPORT_
#endif

/// For symbols exported by the login server
#ifdef HERCULES_CORE_LOGIN
  #define HERCAPI_LOGIN_EXPORT HERCAPI_EXPORT_
  #define HERCAPI_LOGIN_EXTERN extern HERCAPI_EXPORT_
#else
  #define HERCAPI_LOGIN_EXPORT HERCAPI_IMPORT_
  #define HERCAPI_LOGIN_EXTERN extern HERCAPI_IMPORT_
#endif

/// For symbols exported by the map server
#ifdef HERCULES_CORE_MAP
  #define HERCAPI_MAP_EXPORT HERCAPI_EXPORT_
  #define HERCAPI_MAP_EXTERN extern HERCAPI_EXPORT_
#else
  #define HERCAPI_MAP_EXPORT HERCAPI_IMPORT_
  #define HERCAPI_MAP_EXTERN extern HERCAPI_IMPORT_
#endif

/// For C-style symbols exported by plugins, to be loaded by the core
#ifdef HERCULES_PLUGIN
  #define HPM_EXPORT extern "C" HERCAPI_EXPORT_
#else
  #define HPM_EXPORT extern "C"
#endif

#ifndef HERCULES_CORE
  #include "common/HPMi.h"
#endif // HERCULES_CORE

#endif // COMMON_HERCULES_H
