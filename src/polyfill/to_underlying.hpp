#ifndef __POLYFILL_TO_UNDERLYING_HPP__
#define __POLYFILL_TO_UNDERLYING_HPP__

#include <type_traits>

#if defined(__cpp_lib_to_underlying) && __cpp_lib_to_underlying >= 202102L

#include <utility>

namespace mynum::polyfill {
using std::to_underlying;
}

#else

namespace mynum::polyfill {

template <class Enum>
constexpr std::underlying_type_t<Enum> to_underlying(Enum value) noexcept {
  return static_cast<std::underlying_type_t<Enum>>(value);
}

} // namespace mynum::polyfill

#endif

#endif
