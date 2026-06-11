#include <atomic>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <print>
#include <syncstream>
#include <ranges>
#include <thread>

#include <impl/Combination.hpp>

using namespace mynum::impl;

static std::atomic<int> completedTasks = 0;

struct TaskResult {
  int count;
  Combination comb;
  std::shared_ptr<StateValue> value;
  std::map<int, size_t> counters;
};

TaskResult futureTask(int tasks, int totalTasks) {
  std::osyncstream oss(std::cout);

  auto local_max_diff = std::numeric_limits<unsigned int>::min();
  std::shared_ptr<StateValue> local_best = nullptr;
  std::map<int, size_t> per_diff_counters;
  Combination best_comb{};

  const int progressLimit = totalTasks / 100;

  for (auto _ : std::views::iota(0, tasks)) {
    auto cmb = Combination::generate();
    auto res = cmb.solve();

    int signed_diff = cmb.target - res->value;
    per_diff_counters[signed_diff]++;

    unsigned int diff = std::abs(signed_diff);
    if (diff >= local_max_diff) {
      local_max_diff = diff;
      local_best = res;
      best_comb = cmb;
    }

    int current = ++completedTasks;
    if (current % progressLimit == 0 || current == totalTasks) {
      oss << "progress: " << std::setw(15) << std::format("{}/{}", current, totalTasks) << std::endl << std::flush_emit;
    }
  }

  return {tasks, best_comb, local_best, per_diff_counters};
}

void benchmarkQuality(int n) {
  const auto threadCount = std::thread::hardware_concurrency();
  std::vector<std::future<TaskResult>> futures;

  int tasksPerThread = n / threadCount;
  int remaining = n % threadCount;

  if (tasksPerThread != 0) {
    for (auto _ : std::views::iota(0u, threadCount)) {
      const auto tasks = [=, &remaining] {
        auto total = tasksPerThread;
        if (remaining > 0) {
          total += 1;
          remaining--;
        }
        return total;
      }();
      futures.push_back(std::async(std::launch::async, futureTask, tasks, n));
    }
  } else {
    futures.push_back(std::async(std::launch::async, futureTask, remaining, n));
  }

  auto global_best_diff = std::numeric_limits<unsigned int>::min();
  std::shared_ptr<StateValue> best_state = nullptr;
  Combination best_comb;
  std::map<int, size_t> total_diff_counters;

  for (auto &f : futures) {
    auto const &[tasks, comb, state, counters] = f.get();

    for (const auto &[diff, count] : counters) {
      total_diff_counters[diff] += count;
    }

    unsigned int diff = std::abs(comb.target - state->value);
    if (diff >= global_best_diff) {
      global_best_diff = diff;
      best_state = state;
      best_comb = comb;
    }
  }

  std::cout << "Deviation counts:" << std::endl;
  for (const auto &[diff, count] : total_diff_counters) {
    std::cout << std::setw(5) << diff << " -> " << std::setw(10) << count << std::endl;
  }
  std::cout << std::string(50, '-') << std::endl;
  std::cout << "Maximum deviation:" << global_best_diff << std::endl;
  std::cout << "Expected: " << best_comb << std::endl;
  std::cout << std::format("Found: {} = {}", best_state->reconstruct(), best_state->value) << std::endl;
}

template <bool all> void playGame() {
  auto comb = Combination::generate();
  std::cout << "Target: " << comb.target << std::endl;
  std::cout << std::format("Numbers: {} {} {} {}\t{}\t{}", comb.numbers[0], comb.numbers[1], comb.numbers[2],
                           comb.numbers[3], comb.numbers[4], comb.numbers[5])
            << std::endl;

  auto future = std::async(std::launch::async, [&] {
    if constexpr (all) {
      auto sols = comb.allSolutions();
      return sols;
    } else {
      auto sol = comb.solve();
      return sol;
    }
  });

  using namespace std::chrono_literals;
  std::cout << "Starting timer..." << std::endl;
  std::this_thread::sleep_for(10s);
  std::cout << "Time expired..." << std::endl;

  auto solution = future.get();

  std::cout << "Press enter to see solution..." << std::endl;
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  [[maybe_unused]] auto block = std::cin.get();

  if constexpr (all) {
    if (solution.size() == 0) {
      std::cout << "No exact solutions: " << std::endl;
      return;
    }
    std::cout << "Solution count: " << solution.size() << std::endl;
    for (auto sol : solution) {
      std::println("{} = {}", sol->reconstruct(), sol->value);
    }
  } else {
    std::println("Computer solution: {} = {}", solution->reconstruct(), solution->value);
  }
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
    if (std::from_chars(argN.data(), argN.data() + argN.size(), N).ec != std::errc()) {
      std::cerr << "Argument must be an integer" << std::endl;
      return 4;
    }

    benchmarkQuality(N);
  } else if (std::string_view(argv[1]) == "play") {
    if (argc == 2) {
      playGame<false>();
      return 0;
    }
    if (argc > 3) {
      std::cerr << "Too many arguments" << std::endl;
      return 2;
    }
    if (std::string_view(argv[2]) == "all") {
      playGame<true>();
    } else if (std::string_view(argv[2]) == "solo") {
      playGame<false>();
    } else {
      std::cerr << "Unknown play mode" << std::endl;
      return 3;
    }
  } else {
    std::cerr << "Unknown subcomand" << std::endl;
    return 6;
  }
}
