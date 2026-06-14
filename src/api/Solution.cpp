#include <memory>
#include <mynumber/Solution.hpp>
#include <impl/StateValue.hpp>

namespace mynum {

struct Solution::Impl {
  std::shared_ptr<mynum::impl::StateValue> state;
};

Solution::Solution() noexcept = default;

Solution::Solution(std::shared_ptr<void> nativeState) noexcept
    : impl_(std::unique_ptr<Impl, ImplDeleter>(std::make_unique<Impl>().release())) {
  impl_->state = std::static_pointer_cast<mynum::impl::StateValue>(std::move(nativeState));
}

void Solution::ImplDeleter::operator()(Impl *p) noexcept { delete p; }

Solution::~Solution() = default;

int Solution::value() const {
  if (!impl_ || !impl_->state) {
    return 0;
  }
  return impl_->state->value;
}

std::string Solution::expression() const {
  if (!impl_ || !impl_->state) {
    return std::string();
  }
  return impl_->state->reconstruct();
}

bool Solution::valid() const noexcept {
  return impl_ && static_cast<bool>(impl_->state);
}

} // namespace mynum
