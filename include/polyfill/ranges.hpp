#ifndef __POLYFILL_RANGES_HPP__
#define __POLYFILL_RANGES_HPP__

#include <cstddef>
#include <ranges>
#include <tuple>
#include <utility>

namespace mynum::polyfill::views {

#if defined(__cpp_lib_ranges_enumerate) && __cpp_lib_ranges_enumerate >= 202302L

using std::ranges::views::enumerate;

#else

template <std::ranges::view V>
class enumerate_view : public std::ranges::view_interface<enumerate_view<V>> {
public:
  enumerate_view() requires std::default_initializable<V> = default;

  explicit enumerate_view(V base) : base_(std::move(base)) {}

  struct iterator {
    std::ranges::iterator_t<V> current_{};
    std::ranges::sentinel_t<V> end_{};
    std::size_t index_ = 0;

    using iterator_concept = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::tuple<std::size_t, std::ranges::range_value_t<V>>;
    using reference = std::tuple<std::size_t, std::ranges::range_reference_t<V>>;

    iterator() = default;

    iterator(std::ranges::iterator_t<V> current, std::ranges::sentinel_t<V> end, std::size_t index)
        : current_(std::move(current)), end_(std::move(end)), index_(index) {}

    reference operator*() const { return reference{index_, *current_}; }

    iterator &operator++() {
      ++current_;
      ++index_;
      return *this;
    }

    void operator++(int) { ++*this; }

    friend bool operator==(const iterator &lhs, const iterator &rhs) { return lhs.current_ == rhs.current_; }
    friend bool operator==(const iterator &lhs, std::default_sentinel_t) { return lhs.current_ == lhs.end_; }
    friend bool operator==(std::default_sentinel_t, const iterator &rhs) { return rhs.current_ == rhs.end_; }
  };

  iterator begin() { return iterator{std::ranges::begin(base_), std::ranges::end(base_), 0}; }

  iterator begin() const { return iterator{std::ranges::begin(base_), std::ranges::end(base_), 0}; }

  std::default_sentinel_t end() const { return {}; }

private:
  V base_{};
};

template <std::ranges::viewable_range R>
[[nodiscard]] auto enumerate(R &&range) {
  return enumerate_view<std::views::all_t<R>>(std::views::all(std::forward<R>(range)));
}

#endif

#if defined(__cpp_lib_ranges_cartesian_product) && __cpp_lib_ranges_cartesian_product >= 202207L

using std::ranges::views::cartesian_product;

#else

template <std::ranges::view V1, std::ranges::view V2>
class cartesian_product_view : public std::ranges::view_interface<cartesian_product_view<V1, V2>> {
public:
  cartesian_product_view() requires std::default_initializable<V1> && std::default_initializable<V2> = default;

  cartesian_product_view(V1 first, V2 second) : first_(std::move(first)), second_(std::move(second)) {}

  struct iterator {
    using value_type = std::tuple<std::ranges::range_value_t<V1>, std::ranges::range_value_t<V2>>;
    using reference = std::tuple<std::ranges::range_reference_t<V1>, std::ranges::range_reference_t<V2>>;

    std::ranges::iterator_t<V1> outer_{};
    std::ranges::iterator_t<V1> outer_end_{};
    std::ranges::iterator_t<V2> inner_{};
    std::ranges::iterator_t<V2> inner_begin_{};
    std::ranges::iterator_t<V2> inner_end_{};
    bool done_ = true;

    using iterator_concept = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;

    iterator() = default;

    explicit iterator(const cartesian_product_view &parent) {
      outer_ = std::ranges::begin(parent.first_);
      outer_end_ = std::ranges::end(parent.first_);
      inner_begin_ = std::ranges::begin(parent.second_);
      inner_end_ = std::ranges::end(parent.second_);

      if (outer_ != outer_end_ && inner_begin_ != inner_end_) {
        inner_ = inner_begin_;
        done_ = false;
      }
    }

    reference operator*() const { return reference{*outer_, *inner_}; }

    iterator &operator++() {
      ++inner_;
      if (inner_ == inner_end_) {
        ++outer_;
        if (outer_ != outer_end_) {
          inner_ = inner_begin_;
        } else {
          done_ = true;
        }
      }
      return *this;
    }

    void operator++(int) { ++*this; }

    friend bool operator==(const iterator &lhs, std::default_sentinel_t) { return lhs.done_; }
    friend bool operator==(std::default_sentinel_t, const iterator &rhs) { return rhs.done_; }
  };

  iterator begin() { return iterator{*this}; }

  iterator begin() const { return iterator{*this}; }

  std::default_sentinel_t end() const { return {}; }

private:
  V1 first_{};
  V2 second_{};
};

template <std::ranges::viewable_range R1, std::ranges::viewable_range R2>
[[nodiscard]] auto cartesian_product(R1 &&first, R2 &&second) {
  return cartesian_product_view<std::views::all_t<R1>, std::views::all_t<R2>>(
      std::views::all(std::forward<R1>(first)), std::views::all(std::forward<R2>(second)));
}

#endif

} // namespace mynum::polyfill::views

namespace mynum::compat::views {
using std::ranges::views::filter;
using std::ranges::views::iota;
using std::ranges::views::values;

#if defined(__cpp_lib_ranges_enumerate) && __cpp_lib_ranges_enumerate >= 202302L
using std::ranges::views::enumerate;
#else
using mynum::polyfill::views::enumerate;
#endif

#if defined(__cpp_lib_ranges_cartesian_product) && __cpp_lib_ranges_cartesian_product >= 202207L
using std::ranges::views::cartesian_product;
#else
using mynum::polyfill::views::cartesian_product;
#endif

} // namespace mynum::compat::views

#endif
