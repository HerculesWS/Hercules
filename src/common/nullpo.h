// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _NULLPO_H_
#define _NULLPO_H_


#include "../common/cbasetypes.h"

#define NLP_MARK __FILE__, __LINE__, __func__

// enabled by default on debug builds
#if defined(DEBUG) && !defined(NULLPO_CHECK)
#define NULLPO_CHECK
#endif

#if defined(NULLPO_CHECK)
/**
 * Reports NULL pointer information if the passed pointer is NULL
 *
 * @param t pointer to check
 * @return true if the passed pointer is NULL, false otherwise
 */
#define nullpo_chk(t) ( (t) != NULL ? false : (nullpo_info(NLP_MARK, #t), true) )
#else // ! NULLPO_CHECK
#define nullpo_chk(t) ((t), false)
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
 * Returns void if a NULL pointer is found.
 *
 * @param t pointer to check
 */
#define nullpo_retv(t) \
	do { if (nullpo_chk(t)) return; } while(0)

/**
 * Returns the given value if a NULL pointer is found.
 *
 * @param ret value to return
 * @param t   pointer to check
 */
#define nullpo_retr(ret, t) \
	do { if (nullpo_chk(t)) return(ret); } while(0)

/**
 * Breaks fromt he current loop/switch if a NULL pointer is found.
 *
 * @param t pointer to check
 */
#define nullpo_retb(t) \
	if (nullpo_chk(t)) break; else (void)0

/**
 * Reports NULL pointer information
 *
 * @param file       Source file where the error was detected
 * @param line       Line
 * @param func       Function
 * @param targetname Name of the checked symbol
 *
 * It is possible to use the NLP_MARK macro that expands to:
 *   __FILE__, __LINE__, __func__
 */
void nullpo_info(const char *file, int line, const char *func, const char *targetname);

#endif /* _NULLPO_H_ */
