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
#ifndef handler
#define handler(method, url, func, flags)
#define handler2(method, url, func, flags)
#define packet_handler(func)
#endif  // handler

handler2(HTTP_POST, "/userconfig/load", userconfig_load, REQ_API);
handler2(HTTP_POST, "/userconfig/save", userconfig_save, REQ_API_AUTH | REQ_DATA | REQ_AUTO_CLOSE);
handler2(HTTP_POST, "/charconfig/load", charconfig_load, REQ_API_AUTH | REQ_CHAR_ID);
handler2(HTTP_POST, "/emblem/upload", emblem_upload, REQ_EMBLEM_UPLOAD);
handler2(HTTP_POST, "/emblem/download", emblem_download, REQ_API_AUTH | REQ_GUILD_ID | REQ_VERSION);
handler(HTTP_GET, "/test/url", test_url, REQ_DEFAULT);
//packet_handler(userconfig_save_emotes);
