#pragma once

#include <deque>
#include <random>
#include <type_traits>
#include <vector>

namespace moutils::solution_queues {

template <typename Solution, typename Container = std::deque<Solution>>
class fifo {
 public:
  using value_type = Solution;
  using container_type = Container;

  // Remove and return the next solution in the queue.
  //
  // Calling pop on an empty container is undefined.
  auto pop() -> Solution {
    auto ret = std::move(m_container.front());
    m_container.pop_front();
    return ret;
  }

  // Push a new solution to the queue
  template <typename S>
  auto push(S&& s) {
    m_container.emplace_back(std::forward<S>(s));
  }

  // Check if queue is empty
  auto empty() -> bool {
    return m_container.empty();
  }

  auto size() {
    return m_container.size();
  }

 private:
  container_type m_container;
};

template <typename Solution, typename Container = std::vector<Solution>>
class lifo {
 public:
  using value_type = Solution;
  using container_type = Container;

  // Remove and return the next solution in the queue.
  //
  // Calling pop on an empty container is undefined.
  auto pop() -> Solution {
    auto ret = std::move(m_container.back());
    m_container.pop_back();
    return ret;
  }

  // Push a new solution to the queue
  template <typename S>
  auto push(S&& s) {
    m_container.emplace_back(std::forward<S>(s));
  }

  // Check if queue is empty
  auto empty() -> bool {
    return m_container.empty();
  }

  auto size() {
    return m_container.size();
  }

 private:
  container_type m_container;
};

template <typename Solution, typename Rng, typename Container = std::vector<Solution>>
class random {
 public:
  using value_type = Solution;
  using rng_type = Rng;
  using container_type = Container;

  explicit random(Rng&& rng)
      : m_rng(std::move(rng))
      , m_container() {}

  // Remove and return the next solution in the queue.
  //
  // Calling pop on an empty container is undefined.
  auto pop() -> Solution {
    auto size_minus_1 = m_container.size() - 1;
    auto runif = std::uniform_int_distribution<size_t>(0, size_minus_1);
    auto ind = runif(m_rng);
    auto ret = std::move(m_container[ind]);
    if (ind != size_minus_1) {
      m_container[ind] = std::move(m_container.back());
    }
    m_container.pop_back();
    return ret;
  }

  // Push a new solution to the queue
  template <typename S>
  auto push(S&& s) {
    m_container.emplace_back(std::forward<S>(s));
  }

  // Check if queue is empty
  auto empty() -> bool {
    return m_container.empty();
  }

  auto size() {
    return m_container.size();
  }

 private:
  container_type m_container;
  rng_type m_rng;
};

}  // namespace moutils::solution_queues
