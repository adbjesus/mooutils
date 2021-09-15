#include <catch2/catch.hpp>

#include <mooutils/queues.hpp>

#include <random>

TEST_CASE("fifo queue", "[queues]") {
  int n = GENERATE(10, 100, 1000);
  auto queue = mooutils::fifo_queue<int>();
  for (int i = 0; i < n; ++i) {
    queue.push(i);
  }
  for (int i = 0; i < n; ++i) {
    REQUIRE(int(queue.size()) == n - i);
    REQUIRE(queue.empty() == false);
    REQUIRE(queue.pop() == i);
  }
  REQUIRE(int(queue.size()) == 0);
  REQUIRE(queue.empty() == true);
}

TEST_CASE("lifo queue", "[queues]") {
  int n = GENERATE(10, 100, 1000);
  auto queue = mooutils::lifo_queue<int>();
  for (int i = 0; i < n; ++i) {
    queue.push(i);
  }
  for (int i = 0; i < n; ++i) {
    REQUIRE(int(queue.size()) == n - i);
    REQUIRE(queue.empty() == false);
    REQUIRE(queue.pop() == n - i - 1);
  }
  REQUIRE(int(queue.size()) == 0);
  REQUIRE(queue.empty() == true);
}

TEST_CASE("random queue", "[queues]") {
  using rng_type = std::mt19937_64;
  using rng_result_type = typename rng_type::result_type;

  constexpr auto seed_min = std::numeric_limits<rng_result_type>::min();
  constexpr auto seed_max = std::numeric_limits<rng_result_type>::max();

  int n = GENERATE(10, 100, 1000);
  auto seed = GENERATE(take(10, random(seed_min, seed_max)));
  auto queue = mooutils::random_queue<int, rng_type>(rng_type(seed));

  for (int i = 0; i < n; ++i) {
    queue.push(i);
  }
  for (int i = 0; i < n; ++i) {
    REQUIRE(int(queue.size()) == n - i);
    REQUIRE(queue.empty() == false);
    queue.pop();
  }
  REQUIRE(int(queue.size()) == 0);
  REQUIRE(queue.empty() == true);
}
