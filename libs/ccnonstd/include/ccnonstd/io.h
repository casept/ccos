#pragma once

#include <stdarg.h>

/// The char sink function signature for `vprintf_generic`.
typedef void (*vprintf_sink)(char);

/// A customized and stripped-down variant of `vprintf` that works
/// without any pre-allocated buffers or file I/O.
/// Instead, a function is passed in which consumes 1 character at a
/// time and prints it to VGA, or appends it to a string or whatever else.
/// This way, formatting only has to be implemented once,
/// not for each kind of sink.
///
/// This function is not designed to be called directly,
/// but wrapped in more specific and convenient functions.
///
/// Note that currently, only the %s and %c format specifiers are supported.
///
/// Returns 0 on success, -1 on failure.
int vprintf_generic(const vprintf_sink sink, const char* format, va_list vlist);
