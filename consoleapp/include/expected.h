#ifdef __cpp_lib_expected
#include <expected>
template <class T, class E>
using expected = std::expected<T, E>;
template <class E>
using unexpected = std::unexpected<E>;
#else
#include <tl/expected.hpp>
template <class T, class E>
using expected = tl::expected<T, E>;
template <class E>
using unexpected = tl::unexpected<E>;
#endif