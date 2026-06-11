#include <charconv>
#include <chrono>
#include <cmath>
#include <exception>
#include <format>
#include <future>
#include <generator>
#include <string>
#include <system_error>
#include <unordered_set>
#if __has_include(<inplace_vector>)
#include <inplace_vector>
#else
#include <vector>
#endif
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
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

std::generator<std::pair<int, int>> submask_complement_pairs(int mask, bool symmetry = false) {
  for (auto submask : submasks(mask)) {
    auto complement = mask ^ submask;
    if (!symmetry || submask < complement) {
      co_yield {submask, complement};
    }
  }
}

enum class Operator : unsigned short {
  NONE = 0,
  ADD = '+' << 3 | 2,
  SUB = '-' << 3 | 3,
  MUL = '*' << 3 | 4,
  DIV = '/' << 3 | 5,
  LAST = '?' << 3 | 6
};

// Define to_string in the same namespace
std::string to_string(Operator p) { return std::string(1, static_cast<char>(std::to_underlying(p) >> 3)); }

template <> struct std::formatter<Operator> : std::formatter<std::string> {
  static char to_char(const Operator &op) { return std::to_underlying(op) >> 3; }

  auto format(const Operator &op, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "{}", to_char(op));
  }
};

struct StateValue {
  int value;
  Operator op;
  std::shared_ptr<StateValue> left;
  std::shared_ptr<StateValue> right;

  std::string reconstruct() const {
    std::string buffer;
    reconstruct_impl(buffer, Operator::NONE, false);
    return buffer;
  }

  static std::shared_ptr<StateValue> combine(const std::shared_ptr<StateValue> &left,
                                             const std::shared_ptr<StateValue> &right, Operator op) {
    auto value = calculate(left->value, right->value, op);
    if (value < 0) {
      std::cout << left->reconstruct() << std::endl;
      std::cout << right->reconstruct() << std::endl;
      std::cout << std::format("{}", op) << std::endl;
      throw "What the hell";
    }
    return std::make_shared<StateValue>(value, op, left, right);
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

  static int precedence(Operator op) { return std::to_underlying(op) & 0x7; }

  bool needs_parens(Operator parent_op, bool is_right) const {
    if (parent_op == Operator::NONE) {
      return false;
    }

    auto current_prec = precedence(this->op);
    auto parent_prec = precedence(parent_op);

    return (current_prec < parent_prec) ||
           (current_prec == parent_prec and is_right and (parent_op == Operator::SUB || parent_op == Operator::DIV));
  }

  void reconstruct_impl(std::string &buffer, Operator parent_op, bool is_right) const {
    if (op == Operator::NONE) {
      buffer += std::to_string(value);
      return;
    }

    if (left == nullptr || right == nullptr) {
      throw std::runtime_error("Left or right is nullptr");
    }

    bool parens = needs_parens(parent_op, is_right);

    using std::to_string;
    if (parens)
      buffer += '(';
    left->reconstruct_impl(buffer, op, false);
    buffer += ' ';
    buffer += to_string(op);
    buffer += ' ';
    right->reconstruct_impl(buffer, op, true);
    if (parens)
      buffer += ')';
  }
};

struct Combination {
  int target{};
  std::array<int, 6> numbers{};

