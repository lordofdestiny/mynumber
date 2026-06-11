#include <impl/Utils.hpp>

namespace mynum::impl {

std::generator<int> submasks(int mask) {
  int submask = mask;
  while (submask != 0) {
    submask = (submask - 1) & mask;
    co_yield submask;
  }
}

std::generator<std::pair<int, int>> submask_complement_pairs(int mask, bool symmetry) {
  for (auto submask : submasks(mask)) {
    auto complement = mask ^ submask;
    if (!symmetry || submask < complement) {
      co_yield {submask, complement};
    }
  }
}
}
