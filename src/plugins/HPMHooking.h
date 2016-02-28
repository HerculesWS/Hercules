/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2016  Hercules Dev Team
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
#ifndef PLUGINS_HPMHOOKING_H
#define PLUGINS_HPMHOOKING_H

#include "common/hercules.h"

enum HPluginHookType {
	HOOK_TYPE_PRE,
	HOOK_TYPE_POST,
};

struct HPMHooking_interface {
	bool (*AddHook) (enum HPluginHookType type, const char *target, void *hook, unsigned int pID);
	void (*HookStop) (const char *func, unsigned int pID);
	bool (*HookStopped) (void);
};

#ifdef HERCULES_CORE
struct HPMHooking_core_interface {
	bool enabled;
	bool force_return;
	bool (*addhook_sub) (enum HPluginHookType type, const char *target, void *hook, unsigned int pID);
	const char *(*Hooked)(bool *fr);
};
#else // ! HERCULES_CORE
HPExport struct HPMHooking_interface HPMHooking_s;

#define addHookPre(tname,hook) (HPMi->hooking->AddHook(HOOK_TYPE_PRE,(tname),(hook),HPMi->pid))
#define addHookPost(tname,hook) (HPMi->hooking->AddHook(HOOK_TYPE_POST,(tname),(hook),HPMi->pid))
/* need better names ;/ */
/* will not run the original function after pre-hook processing is complete (other hooks will run) */
#define hookStop() (HPMi->hooking->HookStop(__func__,HPMi->pid))
#define hookStopped() (HPMi->hooking->HookStopped())

#endif // ! HERCULES_CORE

#endif // PLUGINS_HPMHOOKING_H
