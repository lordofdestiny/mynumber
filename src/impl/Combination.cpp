#include <vector>
#include <format>
#include <iostream>
#include <polyfill/generator.hpp>
#include <polyfill/ranges.hpp>
#include <random>
#include <ranges>
#include <unordered_map>
#include <unordered_set>

#include <impl/Combination.hpp>

namespace mynum::impl {

static mynum::polyfill::generator<int> submasks(int mask) {
  int submask = mask;
  while (submask != 0) {
    submask = (submask - 1) & mask;
    co_yield submask;
  }
}

static mynum::polyfill::generator<std::pair<int, int>> submask_complement_pairs(int mask, bool symmetry = false) {
  for (auto submask : submasks(mask)) {
    auto complement = mask ^ submask;
    if (!symmetry || submask < complement) {
      co_yield {submask, complement};
    }
  }
}

std::ostream &operator<<(std::ostream &os, const Combination &comb) {
  return os << std::format("Combination({}, {{ {}, {}, {}, {}, {}, {} }}", comb.target, comb.numbers[0],
                           comb.numbers[1], comb.numbers[2], comb.numbers[3], comb.numbers[4], comb.numbers[5]);
}

Combination Combination::generate() {
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

std::vector<std::shared_ptr<StateValue>> Combination::allSolutions() const {
  std::vector<std::unordered_map<int, std::vector<std::shared_ptr<StateValue>>>> states(1ull << numbers.size());

  for (const auto &[i, v] : mynum::compat::views::enumerate(numbers)) {
    states[1ull << i][v].push_back(std::make_shared<StateValue>(v, Operator::NONE, nullptr, nullptr));
  }

  std::vector<std::shared_ptr<StateValue>> solutions;
  std::unordered_set<std::string> seenHashes;

  for (int mask : std::views::iota(1, 1 << numbers.size())) {
    for (auto [submask1, submask2] : submask_complement_pairs(mask, false)) {
      auto state_vec_pairs = mynum::compat::views::cartesian_product(mynum::compat::views::values(states[submask1]),
                                                                    mynum::compat::views::values(states[submask2]));

      for (const auto &[state_vec1, state_vec2] : state_vec_pairs) {
        auto state_pairs =
            mynum::compat::views::cartesian_product(state_vec1, state_vec2) |
            mynum::compat::views::filter([](const auto &pair) { return std::get<0>(pair)->value < std::get<1>(pair)->value; });

        for (const auto &[state1, state2] : state_pairs) {
          std::vector<std::shared_ptr<StateValue>> new_states;
          new_states.reserve(6);
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

std::shared_ptr<StateValue> Combination::solve() const {
  std::vector<std::unordered_map<int, std::shared_ptr<StateValue>>> states(1ull << numbers.size());

  for (const auto &[i, v] : mynum::compat::views::enumerate(numbers)) {
    states[1ull << i][v] = std::make_shared<StateValue>(v, Operator::NONE, nullptr, nullptr);
  }

  auto best_diff = std::numeric_limits<unsigned int>::max();
  std::shared_ptr<StateValue> best_match;

  for (int mask : std::views::iota(1, 1 << numbers.size())) {
    for (auto [submask1, submask2] : submask_complement_pairs(mask)) {
      auto state_pairs = mynum::compat::views::cartesian_product(mynum::compat::views::values(states[submask1]),
                                                                 mynum::compat::views::values(states[submask2]));

      for (const auto &[state1, state2] : state_pairs) {
        auto val1 = state1->value;
        auto val2 = state2->value;

        std::vector<std::shared_ptr<StateValue>> new_states;
        new_states.reserve(6);
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

          unsigned int diff = std::abs(target - new_state->value);

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

} // namespace mynum::impl
