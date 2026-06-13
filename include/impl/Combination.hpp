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

  EXPORT_API friend std::ostream &operator<<(std::ostream &os, const Combination &comb);

  EXPORT_API static Combination generate();

public:
  EXPORT_API std::vector<std::shared_ptr<StateValue>> allSolutions() const;

  EXPORT_API std::shared_ptr<StateValue> solve() const;
};

} // namespace mynum::impl

#endif
