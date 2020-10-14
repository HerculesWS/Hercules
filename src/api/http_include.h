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
#ifndef API_HTTP_INCLUDE_H
#define API_HTTP_INCLUDE_H

#ifdef USE_HTTP_PARSER
#define HTTP_PARSER struct http_parser
#include <http-parser/http_parser.h>
#else  // USE_HTTP_PARSER
#define http_method llhttp_method
#define HTTP_PARSER llhttp_t
#define http_method_str llhttp_method_name
#define http_errno_name llhttp_errno_name
#define http_parser_settings llhttp_settings_s
#include <llhttp/llhttp.h>
#endif  // USE_HTTP_PARSER

#endif /* API_HTTP_INCLUDE_H */
