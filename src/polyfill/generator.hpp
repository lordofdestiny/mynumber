#ifndef __POLYFILL_GENERATOR_HPP__
#define __POLYFILL_GENERATOR_HPP__

#if defined(__cpp_lib_generator) && __cpp_lib_generator >= 202207L

#include <generator>

namespace mynum::polyfill {
using std::generator;
}

#elif ((defined(__cplusplus) && __cplusplus >= 202300L) \
    || (defined(_MSVC_LANG) && _MSVC_LANG >= 202300L)) \
    || (__has_include(<generator>) && defined(__cpp_lib_generator))

#include <generator>

namespace mynum::polyfill {
using std::generator;
}

#else

#include <coroutine>
#include <cstddef>
#include <exception>
#include <iterator>
#include <type_traits>
#include <utility>

namespace mynum::polyfill {

template <class Ref, class Value = std::remove_cvref_t<Ref>>
class generator {
public:
  struct promise_type {
    Value value_{};

    generator get_return_object() noexcept {
      return generator{std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }

    void unhandled_exception() {
      if (std::current_exception()) {
        std::rethrow_exception(std::current_exception());
      }
    }

    std::suspend_always yield_value(const Value &value) {
      value_ = value;
      return {};
    }

    std::suspend_always yield_value(Value &&value) {
      value_ = std::move(value);
      return {};
    }

    void return_void() noexcept {}
  };

  using value_type = Value;
  using reference = Ref;

  struct iterator {
    std::coroutine_handle<promise_type> coro_{};
    bool done_ = true;

    using iterator_concept = std::input_iterator_tag;
    using value_type = generator::value_type;
    using difference_type = std::ptrdiff_t;
    using reference = generator::reference;

    iterator() = default;

    explicit iterator(std::coroutine_handle<promise_type> coro, bool done) 
        : coro_(coro), done_(done) {}

    reference operator*() const { return coro_.promise().value_; }

    iterator &operator++() {
      coro_.resume();
      done_ = coro_.done();
      return *this;
    }

    void operator++(int) { ++*this; }

    bool operator==(std::default_sentinel_t) const { return done_; }
  };

  generator() = default;

  generator(generator &&other) noexcept : coro_(std::exchange(other.coro_, {})) {}

  ~generator() {
    if (coro_) {
      coro_.destroy();
    }
  }

  generator(const generator &) = delete;
  generator &operator=(const generator &) = delete;

  generator &operator=(generator &&other) noexcept {
    if (this != &other) {
      if (coro_) {
        coro_.destroy();
      }
      coro_ = std::exchange(other.coro_, {});
    }
    return *this;
  }

  iterator begin() {
    if (!coro_) {
      return {};
    }
    coro_.resume();
    return iterator{coro_, coro_.done()};
  }

  std::default_sentinel_t end() const { return {}; }

private:
  explicit generator(std::coroutine_handle<promise_type> coro) : coro_(coro) {}

  std::coroutine_handle<promise_type> coro_{};
};

} // namespace mynum::polyfill

#endif

#endif
