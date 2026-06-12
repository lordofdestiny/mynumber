#include <polyfill/to_underlying.hpp>

#include <impl/Operator.hpp>

namespace mynum::impl {

std::string to_string(Operator p) {
  return std::string(1, static_cast<char>(mynum::polyfill::to_underlying(p) >> 3));
}

} // namespace mynum::impl