  friend std::ostream &operator<<(std::ostream &os, const Combination &comb) {
    return os << std::format("Combination({}, {{ {}, {}, {}, {}, {}, {} }}", comb.target, comb.numbers[0],
                             comb.numbers[1], comb.numbers[2], comb.numbers[3], comb.numbers[4], comb.numbers[5]);
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

  std::vector<std::shared_ptr<StateValue>> allSolutions() const {
    std::vector<std::unordered_map<int, std::vector<std::shared_ptr<StateValue>>>> states(1 << numbers.size());

    for (const auto &[i, v] : std::views::enumerate(numbers)) {
      states[1 << i][v].push_back(std::make_shared<StateValue>(v, Operator::NONE, nullptr, nullptr));
    }

    std::vector<std::shared_ptr<StateValue>> solutions;
    std::unordered_set<std::string> seenHashes;

    for (int mask : std::views::iota(1, 1 << numbers.size())) {
      for (auto [submask1, submask2] : submask_complement_pairs(mask, false)) {
        auto state_vec_pairs =
            std::views::cartesian_product(std::views::values(states[submask1]), std::views::values(states[submask2]));

        for (const auto &[state_vec1, state_vec2] : state_vec_pairs) {
          auto state_pairs =
              std::views::cartesian_product(state_vec1, state_vec2) |
              std::views::filter([](const auto &pair) { return std::get<0>(pair)->value < std::get<1>(pair)->value; });

          for (const auto &[state1, state2] : state_pairs) {
#if defined(__cpp_lib_inplace_vector)
            std::inplace_vector<std::shared_ptr<StateValue>, 6> new_states;
#else
            std::vector<std::shared_ptr<StateValue>> new_states;
            new_states.reserve(6);
#endif
            new_states.push_back(StateValue::combine(state1, state2, Operator::ADD));
            new_states.push_back(StateValue::combine(state1, state2, Operator::MUL));
            new_states.push_back(StateValue::combine(state2, state1, Operator::SUB));
            if (state2->value != 0 && state1->value % state2->value == 0) {
              new_states.push_back(StateValue::combine(state1, state2, Operator::DIV));
            }
            if (state1->value != 0 && state2->value % state1->value == 0) {
              new_states.push_back(StateValue::combine(state2, state1, Operator::DIV));
            }

            for (auto new_state : new_states) {
              auto str = new_state->reconstruct();
              if (seenHashes.contains(str)) {
                continue;
              }

              states[mask][new_state->value].push_back(new_state);
              seenHashes.insert(str);
              if (target - new_state->value == 0) {
                solutions.push_back(new_state);
              }
            }
          }
        }
      }
    }

    return solutions;
  }

  std::shared_ptr<StateValue> solve() const {
    std::vector<std::unordered_map<int, std::shared_ptr<StateValue>>> states(1 << numbers.size());

    for (const auto &[i, v] : std::views::enumerate(numbers)) {
      states[1 << i][v] = std::make_shared<StateValue>(v, Operator::NONE, nullptr, nullptr);
    }

    auto best_diff = std::numeric_limits<int>::max();
    std::shared_ptr<StateValue> best_match;

    for (int mask : std::views::iota(1, 1 << numbers.size())) {
      for (auto [submask1, submask2] : submask_complement_pairs(mask)) {
        auto state_pairs =
            std::views::cartesian_product(std::views::values(states[submask1]), std::views::values(states[submask2]));

        for (const auto &[state1, state2] : state_pairs) {
          auto val1 = state1->value;
          auto val2 = state2->value;

#if defined(__cpp_lib_inplace_vector)
          std::inplace_vector<std::shared_ptr<StateValue>, 6> new_states;
#else
          std::vector<std::shared_ptr<StateValue>> new_states;
          new_states.reserve(6);
#endif
          new_states.push_back(StateValue::combine(state1, state2, Operator::ADD));
          new_states.push_back(StateValue::combine(state1, state2, Operator::MUL));
          if (val1 > val2) {
            new_states.push_back(StateValue::combine(state1, state2, Operator::SUB));
          }
          if (val2 > val1) {
            new_states.push_back(StateValue::combine(state2, state1, Operator::SUB));
          }
          if (val2 != 0 && val1 % val2 == 0) {
            new_states.push_back(StateValue::combine(state1, state2, Operator::DIV));
          }
          if (val1 != 0 && val2 % val1 == 0) {
            new_states.push_back(StateValue::combine(state2, state1, Operator::DIV));
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

    return best_match;
  }
};

static std::atomic<int> completedTasks = 0;

struct TaskResult {
  Combination comb;
  std::shared_ptr<StateValue> value;
};

TaskResult futureTask(int tasks, int totalTasks) {
  std::osyncstream oss(std::cout);

  auto local_max_diff = std::numeric_limits<int>::min();
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
      oss << "progress: " << std::setw(20) << std::format("{}/{}", current, totalTasks) << std::endl;
    }
  }

  return {best_comb, local_best};
}

void benchmarkQuality(int n) {
  const auto threadCount = std::thread::hardware_concurrency();
  std::vector<std::future<TaskResult>> futures;

  int tasksPerThread = n / threadCount;
  int remaining = n % threadCount;

  if (tasksPerThread != 0) {
    for (auto _ : std::views::iota(0u, threadCount)) {
      futures.push_back(std::async(std::launch::async, futureTask, tasksPerThread, n));
    }
  }

  if (remaining != 0) {
    futures.push_back(std::async(std::launch::async, futureTask, remaining, n));
  }

  auto global_best_diff = std::numeric_limits<int>::min();
  std::shared_ptr<StateValue> best_state = nullptr;
  Combination best_comb;
  for (auto &f : futures) {
    auto const &[comb, state] = f.get();
    auto diff = std::abs(comb.target - state->value);
    if (diff > global_best_diff) {
      global_best_diff = diff;
      best_state = state;
      best_comb = comb;
    }
  }

  std::cout << "Maximum deviation: " << global_best_diff << std::endl;
  std::cout << "Expected: " << best_comb << std::endl;
  std::cout << std::format("Found: {} = {}", best_state->reconstruct(), best_state->value) << std::endl;
}

void playGame() {
  auto comb = Combination::generate();
  std::cout << "Target: " << comb.target << std::endl;
  std::cout << std::format("Numbers: {} {} {} {}\t{}\t{}", comb.numbers[0], comb.numbers[1], comb.numbers[2],
                           comb.numbers[3], comb.numbers[4], comb.numbers[5])
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

  std::cout << std::format("Computer solution: {} = {}", solution->reconstruct(), solution->value) << std::endl;
}

int main(int argc, char **argv) {
  auto cmb = Combination{227, {2, 5, 5, 9, 10, 50}};
  auto sols = cmb.allSolutions();
  std::cout << "Solution count: " << sols.size() << std::endl;
  for (auto const &sol : sols) {
    std::cout << sol->reconstruct() << std::endl;
  }

  return 0;
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
    if (std::from_chars(argN.data(), argN.data() + argN.size(), N).ec != std::errc()) {
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
