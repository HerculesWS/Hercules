/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2026 Hercules Dev Team
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

/*
 * NOTE: This file was auto-generated and should never be manually edited,
 *       as it will get overwritten.
 */

/* GENERATED FILE DO NOT EDIT */

#if !defined(HERCULES_CORE)
#endif // ! HERCULES_CORE

HPExport const char *HPM_shared_symbols(int server_type)
{
#ifdef COMMON_UTILS_H /* HCache */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("HCache", HCache))
		return "HCache";
#endif
#ifdef COMMON_BASE62_H /* base62 */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("base62", base62))
		return "base62";
#endif
#ifdef COMMON_CORE_H /* cmdline */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("cmdline", cmdline))
		return "cmdline";
#endif
#ifdef COMMON_CONSOLE_H /* console */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("console", console))
		return "console";
#endif
#ifdef COMMON_CORE_H /* core */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("core", core))
		return "core";
#endif
#ifdef COMMON_DB_H /* DB */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("DB", DB))
		return "DB";
#endif
#ifdef COMMON_DES_H /* des */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("des", des))
		return "des";
#endif
#ifdef COMMON_EXTRACONF_H /* extraconf */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("extraconf", extraconf))
		return "extraconf";
#endif
#ifdef COMMON_GRFIO_H /* grfio */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("grfio", grfio))
		return "grfio";
#endif
#ifdef COMMON_CONF_H /* libconfig */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("libconfig", libconfig))
		return "libconfig";
#endif
#ifdef COMMON_MAPINDEX_H /* mapindex */
	if ((server_type & (SERVER_TYPE_MAP|SERVER_TYPE_CHAR)) != 0 && !HPM_SYMBOL("mapindex", mapindex))
		return "mapindex";
#endif
#ifdef COMMON_MD5CALC_H /* md5 */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("md5", md5))
		return "md5";
#endif
#ifdef COMMON_MUTEX_H /* mutex */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("mutex", mutex))
		return "mutex";
#endif
#ifdef COMMON_NULLPO_H /* nullpo */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("nullpo", nullpo))
		return "nullpo";
#endif
#ifdef COMMON_PACKETS_H /* packets */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("packets", packets))
		return "packets";
#endif
#ifdef COMMON_RANDOM_H /* rnd */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("rnd", rnd))
		return "rnd";
#endif
#ifdef COMMON_SHOWMSG_H /* showmsg */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("showmsg", showmsg))
		return "showmsg";
#endif
#ifdef COMMON_SOCKET_H /* sockt */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("sockt", sockt))
		return "sockt";
#endif
#ifdef COMMON_SQL_H /* SQL */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("SQL", SQL))
		return "SQL";
#endif
#ifdef COMMON_STRLIB_H /* StrBuf */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("StrBuf", StrBuf))
		return "StrBuf";
#endif
#ifdef COMMON_STRLIB_H /* strlib */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("strlib", strlib))
		return "strlib";
#endif
#ifdef COMMON_STRLIB_H /* sv */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("sv", sv))
		return "sv";
#endif
#ifdef COMMON_SYSINFO_H /* sysinfo */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("sysinfo", sysinfo))
		return "sysinfo";
#endif
#ifdef COMMON_THREAD_H /* thread */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("thread", thread))
		return "thread";
#endif
#ifdef COMMON_TIMER_H /* timer */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("timer", timer))
		return "timer";
#endif
	return NULL;
}
