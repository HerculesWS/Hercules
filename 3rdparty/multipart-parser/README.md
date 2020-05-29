Multipart Parser
================

This is a parser written in C for multipart bodies. The code was mostly
inspired by [joyent/http-parser](https://github.com/joyent/http-parser). The
parser does not make any syscalls nor allocations, it does not buffer data
except the boundary that you pass to `mulpartparser_init()` function.

Usage
-----

One `multipartparser` struct must be initialized every time you need to parse
a body. The `multipartparser_callbacks` struct can be initialized only once
and can be used by many parsers at the same time. The code to initialize a
multipart parser should be similar to this:

```C
multipartparser_callbacks callbacks;
multipartparser           parser;

multipartparser_callbacks_init(&callbacks); // It only sets all callbacks to NULL.
callbacks.on_body_begin = &on_body_begin;
callbacks.on_part_begin = &on_part_begin;
callbacks.on_header_field = &on_header_field;
callbacks.on_header_value = &on_header_value;
callbacks.on_headers_complete = &on_headers_complete;
callbacks.on_data = &on_data;
callbacks.on_part_end = &on_part_end;
callbacks.on_body_end = &on_body_end;

multipartparser_init(&parser, boundary);
parser->data = my_data;
```

When there is data to parse call the multipart parser like this:

```C
size_t nparsed;

nparsed = multipartparser_execute(&parser, &callbacks, data, size);
if (nparsed != size) {
    /* parse error */
}
```

Callbacks
---------

See [joyent/http-parser's README](https://github.com/joyent/http-parser/blob/master/README.md#callbacks)
to understand how to use callbacks especially for `on_header_field` and `on_header_value`.
