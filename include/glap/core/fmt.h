#pragma once
#ifndef GLAP_USE_FMT
#   ifndef __cpp_lib_format
#       error "standard format requires C++20"
#   endif

#include <format>
namespace glap {
    using std::format;
    using std::format_to;
    using std::print;
}
#else
#include <fmt/format.h>
namespace glap {
    using fmt::format;
    using fmt::format_to;
    using fmt::print;
}
#endif