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
#ifndef COMMON_STRLIB_H
#define COMMON_STRLIB_H

#include "common/hercules.h"

#include <stdarg.h>
#include <string.h>

/// Convenience macros

#define remove_control_chars(str)  (strlib->remove_control_chars_(str))
#define trim(str)                  (strlib->trim_(str))
#define normalize_name(str,delims) (strlib->normalize_name_((str),(delims)))
#define stristr(haystack,needle)   (strlib->stristr_((haystack),(needle)))

#if !(defined(WIN32) && defined(_MSC_VER)) && !defined(HAVE_STRNLEN)
	#define strnlen(string,maxlen) (strlib->strnlen_((string),(maxlen)))
#endif

#ifdef WIN32
	#define HAVE_STRTOK_R
	#define strtok_r(s,delim,save_ptr) strlib->strtok_r_((s),(delim),(save_ptr))
#endif

#define e_mail_check(email)          (strlib->e_mail_check_(email))
#define config_switch(str)           (strlib->config_switch_(str))
#define safestrncpy(dst,src,n)       (strlib->safestrncpy_((dst),(src),(n)))
#define safestrnlen(string,maxlen)   (strlib->safestrnlen_((string),(maxlen)))
#define safesnprintf(buf,sz,fmt,...) (strlib->safesnprintf_((buf),(sz),(fmt),##__VA_ARGS__))
#define strline(str,pos)             (strlib->strline_((str),(pos)))
#define bin2hex(output,input,count)  (strlib->bin2hex_((output),(input),(count)))

/// Bitfield determining the behavior of sv_parse and sv_split.
typedef enum e_svopt {
	// default: no escapes and no line terminator
	SV_NOESCAPE_NOTERMINATE = 0,
	// Escapes according to the C compiler.
	SV_ESCAPE_C = 1,
	// Line terminators
	SV_TERMINATE_LF = 2,
	SV_TERMINATE_CRLF = 4,
	SV_TERMINATE_CR = 8,
	// If sv_split keeps the end of line terminator, instead of replacing with '\0'
	SV_KEEP_TERMINATOR = 16
} e_svopt;

/// Other escape sequences supported by the C compiler.
#define SV_ESCAPE_C_SUPPORTED "abtnvfr\?\"'\\"

/// Parse state.
/// The field is [start,end[
struct s_svstate {
	const char* str; //< string to parse
	int len; //< string length
	int off; //< current offset in the string
	int start; //< where the field starts
	int end; //< where the field ends
	enum e_svopt opt; //< parse options
	char delim; //< field delimiter
	bool done; //< if all the text has been parsed
};


/// StringBuf - dynamic string
struct StringBuf {
	char *buf_;
	char *ptr_;
	unsigned int max_;
};
typedef struct StringBuf StringBuf;

struct strlib_interface {
	char *(*jstrescape) (char* pt);
	char *(*jstrescapecpy) (char* pt, const char* spt);
	int (*jmemescapecpy) (char* pt, const char* spt, int size);
	int (*remove_control_chars_) (char* str);
	char *(*trim_) (char* str);
	char *(*normalize_name_) (char* str,const char* delims);
	const char *(*stristr_) (const char *haystack, const char *needle);

	/* only used when '!(defined(WIN32) && defined(_MSC_VER)) && !defined(HAVE_STRNLEN)', needs to be defined at all times however  */
	size_t (*strnlen_) (const char* string, size_t maxlen);

	/* only used when 'WIN32' */
	char * (*strtok_r_) (char *s1, const char *s2, char **lasts);

	int (*e_mail_check_) (char* email);
	int (*config_switch_) (const char* str);

	/// strncpy that always null-terminates the string
	char *(*safestrncpy_) (char* dst, const char* src, size_t n);

	/// doesn't crash on null pointer
	size_t (*safestrnlen_) (const char* string, size_t maxlen);

	/// Works like snprintf, but always null-terminates the buffer.
	/// Returns the size of the string (without null-terminator)
	/// or -1 if the buffer is too small.
	int (*safesnprintf_) (char *buf, size_t sz, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

	/// Returns the line of the target position in the string.
	/// Lines start at 1.
	int (*strline_) (const char* str, size_t pos);

	/// Produces the hexadecimal representation of the given input.
	/// The output buffer must be at least count*2+1 in size.
	/// Returns true on success, false on failure.
	bool (*bin2hex_) (char *output, const unsigned char *input, size_t count);
};

struct stringbuf_interface {
	StringBuf* (*Malloc) (void);
	void (*Init) (StringBuf* self);
	int (*Printf) (StringBuf *self, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
	int (*Vprintf) (StringBuf* self, const char* fmt, va_list args);
	int (*Append) (StringBuf* self, const StringBuf *sbuf);
	int (*AppendStr) (StringBuf* self, const char* str);
	int (*Length) (StringBuf* self);
	char* (*Value) (StringBuf* self);
	void (*Clear) (StringBuf* self);
	void (*Destroy) (StringBuf* self);
	void (*Free) (StringBuf* self);
};

struct sv_interface {
	/// Parses a single field in a delim-separated string.
	/// The delimiter after the field is skipped.
	///
	/// @param svstate Parse state
	/// @return 1 if a field was parsed, 0 if done, -1 on error.
	int (*parse_next) (struct s_svstate* svstate);

	/// Parses a delim-separated string.
	/// Starts parsing at startoff and fills the pos array with position pairs.
	/// out_pos[0] and out_pos[1] are the start and end of line.
	/// Other position pairs are the start and end of fields.
	/// Returns the number of fields found or -1 if an error occurs.
	int (*parse) (const char* str, int len, int startoff, char delim, int* out_pos, int npos, enum e_svopt opt);

	/// Splits a delim-separated string.
	/// WARNING: this function modifies the input string
	/// Starts splitting at startoff and fills the out_fields array.
	/// out_fields[0] is the start of the next line.
	/// Other entries are the start of fields (null-terminated).
	/// Returns the number of fields found or -1 if an error occurs.
	int (*split) (char* str, int len, int startoff, char delim, char** out_fields, int nfields, enum e_svopt opt);

	/// Escapes src to out_dest according to the format of the C compiler.
	/// Returns the length of the escaped string.
	/// out_dest should be len*4+1 in size.
	size_t (*escape_c) (char* out_dest, const char* src, size_t len, const char* escapes);

	/// Unescapes src to out_dest according to the format of the C compiler.
	/// Returns the length of the unescaped string.
	/// out_dest should be len+1 in size and can be the same buffer as src.
	size_t (*unescape_c) (char* out_dest, const char* src, size_t len);

	/// Skips a C escape sequence (starting with '\\').
	const char* (*skip_escaped_c) (const char* p);

	/// Opens and parses a file containing delim-separated columns, feeding them to the specified callback function row by row.
	/// Tracks the progress of the operation (current line number, number of successfully processed rows).
	/// Returns 'true' if it was able to process the specified file, or 'false' if it could not be read.
	bool (*readdb) (const char* directory, const char* filename, char delim, int mincols, int maxcols, int maxrows, bool (*parseproc)(char* fields[], int columns, int current));
};

#ifdef HERCULES_CORE
void strlib_defaults(void);
#endif // HERCULES_CORE

HPShared struct strlib_interface *strlib;
HPShared struct stringbuf_interface *StrBuf;
HPShared struct sv_interface *sv;

#endif /* COMMON_STRLIB_H */
