// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
//
// NOTE: This file was auto-generated and should never be manually edited,
//       as it will get overwritten.

memcpy(&HPMHooks.source.HCache, HCache, sizeof(struct HCache_interface));
memcpy(&HPMHooks.source.cmdline, cmdline, sizeof(struct cmdline_interface));
memcpy(&HPMHooks.source.console, console, sizeof(struct console_interface));
memcpy(&HPMHooks.source.core, core, sizeof(struct core_interface));
memcpy(&HPMHooks.source.DB, DB, sizeof(struct db_interface));
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
