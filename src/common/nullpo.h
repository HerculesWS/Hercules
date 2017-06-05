/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2016  Hercules Dev Team
 * Copyright (C)  Athena Dev Teams
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
#ifndef COMMON_NULLPO_H
#define COMMON_NULLPO_H

#include "common/hercules.h"

// enabled by default on debug builds
#if defined(DEBUG) && !defined(NULLPO_CHECK)
#define NULLPO_CHECK
#endif

// Skip assert checks on release builds
#if !defined(RELEASE) && !defined(ASSERT_CHECK)
#define ASSERT_CHECK
#endif

/** Assert */

#if defined(ASSERT_CHECK)
// extern "C" {
#include <assert.h>
// }
#if !defined(DEFCPP) && defined(WIN32) && !defined(MINGW)
#include <crtdbg.h>
#endif // !DEFCPP && WIN && !MINGW
#define Assert(EX) assert(EX)
/**
 * Reports an assertion failure if the passed expression is false.
 *
 * @param EX The expression to test.
 * @return false if the passed expression is true, false otherwise.
 */
#define Assert_chk(EX) ( (EX) ? false : (nullpo->assert_report(__FILE__, __LINE__, __func__, #EX, "failed assertion"), true) )
/**
 * Reports an assertion failure (without actually checking it).
 *
 * @param EX the expression to report.
 */
#define Assert_report(EX) (nullpo->assert_report(__FILE__, __LINE__, __func__, #EX, "failed assertion"))
#else // ! ASSERT_CHECK
#define Assert(EX) (EX)
#define Assert_chk(EX) ((EX), false)
#define Assert_report(EX) ((void)(EX))
#endif // ASSERT_CHECK

#if defined(NULLPO_CHECK)
/**
 * Reports NULL pointer information if the passed pointer is NULL
 *
 * @param t pointer to check
 * @return true if the passed pointer is NULL, false otherwise
 */
#define nullpo_chk(t) ( (t) != NULL ? false : (nullpo->assert_report(__FILE__, __LINE__, __func__, #t, "nullpo info"), true) )
#else // ! NULLPO_CHECK
#define nullpo_chk(t) ((void)(t), false)
#endif // NULLPO_CHECK

/**
 * The following macros check for NULL pointers and return from the current
 * function or block in case one is found.
 *
 * It is guaranteed that the argument is evaluated once and only once, so it
 * is safe to call them as:
 * nullpo_ret(x = func());
 * The macros can be used safely in any context, as they expand to a do/while
 * construct, except nullpo_retb, which expands to an if/else construct.
 */

/**
 * Returns 0 if a NULL pointer is found.
 *
 * @param t pointer to check
 */
#define nullpo_ret(t) \
	do { if (nullpo_chk(t)) return(0); } while(0)

/**
 * Returns 0 if the given assertion fails.
 *
 * @param t statement to check
 */
#define Assert_ret(t) \
	do { if (Assert_chk(t)) return(0); } while(0)

/**
 * Returns void if a NULL pointer is found.
 *
 * @param t pointer to check
 */
#define nullpo_retv(t) \
	do { if (nullpo_chk(t)) return; } while(0)

/**
 * Returns void if the given assertion fails.
 *
 * @param t statement to check
 */
#define Assert_retv(t) \
	do { if (Assert_chk(t)) return; } while(0)

/**
 * Returns the given value if a NULL pointer is found.
 *
 * @param ret value to return
 * @param t   pointer to check
 */
#define nullpo_retr(ret, t) \
	do { if (nullpo_chk(t)) return(ret); } while(0)

/**
 * Returns the given value if the given assertion fails.
 *
 * @param ret value to return
 * @param t   statement to check
 */
#define Assert_retr(ret, t) \
	do { if (Assert_chk(t)) return(ret); } while(0)

/**
 * Breaks from the current loop/switch if a NULL pointer is found.
 *
 * @param t pointer to check
 */
#define nullpo_retb(t) \
	if (nullpo_chk(t)) break; else (void)0

/**
 * Breaks from the current loop/switch if the given assertion fails.
 *
 * @param t statement to check
 */
#define Assert_retb(t) \
	if (Assert_chk(t)) break; else (void)0


struct nullpo_interface {
	void (*assert_report) (const char *file, int line, const char *func, const char *targetname, const char *title);
};

#ifdef HERCULES_CORE
void nullpo_defaults(void);
#endif // HERCULES_CORE

HPShared struct nullpo_interface *nullpo;

#endif /* COMMON_NULLPO_H */
