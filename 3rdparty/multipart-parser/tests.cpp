#include "multipartparser.h"

#include <cassert>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <list>
#include <map>
#include <string>

typedef struct part {
    std::map<std::string,std::string> headers;
    std::string body;
} part;

static multipartparser_callbacks g_callbacks;

static bool             g_body_begin_called;
static std::string      g_header_name;
static std::string      g_header_value;
static std::list<part>  g_parts;
static bool             g_body_end_called;

static void init_globals()
{
    g_body_begin_called = false;
    g_header_name.clear();
    g_header_value.clear();
    g_parts.clear();
    g_body_end_called = false;
}

static int on_body_begin(multipartparser* /*parser*/)
{
    g_body_begin_called = true;
    return 0;
}

static int on_part_begin(multipartparser* /*parser*/)
{
    g_parts.push_back(part());
    return 0;
}

static void on_header_done()
{
    g_parts.back().headers[g_header_name] = g_header_value;
    g_header_name.clear();
    g_header_value.clear();
}

static int on_header_field(multipartparser* /*parser*/, const char* data, size_t size)
{
    if (g_header_value.size() > 0)
        on_header_done();
    g_header_name.append(data, size);
    return 0;
}

static int on_header_value(multipartparser* /*parser*/, const char* data, size_t size)
{
    g_header_value.append(data, size);
    return 0;
}

static int on_headers_complete(multipartparser* /*parser*/)
{
    if (g_header_value.size() > 0)
        on_header_done();
    return 0;
}

static int on_data(multipartparser* /*parser*/, const char* data, size_t size)
{
    g_parts.back().body.append(data, size);
    return 0;
}

static int on_part_end(multipartparser* /*parser*/)
{
    return 0;
}

static int on_body_end(multipartparser* /*parser*/)
{
    g_body_end_called = true;
    return 0;
}

void test_simple()
{
    multipartparser parser;

#define BOUNDARY "simple boundary"
#define PART0_BODY                                      \
    "This is implicitly typed plain ASCII text.\r\n"    \
    "It does NOT end with a linebreak."
#define PART1_BODY                                      \
    "This is explicitly typed plain ASCII text.\r\n"    \
    "It DOES end with a linebreak.\r\n"
#define BODY                                            \
    "--" BOUNDARY "\r\n"                                \
    "\r\n"                                              \
    PART0_BODY                                          \
    "\r\n--" BOUNDARY "\r\n"                            \
    "Content-type: text/plain; charset=us-ascii\r\n"    \
    "\r\n"                                              \
    PART1_BODY                                          \
    "\r\n--" BOUNDARY "--\r\n"                          \

    init_globals();
    multipartparser_init(&parser, BOUNDARY);

    assert(multipartparser_execute(&parser, &g_callbacks, BODY, strlen(BODY)) == strlen(BODY));
    assert(g_body_begin_called);
    assert(g_parts.size() == 2);

    part p0 = g_parts.front(); g_parts.pop_front();
    assert(p0.headers.empty());
    assert(p0.body.compare(PART0_BODY) == 0);

    part p1 = g_parts.front(); g_parts.pop_front();
    assert(p1.headers.size() == 1);
    assert(p1.body == PART1_BODY);

    assert(g_body_end_called);

#undef BOUNDARY
#undef BODY
}

void test_simple_with_preamble()
{
    multipartparser parser;

#define BOUNDARY "simple boundary"
#define BODY                                                    \
    "This is the preamble.  It is to be ignored, though it\r\n" \
    "is a handy place for mail composers to include an\r\n"     \
    "explanatory note to non-MIME compliant readers.\r\n"       \
    "--" BOUNDARY "\r\n"                                        \
    "\r\n"                                                      \
    "This is implicitly typed plain ASCII text.\r\n"            \
    "It does NOT end with a linebreak."                         \
    "\r\n--" BOUNDARY "\r\n"                                    \
    "Content-type: text/plain; charset=us-ascii\r\n"            \
    "\r\n"                                                      \
    "This is explicitly typed plain ASCII text.\r\n"            \
    "It DOES end with a linebreak.\r\n"                         \
    "\r\n--" BOUNDARY "--\r\n"                                  \

    init_globals();
    multipartparser_init(&parser, BOUNDARY);

    assert(multipartparser_execute(&parser, &g_callbacks, BODY, strlen(BODY)) == strlen(BODY));
    assert(g_body_begin_called);
    assert(g_parts.size() == 2);
    assert(g_body_end_called);

#undef BOUNDARY
#undef BODY
}

