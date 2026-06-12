#ifndef __STATE_VALUE_HPP__
#define __STATE_VALUE_HPP__

#include <memory>

#include <impl/Export.hpp>
#include <impl/Operator.hpp>

namespace mynum::impl {

struct StateValue {
  int value;
  Operator op;
  std::shared_ptr<StateValue> left;
  std::shared_ptr<StateValue> right;

  EXPORT_API std::string reconstruct() const;

  static std::shared_ptr<StateValue> combine(const std::shared_ptr<StateValue> &left,
                                             const std::shared_ptr<StateValue> &right, Operator op);

private:
  static int calculate(int left, int right, Operator op);

  static int precedence(Operator op) { return std::to_underlying(op) & 0x7; }

  bool needs_parens(Operator parent_op, bool is_right) const;

  void reconstruct_impl(std::string &buffer, Operator parent_op, bool is_right) const;
};

} // namespace mynum::impl

#endif
