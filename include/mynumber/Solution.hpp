#ifndef MYNUMBER_SOLUTION_HPP
#define MYNUMBER_SOLUTION_HPP

#include <memory>
#include <string>
#include <mynumber/Export.hpp>

namespace mynum {

class Solution {
public:
  EXPORT_API Solution() noexcept;
  EXPORT_API int value() const;
  EXPORT_API std::string expression() const;
  EXPORT_API bool valid() const noexcept;
  EXPORT_API explicit operator bool() const noexcept { return valid(); }

  // non-copyable, move-only
  EXPORT_API Solution(const Solution &) = delete;
  EXPORT_API Solution &operator=(const Solution &) = delete;
  EXPORT_API Solution(Solution &&) noexcept = default;
  EXPORT_API Solution &operator=(Solution &&) noexcept = default;
  EXPORT_API ~Solution();

private:
  struct Impl;
  struct ImplDeleter { EXPORT_API void operator()(Impl *p) noexcept; };
  std::unique_ptr<Impl, ImplDeleter> impl_;

  explicit Solution(std::shared_ptr<void> nativeState) noexcept;
  friend class Combination;
 
};

} // namespace mynum

#endif // MYNUMBER_SOLUTION_HPP