void test_simple_with_epilogue()
{
    multipartparser parser;

#define BOUNDARY "simple boundary"
#define BODY                                                    \
    "--" BOUNDARY "\r\n"                                        \
    "\r\n"                                                      \
    "This is implicitly typed plain ASCII text.\r\n"            \
    "It does NOT end with a linebreak."                         \
    "\r\n--" BOUNDARY "\r\n"                                    \
    "Content-type: text/plain; charset=us-ascii\r\n"            \
    "\r\n"                                                      \
    "This is explicitly typed plain ASCII text.\r\n"            \
    "It DOES end with a linebreak.\r\n"                         \
    "\r\n--" BOUNDARY "--\r\n"                                  \
    "This is the epilogue.  It is to be ignored.\r\n"           \

    init_globals();
    multipartparser_init(&parser, BOUNDARY);

    assert(multipartparser_execute(&parser, &g_callbacks, BODY, strlen(BODY)) == strlen(BODY));
    assert(g_body_begin_called);
    assert(g_parts.size() == 2);
    assert(g_body_end_called);

#undef BOUNDARY
#undef BODY
}

void test_headers()
{
    multipartparser parser;

#define BOUNDARY "boundary"
#define BODY                                                                \
    "--" BOUNDARY "\r\n"                                                    \
    "Content-Disposition: form-data; name=\"foo\"; filename=\"bar\"\r\n"    \
    "Content-Type: application/octet-stream\r\n"                            \
    "\r\n"                                                                  \
    "That's the file content!\r\n"                                          \
    "\r\n--" BOUNDARY "--\r\n"                                              \

    init_globals();
    multipartparser_init(&parser, BOUNDARY);

    assert(multipartparser_execute(&parser, &g_callbacks, BODY, strlen(BODY)) == strlen(BODY));
    assert(g_body_begin_called);
    assert(g_parts.size() == 1);
    assert(g_parts.front().headers.size() == 2);
    assert(g_parts.front().headers.find("Content-Disposition")->second == "form-data; name=\"foo\"; filename=\"bar\"");
    assert(g_parts.front().headers.find("Content-Type")->second == "application/octet-stream");
    assert(g_body_end_called);

#undef BOUNDARY
#undef BODY
}

void test_with_empty_headers()
{
    multipartparser parser;

#define BOUNDARY "boundary"
#define BODY                                                    \
    "--" BOUNDARY "\r\n"                                        \
    "\r\n"                                                      \
    "This is implicitly typed plain ASCII text.\r\n"            \
    "It does NOT end with a linebreak."                         \
    "\r\n--" BOUNDARY "--\r\n"

    init_globals();
    multipartparser_init(&parser, BOUNDARY);

    assert(multipartparser_execute(&parser, &g_callbacks, BODY, strlen(BODY)) == strlen(BODY));
    assert(g_body_begin_called);
    assert(g_parts.size() == 1);
    assert(g_parts.front().headers.empty());
    assert(g_body_end_called);

#undef BOUNDARY
#undef BODY
}

void test_with_header_field_parsed_in_two_times()
{
    multipartparser parser;

#define BOUNDARY "boundary"
#define BODY0                                           \
    "--" BOUNDARY "\r\n"
#define BODY1                                           \
    "Content-Disposition: form-data; name=\"foo\"\r\n"  \
    "\r\n"                                              \
    "bar"                                               \
    "\r\n--" BOUNDARY "--\r\n"                          \

    init_globals();
    multipartparser_init(&parser, BOUNDARY);

    assert(multipartparser_execute(&parser, &g_callbacks, BODY0, strlen(BODY0)) == strlen(BODY0));
    assert(multipartparser_execute(&parser, &g_callbacks, BODY1, strlen(BODY1)) == strlen(BODY1));
    assert(g_body_begin_called);
    assert(g_parts.size() == 1);
    assert(g_parts.front().headers.size() == 1);
    assert(g_parts.front().headers.find("Content-Disposition")->second == "form-data; name=\"foo\"");
    assert(g_body_end_called);

#undef BOUNDARY
#undef BODY0
#undef BODY1
}

void test_with_header_field_parsed_in_two_times_1()
{
    multipartparser parser;

#define BOUNDARY "boundary"
#define BODY0                                           \
    "--" BOUNDARY "\r\n"                                \
    "Content-Di"
#define BODY1                                           \
    "sposition: form-data; name=\"foo\"\r\n"            \
    "\r\n"                                              \
    "bar"                                               \
    "\r\n--" BOUNDARY "--\r\n"                          \

    init_globals();
    multipartparser_init(&parser, BOUNDARY);

    assert(multipartparser_execute(&parser, &g_callbacks, BODY0, strlen(BODY0)) == strlen(BODY0));
    assert(multipartparser_execute(&parser, &g_callbacks, BODY1, strlen(BODY1)) == strlen(BODY1));
    assert(g_body_begin_called);
    assert(g_parts.size() == 1);
    assert(g_parts.front().headers.size() == 1);
    assert(g_parts.front().headers.find("Content-Disposition")->second == "form-data; name=\"foo\"");
    assert(g_body_end_called);

#undef BOUNDARY
#undef BODY0
#undef BODY1
}

