#ifndef __COMBINATION_HPP__
#define __COMBINATION_HPP__

#include <array>
#include <iosfwd>
#include <memory>
#include <vector>

#include <impl/Export.hpp>
#include <impl/StateValue.hpp>

namespace mynum::impl {

struct Combination {
  int target{};
  std::array<int, 6> numbers{};

  friend std::ostream &operator<<(std::ostream &os, const Combination &comb);

  static Combination generate();

public:
  std::vector<std::shared_ptr<StateValue>> allSolutions() const;

  std::shared_ptr<StateValue> solve() const;
};

} // namespace mynum::impl

#endif
