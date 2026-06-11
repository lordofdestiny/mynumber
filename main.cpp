#include <charconv>
#include <chrono>
#include <cmath>
#include <exception>
#include <format>
#include <future>
#include <generator>
#include <system_error>
#if __has_include(<inplace_vector>)
#include <inplace_vector>
#else
#include <vector>
#endif
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <print>
#include <random>
#include <sstream>
#include <stdexcept>
#include <syncstream>
#include <thread>
#include <unordered_map>
#include <utility>

std::generator<int> submasks(int mask) {
  int submask = mask;
  while (submask != 0) {
    submask = (submask - 1) & mask;
    co_yield submask;
  }
}

std::generator<std::pair<int, int>> submask_complement_pairs(int mask) {
  for (auto submask : submasks(mask)) {
    co_yield {submask, mask ^ submask};
  }
}

enum class Operator { NONE = 0, ADD = 2, SUB = 3, MUL = 4, DIV = 5, LAST = 6 };

template <> struct std::formatter<Operator> : std::formatter<std::string> {
  static char to_char(const Operator &op) {
    using enum Operator;
    switch (op) {
    case ADD:
      return '+';
    case SUB:
      return '-';
    case MUL:
      return '*';
    case DIV:
      return '/';
    case NONE:
    case LAST:
    default:
      return '?';
    }
  }

  auto format(const Operator &op, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "{}", to_char(op));
  }
};

struct StateValue {
  int value;
  std::shared_ptr<StateValue> left;
  std::shared_ptr<StateValue> right;
  Operator op;

  std::string reconstruct() const {
    return reconstruct_impl(Operator::NONE, false);
  }

  static std::shared_ptr<StateValue>
  combine(const std::shared_ptr<StateValue> &left,
          const std::shared_ptr<StateValue> &right, Operator op) {
    auto value = calculate(left->value, right->value, op);
    return std::make_shared<StateValue>(value, left, right, op);
  }

private:
  static int calculate(int left, int right, Operator op) {
    using enum Operator;
    switch (op) {
    case ADD:
      return left + right;
    case SUB:
      return left - right;
    case MUL:
      return left * right;
    case DIV:
      return left / right;
    default:
      return -1;
    }
  }

  static int precedence(Operator op) {
    int value = std::to_underlying(op);
    if (value < std::to_underlying(Operator::NONE) ||
        value >= std::to_underlying(Operator::LAST)) {
      return std::to_underlying(Operator::LAST);
    }
    return value >> 1;
  }

  bool needs_parens(Operator parent_op, bool is_right) const {
    if (parent_op == Operator::NONE) {
      return false;
    }

    auto current_prec = precedence(this->op);
    auto parent_prec = precedence(parent_op);

    return (current_prec < parent_prec) ||
           (current_prec == parent_prec and is_right and
            (parent_op == Operator::SUB || parent_op == Operator::DIV));
  }

  std::string reconstruct_impl(Operator parent_op, bool is_right) const {
    if (op == Operator::NONE) {
      return std::to_string(value);
    }

    if (left == nullptr || right == nullptr) {
      throw std::runtime_error("Left or right is nullptr");
    }

    auto left_str = left->reconstruct_impl(op, false);
    auto right_str = right->reconstruct_impl(op, true);

    if (needs_parens(parent_op, is_right)) {
      return std::format("({} {} {})", left_str, op, right_str);
    } else {
      return std::format("{} {} {}", left_str, op, right_str);
    }
  }
};

struct Combination {
  int target{};
  std::array<int, 6> numbers{};

  friend std::ostream &operator<<(std::ostream &os, const Combination &comb) {
    return os << std::format("Combination({}, {{ {}, {}, {}, {}, {}, {} }}",
                             comb.target, comb.numbers[0], comb.numbers[1],
                             comb.numbers[2], comb.numbers[3], comb.numbers[4],
                             comb.numbers[5]);
  }

  static Combination generate() {
    static std::random_device rd;
    static std::seed_seq ssq{rd()};
    static std::mt19937 gen{ssq};

    static std::uniform_int_distribution target_dist(1, 999);
    static std::uniform_int_distribution digits_dist(1, 9);

    static std::uniform_int_distribution middle_dist(0, 2);
    static std::array middle_numbers = {10, 15, 20};

    static std::uniform_int_distribution large_dist(0, 3);
    static std::array large_numbers = {25, 50, 75, 100};

    auto target = target_dist(gen);
    std::array<int, 6> numbers = {
        digits_dist(gen),
        digits_dist(gen),
        digits_dist(gen),
        digits_dist(gen),
        middle_numbers[middle_dist(gen)],
        large_numbers[large_dist(gen)],
    };

    return {target, numbers};
  }

