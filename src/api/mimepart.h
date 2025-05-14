/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2020-2025 Hercules Dev Team
 * Copyright (C) 2020-2022 Andrei Karas (4144)
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
#ifndef API_MIMEPART_H
#define API_MIMEPART_H

#ifndef MAX_MIME_NAME
	#define MAX_MIME_NAME 20
#endif

#ifndef MAX_MIME_CONTENT_TYPE
	#define MAX_MIME_CONTENT_TYPE 25
#endif

struct MimePart {
	char name[MAX_MIME_NAME];
	char content_type[MAX_MIME_CONTENT_TYPE];
	char *data;
	uint32 data_size;
};

#endif /* API_MIMEPART_H */
