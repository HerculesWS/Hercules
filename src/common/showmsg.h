// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef COMMON_SHOWMSG_H
#define COMMON_SHOWMSG_H

#include "common/cbasetypes.h"

#ifdef HERCULES_CORE
#	include <libconfig/libconfig.h>
#else
#	include "common/HPMi.h"
#endif

#include <stdarg.h>

// for help with the console colors look here:
// http://www.edoceo.com/liberum/?doc=printf-with-color
// some code explanation (used here):
// \033[2J : clear screen and go up/left (0, 0 position)
// \033[K  : clear line from actual position to end of the line
// \033[0m : reset color parameter
// \033[1m : use bold for font

#define CL_RESET      "\033[0m"
#define CL_CLS        "\033[2J"
#define CL_CLL        "\033[K"

// font settings
#define CL_BOLD       "\033[1m"
#define CL_NORM       CL_RESET
#define CL_NORMAL     CL_RESET
#define CL_NONE       CL_RESET

// background color
#define CL_BG_BLACK   "\033[40m"
#define CL_BG_RED     "\033[41m"
#define CL_BG_GREEN   "\033[42m"
#define CL_BG_YELLOW  "\033[43m"
#define CL_BG_BLUE    "\033[44m"
#define CL_BG_MAGENTA "\033[45m"
#define CL_BG_CYAN    "\033[46m"
#define CL_BG_WHITE   "\033[47m"
// foreground color and normal font (normal color on windows)
#define CL_LT_BLACK   "\033[0;30m"
#define CL_LT_RED     "\033[0;31m"
#define CL_LT_GREEN   "\033[0;32m"
#define CL_LT_YELLOW  "\033[0;33m"
#define CL_LT_BLUE    "\033[0;34m"
#define CL_LT_MAGENTA "\033[0;35m"
#define CL_LT_CYAN    "\033[0;36m"
#define CL_LT_WHITE   "\033[0;37m"
// foreground color and bold font (bright color on windows)
#define CL_BT_BLACK   "\033[1;30m"
#define CL_BT_RED     "\033[1;31m"
#define CL_BT_GREEN   "\033[1;32m"
#define CL_BT_YELLOW  "\033[1;33m"
#define CL_BT_BLUE    "\033[1;34m"
#define CL_BT_MAGENTA "\033[1;35m"
#define CL_BT_CYAN    "\033[1;36m"
#define CL_BT_WHITE   "\033[1;37m"

// foreground color and bold font (bright color on windows)
#define CL_WHITE   CL_BT_WHITE
#define CL_GRAY    CL_BT_BLACK
#define CL_RED     CL_BT_RED
#define CL_GREEN   CL_BT_GREEN
#define CL_YELLOW  CL_BT_YELLOW
#define CL_BLUE    CL_BT_BLUE
#define CL_MAGENTA CL_BT_MAGENTA
#define CL_CYAN    CL_BT_CYAN

#define CL_SPACE   "           "   // space aquivalent of the print messages

enum msg_type {
	MSG_NONE,
	MSG_STATUS,
	MSG_SQL,
	MSG_INFORMATION,
	MSG_NOTICE,
	MSG_WARNING,
	MSG_DEBUG,
	MSG_ERROR,
	MSG_FATALERROR
};

#ifdef HERCULES_CORE
extern int stdout_with_ansisequence; //If the color ANSI sequences are to be used. [flaviojs]
extern int msg_silent; //Specifies how silent the console is. [Skotlex]
extern int console_msg_log; //Specifies what error messages to log. [Ind]
extern char timestamp_format[20]; //For displaying Timestamps [Skotlex]

extern void ClearScreen(void);
extern int vShowMessage_(enum msg_type flag, const char *string, va_list ap);

	extern void ShowMessage(const char *, ...) __attribute__((format(printf, 1, 2)));
	extern void ShowStatus(const char *, ...) __attribute__((format(printf, 1, 2)));
	extern void ShowSQL(const char *, ...) __attribute__((format(printf, 1, 2)));
	extern void ShowInfo(const char *, ...) __attribute__((format(printf, 1, 2)));
	extern void ShowNotice(const char *, ...) __attribute__((format(printf, 1, 2)));
	extern void ShowWarning(const char *, ...) __attribute__((format(printf, 1, 2)));
	extern void ShowDebug(const char *, ...) __attribute__((format(printf, 1, 2)));
	extern void ShowError(const char *, ...) __attribute__((format(printf, 1, 2)));
	extern void ShowFatalError(const char *, ...) __attribute__((format(printf, 1, 2)));
	extern void ShowConfigWarning(config_setting_t *config, const char *string, ...) __attribute__((format(printf, 2, 3)));
#else
	HPExport void (*ShowMessage) (const char *, ...) __attribute__((format(printf, 1, 2)));
	HPExport void (*ShowStatus) (const char *, ...) __attribute__((format(printf, 1, 2)));
	HPExport void (*ShowSQL) (const char *, ...) __attribute__((format(printf, 1, 2)));
	HPExport void (*ShowInfo) (const char *, ...) __attribute__((format(printf, 1, 2)));
	HPExport void (*ShowNotice) (const char *, ...) __attribute__((format(printf, 1, 2)));
	HPExport void (*ShowWarning) (const char *, ...) __attribute__((format(printf, 1, 2)));
	HPExport void (*ShowDebug) (const char *, ...) __attribute__((format(printf, 1, 2)));
	HPExport void (*ShowError) (const char *, ...) __attribute__((format(printf, 1, 2)));
	HPExport void (*ShowFatalError) (const char *, ...) __attribute__((format(printf, 1, 2)));
#endif

#endif /* COMMON_SHOWMSG_H */
