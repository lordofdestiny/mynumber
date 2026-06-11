#include <impl/Operator.hpp>

namespace mynum::impl {

std::string to_string(Operator p) { return std::string(1, static_cast<char>(std::to_underlying(p) >> 3)); }

}
