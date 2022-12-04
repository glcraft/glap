#pragma once

#ifndef GLAP_MODULE
#include "base.h"
#include <version>
#endif

#ifndef GLAP_USE_FMT
#   ifndef __cpp_lib_format
#       error "standard format requires C++20"
#   endif

#ifndef GLAP_MODULE
#include <format>
#endif

GLAP_EXPORT namespace glap {
    using std::format;
    using std::format_to;
}
#else

#ifndef GLAP_MODULE
#include <fmt/format.h>
#endif

GLAP_EXPORT namespace glap {
    using fmt::format;
    using fmt::format_to;
}
#endif