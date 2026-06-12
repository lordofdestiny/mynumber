#include <impl/StateValue.hpp>

namespace mynum::impl {

std::string StateValue::reconstruct() const {
  std::string buffer;
  reconstruct_impl(buffer, Operator::NONE, false);
  return buffer;
}

std::shared_ptr<StateValue> StateValue::combine(const std::shared_ptr<StateValue> &left,
                                                const std::shared_ptr<StateValue> &right, Operator op) {
  auto value = calculate(left->value, right->value, op);
  return std::make_shared<StateValue>(value, op, left, right);
}

int StateValue::calculate(int left, int right, Operator op) {
  using enum Operator;
  switch (op) {
  case ADD:
    return left + right;
  case SUB:
    return left - right;
  case MUL:
    return left * right;
  case DIV:
    return left / right;
  default:
    return -1;
  }
}

bool StateValue::needs_parens(Operator parent_op, bool is_right) const {
  if (parent_op == Operator::NONE) {
    return false;
  }

  auto current_prec = precedence(this->op);
  auto parent_prec = precedence(parent_op);

  return (current_prec < parent_prec) ||
         (current_prec == parent_prec and is_right and (parent_op == Operator::SUB || parent_op == Operator::DIV));
}

void StateValue::reconstruct_impl(std::string &buffer, Operator parent_op, bool is_right) const {
  if (op == Operator::NONE) {
    buffer += std::to_string(value);
    return;
  }

  if (left == nullptr || right == nullptr) {
    throw std::runtime_error("Left or right is nullptr");
  }

  bool parens = needs_parens(parent_op, is_right);

  using std::to_string;
  if (parens)
    buffer += '(';
  left->reconstruct_impl(buffer, op, false);
  buffer += ' ';
  buffer += to_string(op);
  buffer += ' ';
  right->reconstruct_impl(buffer, op, true);
  if (parens)
    buffer += ')';
}

} // namespace mynum::impl
