#include <atomic>
#include <charconv>
#include <chrono>
#include <format>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <thread>
#include <version>

#include <mynumber/Combination.hpp>
#include <mynumber/Solution.hpp>

using namespace mynum;

static std::atomic<int> completedTasks = 0;

#if __cpp_lib_syncbuf
#include <syncstream>
#else
// Fallback to mutex locking
#include <mutex>

static std::mutex cout_mutex;
#endif

struct TaskResult {
  int count;
  Combination comb;
  Solution solution;
  std::map<int, size_t> counters;

  // Custom move constructor to handle move-only types
  TaskResult(int c, Combination &&cm, Solution &&sol, std::map<int, size_t> cnt)
      : count(c), comb(std::move(cm)), solution(std::move(sol)), counters(std::move(cnt)) {}
};

TaskResult futureTask(int tasks, int totalTasks) {
  #if __cpp_lib_syncbuf
  std::osyncstream oss(std::cout);
  oss << std::emit_on_flush;
  #endif

  auto local_max_diff = std::numeric_limits<unsigned int>::min();
  Solution local_best;
  std::map<int, size_t> per_diff_counters;
  Combination best_comb;

  const int progressLimit = totalTasks / 100;

  for (int i = 0; i < tasks; ++i) {
    auto cmb = Combination::generate();
    auto res = cmb.solve();

    int signed_diff = cmb.target() - res.value();
    per_diff_counters[signed_diff]++;

    unsigned int diff = std::abs(signed_diff);
    if (diff >= local_max_diff) {
      local_max_diff = diff;
      local_best = std::move(res);
      best_comb = std::move(cmb);
    }

    int current = ++completedTasks;
    if (current % progressLimit == 0 || current == totalTasks) {
      #if __cpp_lib_syncbuf
      oss << "progress: " << std::setw(15) << std::format("{}/{}", current, totalTasks) << std::endl;
      #else
      std::lock_guard<std::mutex> lock(cout_mutex);
      std::cout << "progress: " << std::setw(15) << std::format("{}/{}", current, totalTasks) << std::endl;
      #endif
    }
  }

  return {tasks, std::move(best_comb), std::move(local_best), per_diff_counters};
}

void benchmarkQuality(int n) {
  const auto threadCount = std::thread::hardware_concurrency();
  std::vector<std::future<TaskResult>> futures;

  int tasksPerThread = n / threadCount;
  int remaining = n % threadCount;

  if (tasksPerThread != 0) {
    for (unsigned i = 0; i < threadCount; ++i) {
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
  Solution best_state;
  Combination best_comb;
  std::map<int, size_t> total_diff_counters;

  for (auto &f : futures) {
    auto res = f.get();
    const auto &counters = res.counters;

    for (const auto &[diff, count] : counters) {
      total_diff_counters[diff] += count;
    }

    unsigned int diff = std::abs(res.comb.target() - res.solution.value());
    if (diff >= global_best_diff) {
      global_best_diff = diff;
      best_state = std::move(res.solution);
      best_comb = std::move(res.comb);
    }
  }

  std::cout << "Deviation counts:" << std::endl;
  for (const auto &[diff, count] : total_diff_counters) {
    std::cout << std::setw(5) << diff << " -> " << std::setw(10) << count << std::endl;
  }
  std::cout << std::string(50, '-') << std::endl;
  std::cout << "Maximum deviation: " << global_best_diff << std::endl;
  const auto numbers = best_comb.numbers();
  std::cout << std::format("Target: {} (Numbers: {} {} {} {} {} {})", best_comb.target(), 
                           numbers[0], numbers[1], numbers[2], numbers[3], numbers[4], numbers[5]) << std::endl;
  std::cout << std::format("Found: {} = {}", best_state.expression(), best_state.value()) << std::endl;
}

void playGameSolo() {
  auto comb = Combination::generate();
  const auto numbers = comb.numbers();
  std::cout << "Target: " << comb.target() << std::endl;
  std::cout << std::format("Numbers: {} {} {} {}\t{}\t{}", numbers[0], numbers[1], numbers[2],
                           numbers[3], numbers[4], numbers[5])
            << std::endl;

  auto future = std::async(std::launch::async, [&] {
    return comb.solve();
  });

  using namespace std::chrono_literals;
  std::cout << "Starting timer..." << std::endl;
  std::this_thread::sleep_for(10s);
  std::cout << "Time expired..." << std::endl;

  auto solution = future.get();

  std::cout << "Press enter to see solution..." << std::endl;
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  [[maybe_unused]] auto block = std::cin.get();

  if (solution.valid()) {
    std::cout << std::format("Computer solution: {} = {}", solution.expression(), solution.value()) << std::endl;
  } else {
    std::cout << "No solution found." << std::endl;
  }
}

void playGameAll() {
  auto comb = Combination::generate();
  const auto numbers = comb.numbers();
  std::cout << "Target: " << comb.target() << std::endl;
  std::cout << std::format("Numbers: {} {} {} {}\t{}\t{}", numbers[0], numbers[1], numbers[2],
                           numbers[3], numbers[4], numbers[5])
            << std::endl;

  auto future = std::async(std::launch::async, [&] {
    return comb.allSolutions();
  });

  using namespace std::chrono_literals;
  std::cout << "Starting timer..." << std::endl;
  std::this_thread::sleep_for(10s);
  std::cout << "Time expired..." << std::endl;

  auto solutions = future.get();

  std::cout << "Press enter to see solutions..." << std::endl;
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  [[maybe_unused]] auto block = std::cin.get();

  if (solutions.empty()) {
    std::cout << "No exact solutions found." << std::endl;
  } else {
    std::cout << "Solution count: " << solutions.size() << std::endl;
    for (auto &sol : solutions) {
      std::cout << std::format("{} = {}", sol.expression(), sol.value()) << std::endl;
    }
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
      playGameSolo();
      return 0;
    }
    if (argc > 3) {
      std::cerr << "Too many arguments" << std::endl;
      return 2;
    }
    if (std::string_view(argv[2]) == "all") {
      playGameAll();
    } else if (std::string_view(argv[2]) == "solo") {
      playGameSolo();
    } else {
      std::cerr << "Unknown play mode" << std::endl;
      return 3;
    }
  } else {
    std::cerr << "Unknown subcommand" << std::endl;
    return 6;
  }
}
