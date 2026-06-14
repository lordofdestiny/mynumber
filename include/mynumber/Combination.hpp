#ifndef MYNUMBER_COMBINATION_HPP
#define MYNUMBER_COMBINATION_HPP

#include <array>
#include <memory>
#include <vector>

#include <mynumber/Export.hpp>
#include <mynumber/Solution.hpp>

namespace mynum {

class Combination {
public:
  EXPORT_API Combination() noexcept;
  EXPORT_API Combination(int target, std::array<int, 6> numbers) noexcept;

  // non-copyable, move-only
  EXPORT_API Combination(const Combination &) = delete;
  EXPORT_API Combination &operator=(const Combination &) = delete;
  EXPORT_API Combination(Combination &&) noexcept = default;
  EXPORT_API Combination &operator=(Combination &&) noexcept = default;
  EXPORT_API ~Combination();

  EXPORT_API int target() const noexcept;
  EXPORT_API void setTarget(int target) noexcept;

  EXPORT_API std::array<int, 6> numbers() const noexcept;
  EXPORT_API void setNumbers(std::array<int, 6> numbers) noexcept;

  EXPORT_API Solution solve() const;
  EXPORT_API std::vector<Solution> allSolutions() const;

  EXPORT_API static Combination generate();

private:
  struct Impl;
  struct ImplDeleter { EXPORT_API void operator()(Impl *p) noexcept; };
  std::unique_ptr<Impl, ImplDeleter> impl_;
 
};

} // namespace mynum

#endif // MYNUMBER_COMBINATION_HPP