void test_with_header_field_parsed_in_two_times_2()
{
    multipartparser parser;

#define BOUNDARY "boundary"
#define BODY0                                           \
    "--" BOUNDARY "\r\n"                                \
    "Content-Disposition"
#define BODY1                                           \
    ": form-data; name=\"foo\"\r\n"                     \
    "\r\n"                                              \
    "bar"                                               \
    "\r\n--" BOUNDARY "--\r\n"                          \

    init_globals();
    multipartparser_init(&parser, BOUNDARY);

    assert(multipartparser_execute(&parser, &g_callbacks, BODY0, strlen(BODY0)) == strlen(BODY0));
    assert(multipartparser_execute(&parser, &g_callbacks, BODY1, strlen(BODY1)) == strlen(BODY1));
    assert(g_body_begin_called);
    assert(g_parts.size() == 1);
    assert(g_parts.front().headers.size() == 1);
    assert(g_parts.front().headers.find("Content-Disposition")->second == "form-data; name=\"foo\"");
    assert(g_body_end_called);

#undef BOUNDARY
#undef BODY0
#undef BODY1
}

void test_with_header_value_parsed_in_two_times()
{
    multipartparser parser;

#define BOUNDARY "boundary"
#define BODY0                                           \
    "--" BOUNDARY "\r\n"                                \
    "Content-Disposition: "
#define BODY1                                           \
    "form-data; name=\"foo\"\r\n"                       \
    "\r\n"                                              \
    "bar"                                               \
    "\r\n--" BOUNDARY "--\r\n"                          \

    init_globals();
    multipartparser_init(&parser, BOUNDARY);

    assert(multipartparser_execute(&parser, &g_callbacks, BODY0, strlen(BODY0)) == strlen(BODY0));
    assert(multipartparser_execute(&parser, &g_callbacks, BODY1, strlen(BODY1)) == strlen(BODY1));
    assert(g_body_begin_called);
    assert(g_parts.size() == 1);
    assert(g_parts.front().headers.size() == 1);
    assert(g_parts.front().headers.find("Content-Disposition")->second == "form-data; name=\"foo\"");
    assert(g_body_end_called);

#undef BOUNDARY
#undef BODY0
#undef BODY1
}

void test_with_header_value_parsed_in_two_times_1()
{
    multipartparser parser;

#define BOUNDARY "boundary"
#define BODY0                                           \
    "--" BOUNDARY "\r\n"                                \
    "Content-Disposition: form-data"
#define BODY1                                           \
    "; name=\"foo\"\r\n"                                \
    "\r\n"                                              \
    "bar"                                               \
    "\r\n--" BOUNDARY "--\r\n"                          \

    init_globals();
    multipartparser_init(&parser, BOUNDARY);

    assert(multipartparser_execute(&parser, &g_callbacks, BODY0, strlen(BODY0)) == strlen(BODY0));
    assert(multipartparser_execute(&parser, &g_callbacks, BODY1, strlen(BODY1)) == strlen(BODY1));
    assert(g_body_begin_called);
    assert(g_parts.size() == 1);
    assert(g_parts.front().headers.size() == 1);
    assert(g_parts.front().headers.find("Content-Disposition")->second == "form-data; name=\"foo\"");
    assert(g_body_end_called);

#undef BOUNDARY
#undef BODY0
#undef BODY1
}

void test_with_header_value_parsed_in_two_times_2()
{
    multipartparser parser;

#define BOUNDARY "boundary"
#define BODY0                                           \
    "--" BOUNDARY "\r\n"                                \
    "Content-Disposition: "
#define BODY1                                           \
    "form-data; name=\"foo\"\r\n"                       \
    "\r\n"                                              \
    "bar"                                               \
    "\r\n--" BOUNDARY "--\r\n"                          \

    init_globals();
    multipartparser_init(&parser, BOUNDARY);

    assert(multipartparser_execute(&parser, &g_callbacks, BODY0, strlen(BODY0)) == strlen(BODY0));
    assert(multipartparser_execute(&parser, &g_callbacks, BODY1, strlen(BODY1)) == strlen(BODY1));
    assert(g_body_begin_called);
    assert(g_parts.size() == 1);
    assert(g_parts.front().headers.size() == 1);
    assert(g_parts.front().headers.find("Content-Disposition")->second == "form-data; name=\"foo\"");
    assert(g_body_end_called);

#undef BOUNDARY
#undef BODY0
#undef BODY1
}

int main()
{
    multipartparser_callbacks_init(&g_callbacks);
    g_callbacks.on_body_begin = &on_body_begin;
    g_callbacks.on_part_begin = &on_part_begin;
    g_callbacks.on_header_field = &on_header_field;
    g_callbacks.on_header_value = &on_header_value;
    g_callbacks.on_headers_complete = &on_headers_complete;
    g_callbacks.on_data = &on_data;
    g_callbacks.on_part_end = &on_part_end;
    g_callbacks.on_body_end = &on_body_end;

    test_simple();
    test_simple_with_preamble();
    test_simple_with_epilogue();

    test_headers();
    test_with_empty_headers();
    test_with_header_field_parsed_in_two_times();
    test_with_header_field_parsed_in_two_times_1();
    test_with_header_field_parsed_in_two_times_2();
    test_with_header_value_parsed_in_two_times();
    test_with_header_value_parsed_in_two_times_1();
    test_with_header_value_parsed_in_two_times_2();

    return 0;
}