  std::shared_ptr<StateValue> solve() const {
    auto states = generate_initial_state();

    int best_diff = std::numeric_limits<int>::max();
    std::shared_ptr<StateValue> best_match;

    for (int mask : std::views::iota(1, 1 << numbers.size())) {
      for (auto [submask1, submask2] : submask_complement_pairs(mask)) {
        for (const auto &[val1, state1] : states[submask1]) {
          for (const auto &[val2, state2] : states[submask2]) {
#if defined(__cpp_lib_inplace_vector)
            std::inplace_vector<std::shared_ptr<StateValue>, 6> new_states;
#else
            std::vector<std::shared_ptr<StateValue>> new_states;
#endif
            new_states.push_back(
                StateValue::combine(state1, state2, Operator::ADD));
            new_states.push_back(
                StateValue::combine(state1, state2, Operator::MUL));
            if (val1 > val2) {
              new_states.push_back(
                  StateValue::combine(state1, state2, Operator::SUB));
            }
            if (val2 < val1) {
              new_states.push_back(
                  StateValue::combine(state2, state1, Operator::SUB));
            }
            if (val2 != 0 && val1 % val2 == 0) {
              new_states.push_back(
                  StateValue::combine(state1, state2, Operator::DIV));
            }
            if (val1 != 0 && val2 % val1 == 0) {
              new_states.push_back(
                  StateValue::combine(state2, state1, Operator::DIV));
            }

            for (auto new_state : new_states) {
              if (states[mask].contains(new_state->value)) {
                new_state.reset();
                continue;
              }

              states[mask][new_state->value] = new_state;

              auto diff = std::abs(target - new_state->value);

              if (diff >= best_diff) {
                continue;
              }

              best_diff = diff;
              best_match = new_state;

              if (diff == 0) {
                return new_state;
              }
            }
          }
        }
      }
    }

    return best_match;
  }

private:
  std::vector<std::unordered_map<int, std::shared_ptr<StateValue>>>
  generate_initial_state() const {
    std::vector<std::unordered_map<int, std::shared_ptr<StateValue>>> initial(
        1 << numbers.size());

    for (const auto &[i, v] : std::views::enumerate(numbers)) {
      initial[1 << i][v] =
          std::make_shared<StateValue>(v, nullptr, nullptr, Operator::NONE);
    }
    return initial;
  }
};

static std::atomic<int> completedTasks = 0;

struct TaskResult {
  Combination comb;
  std::shared_ptr<StateValue> value;
  int diff;
};

TaskResult futureTask(int tasks, int totalTasks) {
  int local_max_diff = std::numeric_limits<int>::min();
  std::shared_ptr<StateValue> local_best = nullptr;
  Combination best_comb{};

  const int progressLimit = totalTasks / 100;

  for (auto _ : std::views::iota(0, tasks)) {
    auto cmb = Combination::generate();
    auto res = cmb.solve();
    int diff = std::abs(cmb.target - res->value);

    if (diff > local_max_diff) {
      local_max_diff = diff;
      local_best = res;
      best_comb = cmb;
    }

    int current = ++completedTasks;
    if (current % progressLimit == 0 || current == totalTasks) {
      std::osyncstream(std::cout)
          << "progress: " << std::setw(20)
          << std::format("{}/{}", current, totalTasks) << std::endl;
    }
  }

  return {best_comb, local_best, local_max_diff};
}

void benchmarkQuality(int n) {
  const auto threadCount = std::thread::hardware_concurrency();
  std::vector<std::future<TaskResult>> futures;

  int tasksPerThread = n / threadCount;
  int remaining = n % threadCount;

  if (tasksPerThread != 0) {
    for (auto _ : std::views::iota(0u, threadCount)) {
      futures.push_back(
          std::async(std::launch::async, futureTask, tasksPerThread, n));
    }
  }

  if (remaining != 0) {
    futures.push_back(std::async(std::launch::async, futureTask, remaining, n));
  }

  int global_best_diff = std::numeric_limits<int>::min();
  std::shared_ptr<StateValue> best_state = nullptr;
  Combination best_comb;
  for (auto &f : futures) {
    auto const &[comb, state, diff] = f.get();
    if (diff > global_best_diff) {
      global_best_diff = diff;
      best_state = state;
      best_comb = comb;
    }
  }

  std::cout << "Maximum deviation: " << global_best_diff << std::endl;
  std::cout << "Expected: " << best_comb << std::endl;
  std::cout << std::format("Found: {} = {}", best_state->reconstruct(),
                           best_state->value)
            << std::endl;
}

void playGame() {
  auto comb = Combination::generate();
  std::cout << "Target: " << comb.target << std::endl;
  std::cout << std::format("Numbers: {} {} {} {}\t{}\t{}", comb.numbers[0],
                           comb.numbers[1], comb.numbers[2], comb.numbers[3],
                           comb.numbers[4], comb.numbers[5])
            << std::endl;

  auto future = std::async(std::launch::async, [&] {
    auto sol = comb.solve();
    return sol;
  });

  using namespace std::chrono_literals;
  std::cout << "Starting timer..." << std::endl;
  std::this_thread::sleep_for(10s);
  std::cout << "Time expired..." << std::endl;

  auto solution = future.get();

  std::cout << "Press enter to see solution..." << std::endl;
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  [[maybe_unused]] auto block = std::cin.get();

  std::cout << std::format("Computer solution: {} = {}",
                           solution->reconstruct(), solution->value)
            << std::endl;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Invalid use" << std::endl;
    return 1;
  }

  if (std::string_view(argv[1]) == "benchmark") {
    if (argc < 3) {
      std::cerr << "Invalid use" << std::endl;
      return 2;
    }

    int N;
    std::string argN = argv[2];
    if (std::from_chars(argN.data(), argN.data() + argN.size(), N).ec !=
        std::errc()) {
      std::cerr << "Argument must be an integer" << std::endl;
      return 0;
    }

    benchmarkQuality(N);
  } else if (std::string_view(argv[1]) == "play") {
    playGame();
  } else {
    std::cerr << "Unknown subcomand" << std::endl;
  }
}
