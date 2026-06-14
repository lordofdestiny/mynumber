#include <algorithm>
#include <memory>
#include <mynumber/Combination.hpp>
#include <impl/Combination.hpp>
#include <impl/StateValue.hpp>

namespace mynum {

struct Combination::Impl {
  mynum::impl::Combination value;
};


Combination::Combination() noexcept : impl_(std::unique_ptr<Impl, ImplDeleter>(std::make_unique<Impl>().release())) {}

Combination::~Combination() = default;

Combination::Combination(int target, std::array<int, 6> numbers) noexcept : impl_(std::unique_ptr<Impl, ImplDeleter>(std::make_unique<Impl>().release())) {
  impl_->value.target = target;
  impl_->value.numbers = numbers;
}

void Combination::ImplDeleter::operator()(Impl *p) noexcept { delete p; }

int Combination::target() const noexcept {
  return impl_->value.target;
}

void Combination::setTarget(int target) noexcept {
  impl_->value.target = target;
}

std::array<int, 6> Combination::numbers() const noexcept {
  return impl_->value.numbers;
}

void Combination::setNumbers(std::array<int, 6> numbers) noexcept {
  impl_->value.numbers = numbers;
}

Solution Combination::solve() const {
  auto nativeSolution = impl_->value.solve();
  return Solution(std::static_pointer_cast<void>(std::move(nativeSolution)));
}

std::vector<Solution> Combination::allSolutions() const {
  auto nativeSolutions = impl_->value.allSolutions();
  std::vector<Solution> solutions;
  solutions.reserve(nativeSolutions.size());

  for (auto &nativeSolution : nativeSolutions) {
    solutions.emplace_back(Solution(std::static_pointer_cast<void>(std::move(nativeSolution))));
  }

  return solutions;
}

Combination Combination::generate() {
  Combination combination;
  combination.impl_->value = mynum::impl::Combination::generate();
  return combination;
}

} // namespace mynum
