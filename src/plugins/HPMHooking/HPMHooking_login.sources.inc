/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2016  Hercules Dev Team
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

memcpy(&HPMHooks.source.HCache, HCache, sizeof(struct HCache_interface));
memcpy(&HPMHooks.source.cmdline, cmdline, sizeof(struct cmdline_interface));
memcpy(&HPMHooks.source.console, console, sizeof(struct console_interface));
memcpy(&HPMHooks.source.core, core, sizeof(struct core_interface));
memcpy(&HPMHooks.source.DB, DB, sizeof(struct db_interface));
memcpy(&HPMHooks.source.lclif, lclif, sizeof(struct lclif_interface));
memcpy(&HPMHooks.source.PRIV__lclif, lclif->p, sizeof(struct lclif_interface_private));
memcpy(&HPMHooks.source.libconfig, libconfig, sizeof(struct libconfig_interface));
memcpy(&HPMHooks.source.login, login, sizeof(struct login_interface));
memcpy(&HPMHooks.source.iMalloc, iMalloc, sizeof(struct malloc_interface));
memcpy(&HPMHooks.source.nullpo, nullpo, sizeof(struct nullpo_interface));
memcpy(&HPMHooks.source.showmsg, showmsg, sizeof(struct showmsg_interface));
memcpy(&HPMHooks.source.sockt, sockt, sizeof(struct socket_interface));
memcpy(&HPMHooks.source.SQL, SQL, sizeof(struct sql_interface));
memcpy(&HPMHooks.source.StrBuf, StrBuf, sizeof(struct stringbuf_interface));
memcpy(&HPMHooks.source.strlib, strlib, sizeof(struct strlib_interface));
memcpy(&HPMHooks.source.sv, sv, sizeof(struct sv_interface));
memcpy(&HPMHooks.source.sysinfo, sysinfo, sizeof(struct sysinfo_interface));
memcpy(&HPMHooks.source.timer, timer, sizeof(struct timer_interface));
