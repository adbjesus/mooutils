#include <catch2/catch.hpp>

#include <fmt/core.h>
#include <fmt/ranges.h>
#include <mooutils/util/dominance.hpp>
#include <mooutils/util/solution.hpp>
#include <mooutils/util/solution_queues.hpp>

#include <algorithm>
#include <cmath>
#include <deque>
#include <list>
#include <random>
#include <ranges>
#include <set>
#include <typeinfo>
#include <vector>

TEST_CASE("fifo_solution_queue", "[solution_queue]") {
  int n = GENERATE(10, 100, 1000);
  auto queue = mooutils::solution_queues::fifo<int>();
  for (int i : std::views::iota(0, n)) {
    queue.push(i);
  }
  for (int i : std::views::iota(0, n)) {
    REQUIRE(queue.empty() == false);
    REQUIRE(queue.pop() == i);
  }
  REQUIRE(queue.empty() == true);
}

TEST_CASE("lifo_solution_queue", "[solution_queue]") {
  int n = GENERATE(10, 100, 1000);
  auto queue = mooutils::solution_queues::lifo<int>();
  for (int i : std::views::iota(0, n)) {
    queue.push(i);
  }
  for (int i : std::views::iota(0, n) | std::views::reverse) {
    REQUIRE(queue.empty() == false);
    REQUIRE(queue.pop() == i);
  }
  REQUIRE(queue.empty() == true);
}

TEST_CASE("random_solution_queue", "[solution_queue]") {
  using rng_type = std::mt19937_64;
  using rng_result_type = std::mt19937_64::result_type;

  int n = GENERATE(10, 100, 1000);
  auto seed = GENERATE(take(1, random(std::numeric_limits<rng_result_type>::min(),
                                      std::numeric_limits<rng_result_type>::max())));
  auto queue = mooutils::solution_queues::random<int, rng_type>(rng_type(seed));

  REQUIRE(queue.empty() == true);

  for (int i : std::views::iota(0, n)) {
    queue.push(i);
    REQUIRE(queue.empty() == false);
  }

  for (int _i : std::views::iota(0, n)) {
    REQUIRE(queue.empty() == false);
    queue.pop();
  }

  REQUIRE(queue.empty() == true);
}
