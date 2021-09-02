#pragma once

#include <deque>
#include <type_traits>
#include <vector>

namespace moco {

template <typename Solution, typename Container = std::deque<Solution>>
class fifosq {
 public:
  using solution_type = Solution;
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

 private:
  container_type m_container;
};

template <typename Solution, typename Container = std::vector<Solution>>
class lifosq {
 public:
  using solution_type = Solution;
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

 private:
  container_type m_container;
};

}  // namespace moco
