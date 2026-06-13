#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <array>
#include <memory>
#include <string>

#include <impl/Combination.hpp>

using emscripten::val;

namespace {

void readNumbers(const val &numbers, std::array<int, 6> &out) {
  if (!numbers.isArray()) {
    return;
  }
  const auto length = numbers["length"].as<unsigned>();
  for (unsigned i = 0; i < 6 && i < length; ++i) {
    out[i] = numbers[i].as<int>();
  }
}

val writeNumbers(const std::array<int, 6> &numbers) {
  val arr = val::array();
  for (int n : numbers) {
    arr.call<void>("push", n);
  }
  return arr;
}

std::string arrayToString(const std::array<int, 6> &nums) {
  std::string buffer = "[";
  for (size_t i = 0; i < 6; ++i) {
    buffer += std::to_string(nums[i]);
    if (i != 5) {
      buffer += ", ";
    }
  }
  buffer += "]";
  return buffer;
}

class WasmSolution {
  std::shared_ptr<mynum::impl::StateValue> state_;

public:
  WasmSolution() : state_(nullptr) {}

  explicit WasmSolution(std::shared_ptr<mynum::impl::StateValue> state) : state_(std::move(state)) {}

  int getValue() const { return state_->value; }

  std::string expression() const { return state_->reconstruct(); }

  std::string toString() const {
    return "Solution: " + expression() + " = " + std::to_string(getValue());
  }
};

class WasmCombination {
  mynum::impl::Combination comb_{};

public:
  WasmCombination() = default;

  WasmCombination(int target, const val &numbers) {
    comb_.target = target;
    readNumbers(numbers, comb_.numbers);
  }

  int getTarget() const { return comb_.target; }

  void setTarget(int target) { comb_.target = target; }

  val getNumbers() const { return writeNumbers(comb_.numbers); }

  void setNumbers(const val &numbers) { readNumbers(numbers, comb_.numbers); }

  WasmSolution solve() const { return WasmSolution(comb_.solve()); }

  std::string toString() const {
    return "Combination { target: " + std::to_string(comb_.target) + ", numbers : " + arrayToString(comb_.numbers) +
           " }";
  }

  val toJSON() const {
    val obj = val::object();
    obj.set("target", comb_.target);

    val numbers = val::array();
    for (size_t i = 0; i < 6; ++i) {
      numbers.call<void>("push", comb_.numbers[i]);
    }
    obj.set("numbers", numbers);

    return obj;
  }

  static WasmCombination generate() {
    WasmCombination combination;
    combination.comb_ = mynum::impl::Combination::generate();
    return combination;
  }
};

} // namespace

EMSCRIPTEN_BINDINGS(mojbroj) {
  emscripten::class_<WasmSolution>("Solution")
      .property("value", &WasmSolution::getValue)
      .function("expression", &WasmSolution::expression)
      .function("toString", &WasmSolution::toString);

  emscripten::class_<WasmCombination>("Combination")
      .constructor<>()
      .constructor<int, const val &>()
      .property("target", &WasmCombination::getTarget, &WasmCombination::setTarget)
      .property("numbers", &WasmCombination::getNumbers, &WasmCombination::setNumbers)
      .function("solve", &WasmCombination::solve)
      .function("toString", &WasmCombination::toString)
      .function("toJSON", &WasmCombination::toJSON)
      .class_function("generate", &WasmCombination::generate);
}
