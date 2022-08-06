#ifdef __cpp_lib_expected
#include <expected>
template <class T, class E>
using expected = std::expected<T, E>;
#else
#include <tl/expected.hpp>
template <class T, class E>
using expected = tl::expected<T, E>;
#endif