#ifndef __OPERATOR_HPP__
#define __OPERATOR_HPP__

#include <format>
#include <string>
#include <utility>

#include <impl/Export.hpp>

namespace mynum::impl {

enum class Operator : unsigned short {
  NONE = 0,
  ADD = '+' << 3 | 2,
  SUB = '-' << 3 | 3,
  MUL = '*' << 3 | 4,
  DIV = '/' << 3 | 5,
  LAST = '?' << 3 | 6
};

EXPORT_API std::string to_string(Operator p);

}

template <> struct std::formatter<mynum::impl::Operator> : std::formatter<std::string> {
  EXPORT_API static char to_char(const mynum::impl::Operator &op) { return std::to_underlying(op) >> 3; }

  EXPORT_API auto format(const mynum::impl::Operator &op, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "{}", to_char(op));
  }
};

#endif
