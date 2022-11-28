#pragma once

#ifndef GLAP_MODULE
#include "base.h"
#include <version>

#ifndef GLAP_USE_FMT
#   ifndef __cpp_lib_format
#       error "standard format requires C++20"
#   endif
#include <format>
#else
#include <fmt/format.h>
#endif
#endif

#ifndef GLAP_USE_FMT
GLAP_EXPORT namespace glap {
    using std::format;
    using std::format_to;
    using std::print;
}
#else
GLAP_EXPORT namespace glap {
    using fmt::format;
    using fmt::format_to;
    using fmt::print;
}
#endif