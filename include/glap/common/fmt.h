#pragma once
#if defined (__cpp_lib_format) && __cpp_lib_format >= 201907L
#include <format>
namespace glap {
    using std::format;
}
#else
#include <fmt/format.h>
namespace glap {
    using fmt::format;
}
#endif