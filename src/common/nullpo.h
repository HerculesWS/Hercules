// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef COMMON_NULLPO_H
#define COMMON_NULLPO_H

#include "../common/cbasetypes.h"

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
#define Assert_chk(EX) ( (EX) ? false : (assert_report(__FILE__, __LINE__, __func__, #EX, "failed assertion"), true) )
#else // ! ASSERT_CHECK
#define Assert(EX) (EX)
#define Assert_chk(EX) ((EX), false)
#endif // ASSERT_CHECK

#if defined(NULLPO_CHECK)
/**
 * Reports NULL pointer information if the passed pointer is NULL
 *
 * @param t pointer to check
 * @return true if the passed pointer is NULL, false otherwise
 */
#define nullpo_chk(t) ( (t) != NULL ? false : (assert_report(__FILE__, __LINE__, __func__, #t, "nullpo info"), true) )
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


void assert_report(const char *file, int line, const char *func, const char *targetname, const char *title);

#endif /* COMMON_NULLPO_H */
